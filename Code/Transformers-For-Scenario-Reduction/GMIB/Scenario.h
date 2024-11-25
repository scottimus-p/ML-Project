#pragma once

#include <vector>

#include "json.hpp"
#include "FundType.h"

using json = nlohmann::json;
using std::vector;

class FundReturns
{
    vector<double> returns;

public:

    explicit FundReturns(const vector<double>& r) :
        returns(r)
    {}

    explicit FundReturns(vector<double>&& r) :
        returns(std::move(r))
    {}

    [[nodiscard]] double get_return(int month) const
    {
        return returns[month];
    }
};


class Scenario
{
    FundReturns DiversifiedFund;
    FundReturns InternationalFund;
    FundReturns IntermediateRiskFund;
    FundReturns AggressiveFund;
    FundReturns MoneyFund;
    FundReturns IntGovtFund;
    FundReturns LongCorpFund;

public:

    Scenario() = default;
    Scenario(const json &data, int num_months);

    [[nodiscard]] double get_monthly_return(FundType fund, int month) const
    {
        switch (fund)
        {
        case FundType::DIVERSIFIED:       return DiversifiedFund.get_return(month);
        case FundType::INTERNATIONAL:     return InternationalFund.get_return(month);
        case FundType::INTERMEDIATE:      return IntermediateRiskFund.get_return(month);
        case FundType::AGGRESSIVE:        return AggressiveFund.get_return(month);
        case FundType::MONEY_MARKET:      return MoneyFund.get_return(month);
        case FundType::GOVT_INTERMEDIATE: return IntGovtFund.get_return(month);
        case FundType::CORPORATE_LONG:    return LongCorpFund.get_return(month);

        default: throw;
        }
    }
};

