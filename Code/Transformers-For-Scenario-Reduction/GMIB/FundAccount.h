#pragma once

#include <array>
#include <numeric>

#include "FundType.h"
#include "Scenario.h"

using std::array;


class IndividualFund
{

public:

    double FundValue;
    const FundType Type;

    IndividualFund(FundType type) :
        Type(type),
        FundValue(0)
    {

    }
};


class FundAccount
{
    IndividualFund DiversifiedFund;
    IndividualFund InternationalFund;
    IndividualFund IntermediateRiskFund;
    IndividualFund AggressiveFund;

    IndividualFund MoneyFund;
    IndividualFund IntGovtFund;
    IndividualFund LongCorpFund;

    constexpr static int NUM_FUNDS = 7;

    array<IndividualFund, NUM_FUNDS> funds {DiversifiedFund, InternationalFund, IntermediateRiskFund, AggressiveFund, MoneyFund, IntGovtFund, LongCorpFund};

    array<double, NUM_FUNDS> TargetAllocation;
    bool UseTargetAllocation;

public:

    FundAccount() :
        UseTargetAllocation (false),
        DiversifiedFund     (FundType::DIVERSIFIED),
        InternationalFund   (FundType::INTERNATIONAL),
        IntermediateRiskFund(FundType::INTERMEDIATE),
        AggressiveFund      (FundType::AGGRESSIVE),
        MoneyFund           (FundType::MONEY_MARKET),
        IntGovtFund         (FundType::GOVT_INTERMEDIATE),
        LongCorpFund        (FundType::CORPORATE_LONG)
    {
        TargetAllocation.fill(0);
    }

    void add_deposit(double amount)
    {
        for (auto& f : funds)
            f.FundValue += amount / NUM_FUNDS;
    }

    void add_depsoit(double amount, array<double, NUM_FUNDS> deposit_allocation)
    {
        for (int i = 0; i < NUM_FUNDS; i++)
            funds[i].FundValue += amount * deposit_allocation[i];
    }

    void rollforward_funds_this_month(int month, const Scenario& scenario)
    {
        for (auto& f : funds)
            f.FundValue *= (1 + scenario.get_monthly_return(f.Type, month));
    }

    _NODISCARD double get_total_fund_value() const
    {
        return std::reduce(funds.cbegin(), funds.cend(), 0., [&](double d, IndividualFund f) -> double {
            return d + f.FundValue;
        });
    }
};

