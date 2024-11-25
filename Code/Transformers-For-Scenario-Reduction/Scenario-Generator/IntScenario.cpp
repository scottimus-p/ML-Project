#include "IntScenario.h"

#include "C3RNG.h"
#include "StochasticExclusionTest.h"
#include "ScenarioGenerator.h"

#include <cmath>


double Minr2;

YieldCurve& IntScenario::curve(int curveNum)
{
    return curves(curveNum);
}


/**
 * This routine calculates a signficance measure for this scenario.
 * The significance measure is used by the scenario picking tool
 * to rank scenarios and then pick subsets of scenarios whose members
 * have equally spaced ranks.
 */

double IntScenario::significance()
{
    int n = 360;
    if (n > numCurves)
        n = numCurves;

    double s = 0;   // annuity factor as sum of discount factors
    double v = 1;   // discount factor
    
    for (auto i = 1; i <= n; i++)
    {
        YieldCurve& yldCurve = curves(i);
        double intRate = yldCurve.rateAtMaturity(20);

        v = pow(v * (1 + intRate / 2), -0.33333333333);
        s = s + v;
    }

    s = sqrt(s);

    return s;
}


/**
 *
 * This routine generates an interest rate scenario.
 * Arguments:
 * scenNumber     determines the random number seed for stochastic scenarios
 * testScenario   determines whether to generate one of the stochastic
 *                exclusion test scenarios.
 */
template <typename Generator>
void IntScenario::Generate(int scenNumber, bool testScenario, actlib::vector<double> initialRateCurve, int ProjectionYears, ScenarioGeneratorParams params, Generator& m_RNG)
{
    // Items used in the interpolation of yield curves for the Stochastic Exclusion Test
    actlib::vector<double> initialCurveFit(Range{ .lo = 1, .hi = 10 });        // Variances between NS fitted curve and actual initial curve, by duration

    numCurves = ProjectionYears * 12;  // Number of yield curves generate

    // Set up the arrays to generate
    actlib::table<double> randNums (numCurves, 3);
    curves = actlib::vector<YieldCurve>(numCurves + 1);

    // Initialize the starting yield curve
    curves(0) = YieldCurve();

    curves(0).Initialize(params.int_params.initial_short_rate, params.int_params.initial_long_rate, log(params.int_params.initial_volatility));

    if (true)  // If (testScenario) Then
    {
        curves(0).interpolateNS();

        for (auto i = 1; i <= 10; i++)
            initialCurveFit(i) = initialRateCurve(i) - curves(0).rateAtIndex(i);

         curves(0).perturb(initialCurveFit, 1);
    }



    // Generate the random numbers*****************************
    if (testScenario)
    {
        for (auto i = 1; i <= numCurves; i++)
        {
            randNums(i, 1) = testShock(scenNumber, i, LongIntShock);
            randNums(i, 2) = testShock(scenNumber, i, IntDiffShock);
            randNums(i, 3) = 0;

            randNums(i, 2) = randNums(i, 1) * params.correl12 + randNums(i, 2) * params.const1;
        }
    }
    else
    {
        m_RNG.Reseed(scenNumber - 1 + 200); // Re-seed the generator based on scenario number

        for (auto i = 0; i < numCurves; i++)
        {
            // First generate the uncorrelated random numbers
            for (auto j = 0; j < 3; j++)
                randNums(i, j) = InverseNormal(m_RNG.GetNext());
                
            // Now apply formulas to correlate the random numbers
            // The order of the next two lines was reversed before version 7.1,
            // leading to incorrect correlations if the user overrode default parameters
            // and specified non-zero correlations between the third random number and the other two.
            randNums(i, 2) = randNums(i, 0) * params.correl13 + randNums(i, 1) * params.const2 + randNums(i, 2) * params.const3;
            randNums(i, 1) = randNums(i, 0) * params.correl12 + randNums(i, 1) * params.const1;
            // The random numbers are now a correlated sample
        }
    }
    
    // Generate future yield curves *****************************

    // First initialize the prior month rates
    double oldShortRate = curves(0).rateAtIndex(3);
    double oldLogLongRate = log(curves(0).rateAtIndex(9));
    double oldDiff = curves(0).rateAtIndex(9) - curves(0).rateAtIndex(3);
    double oldLogVol = curves(0).getLogVolatility();

    // Initialize the soft cap and floor on the log long rate
    double minLogLongRate = log(params.int_params.min_long_rate);
    double maxLogLongRate = log(params.int_params.max_long_rate);
    double minShortRate = params.int_params.min_short_rate;


    // Loop by month generating new rates
    for (auto i = 0; i < numCurves; i++)
    {
        // Update the log volatility
        double newLogVol = (1 - params.int_params.beta3) * oldLogVol + params.const4 + randNums(i, 2) * params.int_params.sigma3;

        // Apply soft cap and floor on the long rate
        double oldLongRate = exp(oldLogLongRate);

        // Compute the new long rate
        // (application of soft cap moved from before this calculation to just before adding the random shock February 2016)
        double newLogLongRate = std::max(minLogLongRate, std::min(maxLogLongRate, (1 - params.int_params.beta1) * oldLogLongRate + params.const5 + params.int_params.psi * (params.int_params.tau2 - oldDiff))) + (exp(newLogVol) * randNums(i, 0));
        double newLongRate = exp(newLogLongRate);

        // Compute the new short rate
        double newDiff = (1 - params.int_params.beta2) * oldDiff + params.int_params.beta2 * params.int_params.tau2 + params.int_params.phi * (oldLogLongRate - log(params.int_params.tau1)) + params.int_params.sigma2 * randNums(i, 1) * pow(oldLongRate, params.int_params.theta);
        double newShortRate = newLongRate - newDiff;

        if (newShortRate < minShortRate)
            newShortRate = minShortRate; // = kappa * newLongRate

        // Save the new yield curve********************************
        curves(i) = YieldCurve();
        curves(i).Initialize(newShortRate, newLongRate, newLogVol);
        
        // Note that Initialize carries out the interpolation of the 10-point curve
        // using the Nelson-Siegel formula.

        // During the first 12 months, make adjustments for smooth fit to the initial curve
        if (i < 12)
            curves(i).perturb(initialCurveFit, (12.0 - i) / 12.0);

        // Since perturb() enforces no negative interest rates, call it for later months too.
        else
            curves(i).perturb(initialCurveFit, 0);


        // Set up for the next month
        oldShortRate   = newShortRate;
        oldLongRate    = newLongRate;
        oldLogLongRate = newLogLongRate;
        oldDiff        = newDiff;
        oldLogVol      = newLogVol;
    }
}


string IntScenario::serializeToJson() const
{
    auto curveToString = [&](int curveNum) -> string
    {
        string s;
        s += "\"" + std::to_string(curveNum) + "\"";
        s += ":";
        s += curves(curveNum).serializeToJson();

        return s;
    };

    string json;

    json += "{";

    for (auto month = curves.lower_bound(); month < curves.upper_bound(); month++)
    {
        json += curveToString(month);
        json += ",\n";
    }

    json += curveToString(curves.upper_bound());

    json += "}\n";

    return json;
}
