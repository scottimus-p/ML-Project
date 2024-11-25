#pragma once

#include "ScenarioGeneratorParams.hpp"

/**
 * This module keeps the parameters for the return generator for a class of equities.
 * It also keeps the current state (volatility) of an equity class and provides
 * a function to generate the return for the next period, given the current state
 * and shocks to the return and to its volatility.
 */

class EquityFundReturn
{
    double targetVol {};
    double minVol {};
    double maxVolBefore {};
    double maxVolAfter {};
    double meanRevStrength {};
    double volStdDev {};
    double a {};
    double b {};
    double C {};

    double SETmedianReturn {};
    double SETvolatility {};

public:

    double currentVol {};

    double getNextReturn(double returnShock, double volShock);
    double getNextReturnSET(double returnShock);

    EquityFundReturn(EquityFundParams params) :
    targetVol(params.targetVol),
    minVol(params.minVol),
    maxVolBefore(params.maxVolBefore),
    maxVolAfter(params.maxVolAfter),
    meanRevStrength(params.meanRevStrength),
    volStdDev(params.volStdDev),
    a(params.a),
    b(params.b),
    C(params.C),
    SETmedianReturn(params.SETmedianReturn),
    SETvolatility(params.SETvolatility),
    currentVol(params.current_vol)
    {}

    EquityFundReturn() = default;

};

