#include "ScenarioGenerator.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include "HistCurves.h"

using std::ofstream;

map<Date, double> naicMeanReversionPoints;

// The inverse normal routine in Excel is poor -- this version is much better.
double InverseNormal(double p)
{
    // Adapted for Microsoft Visual Basic from Peter Acklam's
    // "An algorithm for computing the inverse normal cumulative distribution function"
    // (http://home.online.no/~pjacklam/notes/invnorm/)
    // by John Herrero (3-Jan-03)

    // Define coefficients in rational approximations
    constexpr double a1 = -39.6968302866538;
    constexpr double a2 = 220.946098424521;
    constexpr double a3 = -275.928510446969;
    constexpr double a4 = 138.357751867269;
    constexpr double a5 = -30.6647980661472;
    constexpr double a6 = 2.50662827745924;

    constexpr double b1 = -54.4760987982241;
    constexpr double b2 = 161.585836858041;
    constexpr double b3 = -155.698979859887;
    constexpr double b4 = 66.8013118877197;
    constexpr double b5 = -13.2806815528857;

    constexpr double c1 = -7.78489400243029E-03;
    constexpr double c2 = -0.322396458041136;
    constexpr double c3 = -2.40075827716184;
    constexpr double c4 = -2.54973253934373;
    constexpr double c5 = 4.37466414146497;
    constexpr double c6 = 2.93816398269878;

    constexpr double d1 = 7.78469570904146E-03;
    constexpr double d2 = 0.32246712907004;
    constexpr double d3 = 2.445134137143;
    constexpr double d4 = 3.75440866190742;

    // Define break-points
    constexpr double p_low = 0.02425;
    constexpr double p_high = 1 - p_low;

    // Define work variables

    // If argument out of bounds, raise error
    if (p <= 0 || p >= 1) throw;

    if (p < p_low)
    {
        // Rational approximation for lower region
        double q = sqrt(-2 * log(p));
        return (((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) / ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
    }
    else if (p <= p_high)
    {
        // Rational approximation for lower region
        double q = p - 0.5;
        double r = q * q;
        return (((((a1 * r + a2) * r + a3) * r + a4) * r + a5) * r + a6) * q / (((((b1 * r + b2) * r + b3) * r + b4) * r + b5) * r + 1);
    }
    else if (p < 1)
    {
        // Rational approximation for upper region
        double q = sqrt(-2 * log(1 - p));
        return -(((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) / ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
    }

    throw;
}


bool ScenarioGenerator::StartingCurveOK(Date startingDate, const map<double, double>& startingCurve, const map<Date, map<double, double>>& historicalData) const
{
    Date histDate;
    HistCurves histCurves;


    auto StartingCurveOKbbb = true;

    histCurves.Initialize(historicalData);
    histDate = startingDate;

    // Check whether the starting date is OK
    if (histDate < histCurves.getFirstDate() || histDate > histCurves.getLastDate())
        return false;
    else // Date is within historical curves period.
    {
        // Check whether the starting yield curve matches history.
        bool mismatch = false;

        if (startingCurve != histCurves.getCurveByDate(histDate))
            mismatch = true;

        if (mismatch)
            return false;

    }  // Date is within historical curves period'

    return true;
}


void ScenarioGenerator::GenerateSingleScenario(int scn_number, Frequency projFrequency, int ProjectionYears, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, const string& output_dir)
{
    thread_local MersenneTwister m_RNG;               // Random Number Generator

    SetEquityVolatilities(params.DiversifiedVol, params.InternationalVol, params.IntermediateVol, params.AggressiveVol);  // Resets the volatility for the equity fund scenarios that use stochastic volatility

    intScenario.Generate(scn_number, generateForStochExclTest, HistData.getCurveVecByDate(startDate),
        ProjectionYears, params, m_RNG);

    fundScenario.Generate(scn_number, intScenario, generateForStochExclTest, ProjectionYears,
                          CorrelationMatrix, m_RNG, DiversifiedFund, InternationalFund, IntermediateRiskFund,
                          AggressiveFund, MoneyFund, IntGovtFund, LongCorpFund);

    string outfilename (output_dir + "scenario_" + std::to_string(scn_number) + ".json");

    writeScenarioToFile(scn_number, outfilename);
}

void ScenarioGenerator::GenerateScenarioRange(int start_scn_number, int end_scn_number, Frequency projFrequency, int ProjectionYears, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, const string& output_dir)
{
    for (auto i = start_scn_number; i < end_scn_number; i++)
    {
        GenerateSingleScenario(i, projFrequency, ProjectionYears, startDate, generateForStochExclTest, useNaicMeanRevPoint, params, CorrelationMatrix, output_dir);
    }
}


void ScenarioGenerator::generateAllScenarios(Frequency projFrequency, int ProjectionYears, int num_scenarios, Date startDate, bool generateForStochExclTest, bool useNaicMeanRevPoint, ScenarioGeneratorParams params, const actlib::table<double>& CorrelationMatrix, int num_threads, const string& output_dir)
{
    // Used for file purposes - output fileNums
    actlib::vector<int> naYieldFileNums(YieldPoints + 9);  // last 9 are for fund returns
    actlib::vector<int> naRNG_FileNums(NumProcesses + 1);  // last 1 is for fund return shocks

    // nProjFrequency: 1=Annual, 2=S/A, 4=Quarterly, 12=Monthly (the default)
    int nProjFrequency = periods_per_year(projFrequency);


    // Update the mean reversion point based on the scenario start date
    if (useNaicMeanRevPoint) MeanReversionPointUpdate(startDate, naicMeanReversionPoints);

    // Save the start time for use when later determining total elapsed time.
    auto StartTime = std::chrono::system_clock::now();


    // Initialize the historical yield curve data to be used when interpolating generated curves
    // Set up an array used for headings in output files
    double vOutputMaturities[] = { 0.25, 0.5, 1, 2, 3, 5, 7, 10, 20, 30 };

    map<Date, map<double, double>> historicalData;

    constexpr Date April1953 {.month = APRIL, .year = 1953};

    load_historical_data(historicalData, April1953, 0.25, "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\0.25.csv");
    load_historical_data(historicalData, April1953, 0.5,  "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\0.5.csv");
    load_historical_data(historicalData, April1953, 1,    "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\1.csv");
    load_historical_data(historicalData, April1953, 2,    "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\2.csv");
    load_historical_data(historicalData, April1953, 3,    "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\3.csv");
    load_historical_data(historicalData, April1953, 5,    "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\5.csv");
    load_historical_data(historicalData, April1953, 7,    "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\7.csv");
    load_historical_data(historicalData, April1953, 10,   "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\10.csv");
    load_historical_data(historicalData, April1953, 20,   "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\20.csv");
    load_historical_data(historicalData, April1953, 30,   "C:\\Users\\scott\\source\\repos\\Scenario-Generator\\Historic-Curves\\30.csv");

    HistData.Initialize(historicalData);

    DiversifiedFund      = EquityFundReturn(params.diversified_params);
    InternationalFund    = EquityFundReturn(params.international_params);
    IntermediateRiskFund = EquityFundReturn(params.intermediate_params);
    AggressiveFund       = EquityFundReturn(params.aggressive_params);

    MoneyFund = FixedFundReturn(params.money_market);
    IntGovtFund = FixedFundReturn(params.us_intermed_govt);
    LongCorpFund = FixedFundReturn(params.us_long_corporate);

    if (num_threads == 1)
    {
        for (auto i = 1; i <= num_scenarios; i++)
        {
            std::cout << "Generating scenario " << i << "\n";
            GenerateSingleScenario(i, projFrequency, ProjectionYears, startDate, generateForStochExclTest, useNaicMeanRevPoint, params, CorrelationMatrix, output_dir);
        }
    }
    else
    {
        vector<std::thread> thread_pool(num_threads);

        // Loop to generate the scenarios and write results to output files
        int num_scenarios_per_thread = num_scenarios / num_threads;
        for (auto i = 0; i < num_threads; i ++)
        {
            int start_scn = i * num_scenarios_per_thread + 1;
            int end_scn   = std::min(start_scn + num_scenarios_per_thread, num_scenarios + 1);
        
            if (i == num_threads - 1)
                end_scn += num_scenarios % num_threads;

            thread_pool[i] = std::move(std::thread(&ScenarioGenerator::GenerateScenarioRange, this, start_scn, end_scn, projFrequency, ProjectionYears, startDate, generateForStochExclTest, useNaicMeanRevPoint, params, CorrelationMatrix, output_dir));
        }

        for (auto j = 0; j < num_threads; j++)
        {
            thread_pool[j].join();
        }
    }

    auto dEndTime = std::chrono::system_clock::now();

    std::cout << "Processing time = " << std::chrono::duration_cast<std::chrono::seconds>(dEndTime - StartTime).count() << " seconds" << std::endl;
}


string ScenarioGenerator::scenarioToJsonString(int scenarioNumber) const
{
    string s;

    s += "{";

    s += "\"equities\":" + fundScenario.serializeToJson() + /*"," +*/ "\n";
    //s += "\"yield_curves\":" + intScenario.serializeToJson() + "\n";

    s += "}";

    return s;
}

void ScenarioGenerator::writeScenarioToFile(int scenarioNumber, string filename) const
{
    ofstream file (filename, std::ios::out | std::ios::trunc);

    file << scenarioToJsonString(scenarioNumber);

    file.close();
}


void ScenarioGenerator::SetEquityVolatilities(double DiversifiedVol, double InternationalVol, double IntermediateVol, double AggressiveVol)
{
    DiversifiedFund.currentVol      = DiversifiedVol;
    InternationalFund.currentVol    = InternationalVol;
    IntermediateRiskFund.currentVol = InternationalVol;
    AggressiveFund.currentVol       = AggressiveVol;
}


void ScenarioGenerator::MeanReversionPointUpdate(Date startDate, map<Date, double> NaicMeanRevPoints)
{
    if (startDate.month > 12)
        throw;

    if (startDate.year < 1954)
        throw;
 
    params.int_params.tau1 = NaicMeanRevPoints.at(startDate);
}
