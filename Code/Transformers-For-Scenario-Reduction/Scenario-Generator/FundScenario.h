#pragma once

#include <string>

#include "Vector.h"
#include "Cholesky.h"
#include "IntScenario.h"
#include "C3RNG.h"
#include "EquityFundReturn.h"
#include "FixedFundReturn.h"

using std::string;

class FundScenario
{
    actlib::table<double> returns;
    int numMonths;

    enum FundType
    {
        USDiversified,
        International,
        Intermediate,
        Aggressive,
        MoneyMkt,
        MedGovt,
        LongCorp,

        First = USDiversified,
        Last = LongCorp
    };

    constexpr static string FundTypeToString(FundType f)
    {
        switch (f)
        {
        case USDiversified: return "USDiversified";
        case International: return "International";
        case Intermediate: return "Intermediate";
        case Aggressive: return "Aggressive";
        case MoneyMkt: return "MoneyMkt";
        case MedGovt: return "MedGovt";
        case LongCorp: return "LongCorp";

        default: throw;
        }
    }

    Cholesky correlator;

public:

    double wealthFactor(int monthNum, int n) const;
    double totalReturn(int monthNum, int n) const;

    double randNum(int monthNum, int n);

    template <typename Generator>
    void Generate(int scenNumber, IntScenario intScenario, bool testScenario, int ProjectionYears,
                  actlib::table<double> CorrelationMatrix, Generator& m_RNG, EquityFundReturn DiversifiedFund,
                  EquityFundReturn InternationalFund, EquityFundReturn IntermediateRiskFund, EquityFundReturn AggressiveFund,
                  FixedFundReturn MoneyFund, FixedFundReturn IntGovtFund, FixedFundReturn LongCorpFund);

    _NODISCARD string serializeToJson() const;
};

template void FundScenario::Generate<MersenneTwister>(int scenNumber, IntScenario intScenario, bool testScenario, int ProjectionYears,
    actlib::table<double> CorrelationMatrix, MersenneTwister& m_RNG, EquityFundReturn DiversifiedFund,
    EquityFundReturn InternationalFund, EquityFundReturn IntermediateRiskFund, EquityFundReturn AggressiveFund,
    FixedFundReturn MoneyFund, FixedFundReturn IntGovtFund, FixedFundReturn LongCorpFund);
