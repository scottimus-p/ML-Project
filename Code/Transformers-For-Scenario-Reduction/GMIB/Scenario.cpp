#include "Scenario.h"

#include <string>
#include <vector>

using std::string;
using std::vector;


vector<double> get_returns(const json& data, int num_months, string fund_name)
{
    vector<double> returns(num_months, 0);

    for (auto m = 0; m < returns.size(); m++)
    {
        returns[m] = data["equities"][fund_name][m];
    }

    return returns;
}

Scenario::Scenario(const json& data, int num_months) :
    DiversifiedFund      (FundReturns(get_returns(data, num_months, "USDiversified"))),
    InternationalFund    (FundReturns(get_returns(data, num_months, "International"))),
    AggressiveFund       (FundReturns(get_returns(data, num_months, "Aggressive"))),
    IntermediateRiskFund (FundReturns(get_returns(data, num_months, "Intermediate"))),
    MoneyFund            (FundReturns(get_returns(data, num_months, "MoneyMkt"))),
    IntGovtFund          (FundReturns(get_returns(data, num_months, "MedGovt"))),
    LongCorpFund         (FundReturns(get_returns(data, num_months, "LongCorp")))
{
}
