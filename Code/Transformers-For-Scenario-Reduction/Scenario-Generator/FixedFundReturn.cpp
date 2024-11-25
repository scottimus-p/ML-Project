#include "FixedFundReturn.h"

#include <cmath>

double FixedFundReturn::getNextReturn(YieldCurve prevYldCurve, YieldCurve currYldCurve, double shock)
{
    double prevIntRate = prevYldCurve.rateAtMaturity(maturity);
    double currIntRate = currYldCurve.rateAtMaturity(maturity);

    double currReturn = monthlyFactor * (prevIntRate + monthlySpread) + duration * (prevIntRate - currIntRate) + shock * sqrt(prevIntRate) * volatility;

    return currReturn;
}