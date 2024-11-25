#include "YieldCurve.h"

#include <cmath>

YieldCurve::YieldCurve() :
    interpolatedRates(Range{ .lo = 1, .hi = 10 }),
    generatedRates(Range{ .lo = 1, .hi = 2 }),
    spotRates(Range{ .lo = 1, .hi = 10 }),
    maturities(Range{ .lo = 1, .hi = 10 })
{

}

double YieldCurve::generatedRate(int i) const
{
    // Retrieves one of the two generated rates (short=1, long=2)
    return generatedRates(i);
}

double YieldCurve::spotRateAtIndex(int index)
{
    // Retrieves the spot rate at one of the 10 index maturities
    if (!spotRatesAvailable)
        calcSpotRates();

    return spotRates(index);
}

double YieldCurve::rateAtIndex(int index) const
{
    // Retrieves one of the 10 interpolated rates
    return interpolatedRates(index);
}

double YieldCurve::rateAtMaturity(double maturityYrs) const
{
    // Linearly interpolates a rate given a maturity in years, based on rates at index maturities
    double rate;

    if (maturityYrs < 0.25)
        rate = interpolatedRates(1);
    else if (maturityYrs < 0.5)
        rate = rateAtIndex(1) + ((maturityYrs - 0.25) / 0.25) * (interpolatedRates(2) - interpolatedRates(1));
    else if (maturityYrs < 1)
        rate = rateAtIndex(2) + ((maturityYrs - 0.5) / 0.5) * (interpolatedRates(3) - interpolatedRates(2));
    else if (maturityYrs < 2)
        rate = rateAtIndex(3) + ((maturityYrs - 1) / 1) * (interpolatedRates(4) - interpolatedRates(3));
    else if (maturityYrs < 3)
        rate = rateAtIndex(4) + ((maturityYrs - 2) / 1) * (interpolatedRates(5) - interpolatedRates(4));
    else if (maturityYrs < 5)
        rate = rateAtIndex(5) + ((maturityYrs - 3) / 2) * (interpolatedRates(6) - interpolatedRates(5));
    else if (maturityYrs < 7)
        rate = rateAtIndex(6) + ((maturityYrs - 5) / 2) * (interpolatedRates(7) - interpolatedRates(6));
    else if (maturityYrs < 10)
        rate = rateAtIndex(7) + ((maturityYrs - 7) / 3) * (interpolatedRates(8) - interpolatedRates(7));
    else if (maturityYrs < 20)
        rate = rateAtIndex(8) + ((maturityYrs - 10) / 10) * (interpolatedRates(9) - interpolatedRates(8));
    else if (maturityYrs < 30)
        rate = rateAtIndex(9) + ((maturityYrs - 20) / 10) * (interpolatedRates(10) - interpolatedRates(9));
    else
        rate = interpolatedRates(10);

    return rate;
}


double YieldCurve::getLogVolatility() const
{
    return logVolatility;
}


void YieldCurve::Initialize(double shortRate, double longRate, double logVol)
{
    /***************************************** floor the generated rates at 0.0001 = 0.01% *****************/
    generatedRates(1) = std::max(shortRate, 0.0001);
    generatedRates(2) = std::max(longRate, 0.0001);
    logVolatility = logVol;
    /***************************************** floor the generated rates at 0.0001 = 0.01% *****************/

    maturities(1) = 0.25;
    maturities(2) = 0.5;
    maturities(3) = 1;
    maturities(4) = 2;
    maturities(5) = 3;
    maturities(6) = 5;
    maturities(7) = 7;
    maturities(8) = 10;
    maturities(9) = 20;
    maturities(10) = 30;

    interpolateNS();

    spotRatesAvailable = false;
}


void YieldCurve::interpolate(HistCurves& HistData)
{
    actlib::vector<double> histRates(Range({ .lo = 1, .hi = 10 }));
    long histCurveIndex;
    double shortRatio, longRatio;

    histCurveIndex = HistData.BestFittingCurve(generatedRates(1), generatedRates(2), generatedRates(3), 2);

    for (int i = histRates.lower_bound(); i <= histRates.upper_bound(); i++)
        histRates(i) = HistData.histCurveRateByIndex(histCurveIndex, i);


    shortRatio = generatedRates(1) / histRates(3);
    longRatio = generatedRates(3) / histRates(9);

    for (auto i = 1; i <= 3; i++)
        interpolatedRates(i) = histRates(i) * shortRatio;

    for (auto i = 4; i <= 8; i++)
        interpolatedRates(i) = histRates(i) * (shortRatio + (longRatio - shortRatio) * (maturities(i) - maturities(3)) / 19);

    for (auto i = 9; i <= 10; i++)
        interpolatedRates(i) = histRates(i) * longRatio;

    // Prevent negative rates
    for (auto i = 1; i <= 10; i++)
        interpolatedRates(i) = std::max(0.0001, interpolatedRates(i));
}


