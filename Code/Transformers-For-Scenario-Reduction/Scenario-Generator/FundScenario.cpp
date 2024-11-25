#include "FundScenario.h"

#include "StochasticExclusionTest.h"
#include "ScenarioGenerator.h"

double FundScenario::wealthFactor(int monthNum, int n) const
{
    return returns(n, monthNum);
}


double FundScenario::totalReturn(int monthNum, int n) const
{
    return returns(n, monthNum) / returns(n, monthNum - 1) - 1;
}


double FundScenario::randNum(int monthNum, int n)
{
    // This should only be called after Generate() has been executed.
    // The return value is from the correlated random sample
    return correlator.corrNum(n, monthNum);
}


template <typename Generator>
void FundScenario::Generate(int scenNumber, IntScenario intScenario, bool testScenario, int ProjectionYears, 
                            actlib::table<double> CorrelationMatrix, Generator& m_RNG, EquityFundReturn DiversifiedFund,
                            EquityFundReturn InternationalFund, EquityFundReturn IntermediateRiskFund, EquityFundReturn AggressiveFund,
                            FixedFundReturn MoneyFund, FixedFundReturn IntGovtFund, FixedFundReturn LongCorpFund)
{
    numMonths = ProjectionYears * 12;  // Number of yield curves generate

    // Set up the array of returns to generate and initialize first value to 1.0
    returns = actlib::table<double>(9, numMonths + 1);

    for (auto j = 0; j <= 8; j++)
        returns(j, 0) = 1;

    // Set up the random number correlator
    correlator.setup(11, numMonths, CorrelationMatrix);


    // Generate the uncorrelated random numbers
    // Re-seed the generator based on scenario number
    m_RNG.Reseed(scenNumber - 1 + 10200);

    for (auto i = 0; i < numMonths; i++)
    {
        for (auto j = 0; j <= 10; j++)
            correlator.SetRandNum(j, i, InverseNormal(m_RNG.GetNext()));
    }

    // Use Cholesky decomposition to correlate the random samples
    correlator.correlate();

    // Loop by month generating new rates
    if (testScenario)
    {
        for (auto i = 1; i <= numMonths; i++)
        {
            returns(0, i) = DiversifiedFund.getNextReturnSET(testShock(scenNumber, i, EquityShock));
            returns(1, i) = InternationalFund.getNextReturnSET(testShock(scenNumber, i, EquityShock));
            returns(2, i) = IntermediateRiskFund.getNextReturnSET(testShock(scenNumber, i, EquityShock));
            returns(3, i) = AggressiveFund.getNextReturnSET(testShock(scenNumber, i, EquityShock));

            // The prior and current yield curves are needed here
            YieldCurve priorCurve = intScenario.curve(i - 1);
            YieldCurve currentCurve = intScenario.curve(i);

            returns(4, i) = MoneyFund.getNextReturn(priorCurve, currentCurve, 0);
            returns(5, i) = IntGovtFund.getNextReturn(priorCurve, currentCurve, 0);
            returns(6, i) = LongCorpFund.getNextReturn(priorCurve, currentCurve, 0);

            returns(7, i) = 0.65 * returns(5, i) + 0.35 * returns(6, i);  // Blended FIXED fund
            returns(8, i) = 0.6 * returns(0, i) + 0.4 * returns(7, i);    // Blended BALANCED fund

            for (auto j = 0; j <= 8; j++)  // convert each return into a cumulative wealth factor
            {
                returns(j, i) = returns(j, i - 1) * (1 + returns(j, i));
            }
        }
    }
    else
    {
        for (auto i = 1; i <= numMonths; i++)
        {
            int k = i - 1; // used to index into random number array
            returns(0, i) = DiversifiedFund.getNextReturn(randNum(k, 1), randNum(k, 0));
            returns(1, i) = InternationalFund.getNextReturn(randNum(k, 3), randNum(k, 2));
            returns(2, i) = IntermediateRiskFund.getNextReturn(randNum(k, 5), randNum(k, 4));
            returns(3, i) = AggressiveFund.getNextReturn(randNum(k, 7), randNum(k, 6));

            // The prior and current yield curves are needed here
            YieldCurve priorCurve = intScenario.curve(i - 1);
            YieldCurve currentCurve = intScenario.curve(i);

            returns(4, i) = MoneyFund.getNextReturn(priorCurve, currentCurve, randNum(k, 8));
            returns(5, i) = IntGovtFund.getNextReturn(priorCurve, currentCurve, randNum(k, 9));
            returns(6, i) = LongCorpFund.getNextReturn(priorCurve, currentCurve, randNum(k, 10));

            returns(7, i) = 0.65 * returns(5, i) + 0.35 * returns(6, i);  // Blended FIXED fund
            returns(8, i) = 0.6 * returns(0, i) + 0.4 * returns(7, i);    // Blended BALANCED fund

            for (auto j = 0; j <= 8; j++)  // convert each return into a cumulative wealth factor
                returns(j, i) = returns(j, i - 1) * (1 + returns(j, i));
        }
    }
}


string FundScenario::serializeToJson() const
{
    auto fundToString = [&](FundType f) -> string
    {
        string s;

        s += "\"" + FundTypeToString(FundType(f)) + "\"";

        s += ":";

        s += "[";

        for (int month = returns.lower_bound(2) + 1; month < returns.upper_bound(2); month++)
            s += std::to_string(totalReturn(month, f)) + ",";

        s += std::to_string(totalReturn(returns.upper_bound(2), f));

        s += "]";

        return s;
    };

    string json;

    json += "{";

    for (int fund = FundType::First; fund < FundType::Last; fund++)
    {
        json += fundToString(FundType(fund));
        json += ",\n";
    }

    json += fundToString(FundType(FundType::Last));

    json += "}";

    return json;
}
