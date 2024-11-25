#pragma once

#include <string>

#include "Vector.h"

#include "C3RNG.h"
#include "ScenarioGeneratorParams.hpp"
#include "YieldCurve.h"

using std::string;


/**
 * This class represents a single interest rate scenario.
 * The "Generate" routine generates the scenario entirely in VBA code.
 * The scenario generation algrorithm parallels that shown on the
 * "Single Scenario" tab of the workbook.  Parameters are taken from
 * the user input to the workbook.
 *
 * This scenario class includes the random numbers used, the generated
 * short and long rates, and the 10-point interpolated yield curve.
*/

class IntScenario
{
    actlib::vector<YieldCurve> curves;
    int numCurves;

public:

    YieldCurve& curve(int curveNum);

    double significance();

    template <typename Generator>
    void Generate(int scenNumber, bool testScenario, actlib::vector<double> initialRateCurve, int ProjectionYears, ScenarioGeneratorParams params, Generator& m_RNG);

    _NODISCARD string serializeToJson() const;
};

template void IntScenario::Generate<MersenneTwister>(int scenNumber, bool testScenario, actlib::vector<double> initialRateCurve, int ProjectionYears, ScenarioGeneratorParams params, MersenneTwister& m_RNG);