void YieldCurve::interpolateNS()
{
    // Use Nelson-Siegel two point interpolation

    double t;

    double r1 = generatedRates(1);
    double r20 = generatedRates(2);

    double k = 0.4;
    double const1 = (1 - exp(-k * 1)) / (k * 1);
    double const20 = (1 - exp(-k * 20)) / (k * 20);

    double b1 = (r1 - r20) / (const1 - const20);
    double b0 = r1 - (b1 * const1);

    for (auto i = 1; i <= 10; i++)
    {
        t = maturities(i);
        if (t == 0)
            t = 0.25;

        interpolatedRates(i) = b0 + b1 * (1 - exp(-k * t)) / (k * t);
    }
}


void YieldCurve::perturb(actlib::vector<double> adj, double portion)
{
    // The argument array must have 10 points.  It is used to adjust
    // each of the 10 points on the interpolated yield curve.
    // This is used in the first 12 months for scenarios in the
    // stochastic exclusion test, to reflect the shape of the
    // initial yield curve.

    for (auto i = 1; i <= 10; i++)
    {
        interpolatedRates(i) = interpolatedRates(i) + portion * adj(i);
        // Now enforce no negative interest rates
        interpolatedRates(i) = std::max(0.0001, interpolatedRates(i));
    }
}


void YieldCurve::calcSpotRates()
{
    // This routine fills in spot rates
    // based on the treasury bond rates
    actlib::vector<double> b(60);
    actlib::vector<double> s(60);
    double annuityFactor;

    // Fill in the bond curve

    b(0) = interpolatedRates(1);
    b(1) = interpolatedRates(2);
    b(2) = interpolatedRates(3);
    b(3) = 0.5 * (interpolatedRates(3) + interpolatedRates(4));
    b(4) = interpolatedRates(4);
    b(5) = 0.5 * (interpolatedRates(4) + interpolatedRates(5));
    b(6) = interpolatedRates(5);

    for (auto i = 1; i <= 4; i++)
        b(6 + i) = interpolatedRates(5) + (i * 0.25 * (interpolatedRates(6) - interpolatedRates(5)));

    for (auto i = 1; i <= 4; i++)
        b(10 + i) = interpolatedRates(6) + (i * 0.25 * (interpolatedRates(7) - interpolatedRates(6)));


    for (auto i = 1; i <= 6; i++)
        b(14 + i) = interpolatedRates(7) + ((i / 6) * (interpolatedRates(8) - interpolatedRates(7)));


    for (auto i = 1; i <= 20; i++)
        b(20 + i) = interpolatedRates(8) + ((i / 20) * (interpolatedRates(9) - interpolatedRates(8)));


    for (auto i = 1; i <= 20; i++)
        b(40 + i) = interpolatedRates(9) + ((i / 20) * (interpolatedRates(10) - interpolatedRates(9)));


    // Turn the bond rates into half-year rates
    for (auto i = 0; i <= 60; i++)
        b(i) = b(i) / 2;


    // Calculate spot rates from bond rates

    // use the bond yields to compute spot rates at half-year intervals
    s(0) = b(0);
    s(1) = b(1);
    annuityFactor = 1 / (1 + s(1));
    for (auto i = 2; i <= 60; i++)
    {
        double pvFactor = (1 - b(i) * annuityFactor) / (1 + b(i));
        if (pvFactor > 0)
            s(i) = pow(1 / pvFactor, (1 / i)) - 1;

        annuityFactor = annuityFactor + pvFactor;
    }

    // fill in the spot rate array with annual-effective rates (not bond-equivalent)
    spotRates(1) = pow(1 + s(0), 2) - 1;
    spotRates(2) = pow(1 + s(1), 2) - 1;
    spotRates(3) = pow(1 + s(2), 2) - 1;
    spotRates(4) = pow(1 + s(4), 2) - 1;
    spotRates(5) = pow(1 + s(6), 2) - 1;
    spotRates(6) = pow(1 + s(10), 2) - 1;
    spotRates(7) = pow(1 + s(14), 2) - 1;
    spotRates(8) = pow(1 + s(20), 2) - 1;
    spotRates(9) = pow(1 + s(40), 2) - 1;
    spotRates(10) = pow(1 + s(60), 2) - 1;

    spotRatesAvailable = true;

}

string YieldCurve::serializeToJson() const
{
    string json;

    json += "{";

    auto maturityToString = [&](int idx) -> string
    {
        string s;
        s += "\"" + std::to_string(maturities(idx)) + "\"";
        s += ":";
        s += std::to_string(rateAtIndex(idx));

        return s;
    };

    for (auto i = maturities.lower_bound(); i < maturities.upper_bound(); i++)
    {
        json += maturityToString(i);
        json += ",";// ",\n";
    }

    json += maturityToString(maturities.upper_bound());

    json += "}";

    return json;
}