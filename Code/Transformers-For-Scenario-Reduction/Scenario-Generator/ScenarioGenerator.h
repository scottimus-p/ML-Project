#pragma once

//#include <map>
#include <string>
#include <vector>

#include "Vector.h"

#include "C3RNG.h"
#include "HistCurves.h"
#include "EquityFundReturn.h"
#include "FixedFundReturn.h"
#include "IntScenario.h"
#include "FundScenario.h"
#include "ScenarioGeneratorParams.hpp"


using std::map;
using std::string;
using std::vector;

double InverseNormal(double p);

enum class Frequency
{
    ANNUAL,
    SEMIANNUAL,
    QUARTERLY,
    MONTHLY
};


inline int periods_per_year(Frequency f)
{
    switch (f)
    {
    case Frequency::ANNUAL:     return 1;
    case Frequency::SEMIANNUAL: return 2;
    case Frequency::QUARTERLY:  return 4;
    case Frequency::MONTHLY:    return 12;
    default: throw;
    }
}


class ScenarioGenerator
{
    HistCurves HistData;  // Object containing all historical yield curves

    ScenarioGeneratorParams params;

    // Rounding level
    int precision;

    // Time step as a number of months
    int timeStepMonths;

    EquityFundReturn DiversifiedFund;
    EquityFundReturn InternationalFund;
    EquityFundReturn IntermediateRiskFund;
    EquityFundReturn AggressiveFund;

    FixedFundReturn MoneyFund;
    FixedFundReturn IntGovtFund;
    FixedFundReturn LongCorpFund;


    // 10 Yield Curve points (3m, 6m, 1, 2, 3, 5, 7, 10, 20, 30 yrs)
    static constexpr int YieldPoints = 10;

    // Number of stochastic processes in the interest rate generator
    static constexpr int NumProcesses = 3;

    IntScenario intScenario;
    FundScenario fundScenario;

    int scenarioCount;

    bool StartingCurveOK(Date startingDate, const map<double, double>& startingCurve, const map<Date, map<double, double>>& historicalData) const;

    bool isFilenameSuffixValid(string filenameSuffix) const;

    void MeanReversionPointUpdate(Date startDate, map<Date, double> NaicMeanRevPoints);

    void SetEquityVolatilities(double DiversifiedVol, double InternationalVol, double IntermediateVol, double AggressiveVol);

    void GenerateSingleScenario(int scn_number, Frequency projFrequency, int ProjectionYears, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, const string& output_dir);
    void GenerateScenarioRange(int start_scn_number, int end_scn_number, Frequency projFrequency, int ProjectionYears, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, const string& output_dir);

    string scenarioToJsonString(int scenarioNumber) const;

public:

    void generateAllScenarios(Frequency projFrequency, int ProjectionYears, int num_scenarios, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, int num_threads, const string& output_dir);

    void writeScenarioToFile(int scenarioNumber, string filename) const;
};

