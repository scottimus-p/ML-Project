#pragma once

#include "ScenarioGeneratorParams.hpp"
#include "YieldCurve.h"

/**
 * This module keeps the parameters for the return generator for a fixed investment fund.
 * It also provides a function to generate the return for the next period,
 * given previous and current yield curves and a random shock.
 */

class FixedFundReturn
{
    double maturity      {};  // maturity in years for bonds in this fund
    double monthlyFactor {};  // usually 1/12 = 0.0833333
    double monthlySpread {};
    double duration      {};  // measured in years
    double volatility    {};  // volatility due to credit spreads

public:
    double getNextReturn(YieldCurve prevYldCurve, YieldCurve currYldCurve, double shock);

    FixedFundReturn() = default;

    FixedFundReturn(FixedFundParams params) :
        maturity(params.maturity),
        monthlyFactor(params.monthlyFactor),
        monthlySpread(params.monthlySpread),
        duration(params.duration),
        volatility(params.volatility)
    {}

};

