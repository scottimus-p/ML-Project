#include "EquityFundReturn.h"

#include <cmath>
#include <algorithm>

double EquityFundReturn::getNextReturn(double returnShock, double volShock)
{
    double currLogVol;
    double targetLogVol;
    double nextReturn;
    double nextMeanReturn;

    currLogVol = log(currentVol);
    targetLogVol = log(targetVol);

    // First update the volatility
    currLogVol = (1 - meanRevStrength) * currLogVol + (meanRevStrength * targetLogVol);
    currentVol = exp(currLogVol);
    currentVol = std::min(maxVolBefore, currentVol);
    currLogVol = log(currentVol);

    currLogVol = currLogVol + volShock * volStdDev;
    currentVol = exp(currLogVol);
    currentVol = std::clamp(currentVol, minVol, maxVolAfter);

    // Next compute the log return based on the volatility and shock
    nextMeanReturn = a + b * currentVol + C * currentVol * currentVol;
    nextReturn     = (nextMeanReturn / 12) + returnShock * (currentVol / sqrt(12.));

    return exp(nextReturn) - 1;
}


double EquityFundReturn::getNextReturnSET(double returnShock)
{
    double constexpr ONE_TWELFTH = 1. / 12;

    return (pow(1. + SETmedianReturn, (ONE_TWELFTH)) - 1) + returnShock * (SETvolatility / (sqrt(12.)));
}