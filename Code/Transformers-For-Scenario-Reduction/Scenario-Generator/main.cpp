#include "ScenarioGenerator.h"
#include <fstream>
#include <string>
#include "argparse.hpp"
#include "json.hpp"

using std::ifstream;
using std::string;

using json = nlohmann::json;


struct ScenarioGeneratorArgs : public argparse::Args
{
    string& out_path      = kwarg("output_dir", "directory to write output");
    string& param_file    = kwarg("param_file", "file with parameters for scenario generator");
    int& num_period       = kwarg("num_periods", "number of periods to generate");
    char& frequency       = kwarg("frequency", "the frequency to generate. 'a' for annual, 'q' for quarterly, 'm' for monthly", "m");
    bool& single_file     = flag("s,single_file", "a flag to write all output in a single file").set_default(false);
    int& num_scenarios    = kwarg("num_scenarios", "number of scenarios to generate");
    int& num_threads      = kwarg("threads,t", "number of threads").set_default(1);
};


int main(int argc, char** argv)
{
    auto args = argparse::parse<ScenarioGeneratorArgs>(argc, argv);

    args.print();

    ifstream param_file(args.param_file);
    json data = json::parse(param_file);

    ScenarioGeneratorParams params;

    params.correl12 = data["interest_rate_params"]["correl12"];
    params.correl13 = data["interest_rate_params"]["correl13"];
    params.correl23 = data["interest_rate_params"]["correl23"];

    params.int_params.beta1  = data["interest_rate_params"]["beta1"];
    params.int_params.beta2  = data["interest_rate_params"]["beta2"];
    params.int_params.beta3  = data["interest_rate_params"]["beta3"];
    
    params.int_params.tau1   = data["interest_rate_params"]["tau1"];
    params.int_params.tau2   = data["interest_rate_params"]["tau2"];
    params.int_params.tau3   = data["interest_rate_params"]["tau3"];
    
    params.int_params.sigma2 = data["interest_rate_params"]["sigma2"];
    params.int_params.sigma3 = data["interest_rate_params"]["sigma3"];
    
    params.int_params.psi    = data["interest_rate_params"]["psi"];
    params.int_params.phi    = data["interest_rate_params"]["phi"];
    params.int_params.theta  = data["interest_rate_params"]["theta"];
    params.int_params.kappa  = data["interest_rate_params"]["kappa"];

    params.int_params.initial_short_rate = data["interest_rate_params"]["init_rate_short"];
    params.int_params.initial_long_rate  = data["interest_rate_params"]["init_rate_long"];
    params.int_params.initial_volatility = data["interest_rate_params"]["init_vol"];

    params.int_params.min_long_rate  = data["interest_rate_params"]["min_long_rate"];
    params.int_params.max_long_rate  = data["interest_rate_params"]["max_long_rate"];
    params.int_params.min_short_rate = data["interest_rate_params"]["min_short_rate"];

    params.AggressiveVol    = data["equity_params"]["aggressive_vol"];
    params.DiversifiedVol   = data["equity_params"]["diversified_vol"];
    params.IntermediateVol  = data["equity_params"]["intermediate_vol"];
    params.InternationalVol = data["equity_params"]["international_vol"];

    params.update_consts();

    actlib::table<double> correlationMatrix (11, 11);

    vector<string> markets {"US_LogVol", "US_LogRet", "Intl_LogVol", "Intl_LogRet", "Small_LogVol", "Small_LogRet", "Aggr_LogVol", "Aggr_LogRet", "Money_Ret", "IT_Govt_Ret", "LTCorp_Ret"};

    int j = 0;
    for (auto i = 0; i < markets.size(); i++)
    {
        for (auto j = 0; j < markets.size(); j++)
        {
            correlationMatrix(i, j) = data["equity_correlations"][i][markets[i]][j][markets[j]];
        }
    }

    ScenarioGenerator scn_gen;

    params.diversified_params.targetVol       = data["equity_params"]["diversified"]["target_vol"];
    params.diversified_params.meanRevStrength = data["equity_params"]["diversified"]["mean_rev_strength"];
    params.diversified_params.volStdDev       = data["equity_params"]["diversified"]["vol_std_dev"];
    params.diversified_params.a               = data["equity_params"]["diversified"]["a"];
    params.diversified_params.b               = data["equity_params"]["diversified"]["b"];
    params.diversified_params.C               = data["equity_params"]["diversified"]["c"];
    params.diversified_params.current_vol     = data["equity_params"]["diversified"]["current_vol"];
    params.diversified_params.minVol          = data["equity_params"]["diversified"]["min_vol"];
    params.diversified_params.maxVolBefore    = data["equity_params"]["diversified"]["max_vol_before"];
    params.diversified_params.maxVolAfter     = data["equity_params"]["diversified"]["max_vol_after"];
    params.diversified_params.SETmedianReturn = data["equity_params"]["diversified"]["SETmedianReturn"];
    params.diversified_params.SETvolatility   = data["equity_params"]["diversified"]["SETvolatility"];

    params.international_params.targetVol       = data["equity_params"]["international"]["target_vol"];
    params.international_params.meanRevStrength = data["equity_params"]["international"]["mean_rev_strength"];
    params.international_params.volStdDev       = data["equity_params"]["international"]["vol_std_dev"];
    params.international_params.a               = data["equity_params"]["international"]["a"];
    params.international_params.b               = data["equity_params"]["international"]["b"];
    params.international_params.C               = data["equity_params"]["international"]["c"];
    params.international_params.current_vol     = data["equity_params"]["international"]["current_vol"];
    params.international_params.minVol          = data["equity_params"]["international"]["min_vol"];
    params.international_params.maxVolBefore    = data["equity_params"]["international"]["max_vol_before"];
    params.international_params.maxVolAfter     = data["equity_params"]["international"]["max_vol_after"];
    params.international_params.SETmedianReturn = data["equity_params"]["international"]["SETmedianReturn"];
    params.international_params.SETvolatility   = data["equity_params"]["international"]["SETvolatility"];

    params.intermediate_params.targetVol       = data["equity_params"]["intermediate"]["target_vol"];
    params.intermediate_params.meanRevStrength = data["equity_params"]["intermediate"]["mean_rev_strength"];
    params.intermediate_params.volStdDev       = data["equity_params"]["intermediate"]["vol_std_dev"];
    params.intermediate_params.a               = data["equity_params"]["intermediate"]["a"];
    params.intermediate_params.b               = data["equity_params"]["intermediate"]["b"];
    params.intermediate_params.C               = data["equity_params"]["intermediate"]["c"];
    params.intermediate_params.current_vol     = data["equity_params"]["intermediate"]["current_vol"];
    params.intermediate_params.minVol          = data["equity_params"]["intermediate"]["min_vol"];
    params.intermediate_params.maxVolBefore    = data["equity_params"]["intermediate"]["max_vol_before"];
    params.intermediate_params.maxVolAfter     = data["equity_params"]["intermediate"]["max_vol_after"];
    params.intermediate_params.SETmedianReturn = data["equity_params"]["intermediate"]["SETmedianReturn"];
    params.intermediate_params.SETvolatility   = data["equity_params"]["intermediate"]["SETvolatility"];

    params.aggressive_params.targetVol       = data["equity_params"]["aggressive"]["target_vol"];
    params.aggressive_params.meanRevStrength = data["equity_params"]["aggressive"]["mean_rev_strength"];
    params.aggressive_params.volStdDev       = data["equity_params"]["aggressive"]["vol_std_dev"];
    params.aggressive_params.a               = data["equity_params"]["aggressive"]["a"];
    params.aggressive_params.b               = data["equity_params"]["aggressive"]["b"];
    params.aggressive_params.C               = data["equity_params"]["aggressive"]["c"];
    params.aggressive_params.current_vol     = data["equity_params"]["aggressive"]["current_vol"];
    params.aggressive_params.minVol          = data["equity_params"]["aggressive"]["min_vol"];
    params.aggressive_params.maxVolBefore    = data["equity_params"]["aggressive"]["max_vol_before"];
    params.aggressive_params.maxVolAfter     = data["equity_params"]["aggressive"]["max_vol_after"];
    params.aggressive_params.SETmedianReturn = data["equity_params"]["aggressive"]["SETmedianReturn"];
    params.aggressive_params.SETvolatility   = data["equity_params"]["aggressive"]["SETvolatility"];

    params.money_market.maturity = data["bond_index_params"]["money_market"]["maturity"];
    params.money_market.monthlyFactor = data["bond_index_params"]["money_market"]["monthly_factor"];
    params.money_market.monthlySpread = data["bond_index_params"]["money_market"]["monthly_spread"];
    params.money_market.duration = data["bond_index_params"]["money_market"]["duration"];
    params.money_market.volatility = data["bond_index_params"]["money_market"]["volatility"];

    params.us_intermed_govt.maturity = data["bond_index_params"]["us_intermed_govt"]["maturity"];
    params.us_intermed_govt.monthlyFactor = data["bond_index_params"]["us_intermed_govt"]["monthly_factor"];
    params.us_intermed_govt.monthlySpread = data["bond_index_params"]["us_intermed_govt"]["monthly_spread"];
    params.us_intermed_govt.duration = data["bond_index_params"]["us_intermed_govt"]["duration"];
    params.us_intermed_govt.volatility = data["bond_index_params"]["us_intermed_govt"]["volatility"];

    params.us_long_corporate.maturity = data["bond_index_params"]["us_long_corporate"]["maturity"];
    params.us_long_corporate.monthlyFactor = data["bond_index_params"]["us_long_corporate"]["monthly_factor"];
    params.us_long_corporate.monthlySpread = data["bond_index_params"]["us_long_corporate"]["monthly_spread"];
    params.us_long_corporate.duration = data["bond_index_params"]["us_long_corporate"]["duration"];
    params.us_long_corporate.volatility = data["bond_index_params"]["us_long_corporate"]["volatility"];

    double DiversifiedVol   = data["equity_params"]["diversified_vol"];
    double InternationalVol = data["equity_params"]["international_vol"];
    double IntermediateVol  = data["equity_params"]["intermediate_vol"];
    double AggressiveVol    = data["equity_params"]["aggressive_vol"];

    Date start_date {.month = Month::JANUARY, .year = 2022};

    bool generateForStochExclTest = false;
    bool useNaicMeanRevPoint      = false;
    bool writeRNG                 = true;
    bool writeMultFiles           = !args.single_file;
    bool writeSingleFile          = args.single_file;
    bool writeEconSML             = false;
    int num_scenarios             = args.num_scenarios;

    Frequency freq = [&]() {
            switch (args.frequency)
            {
            case 'a': return Frequency::ANNUAL;
            case 'q': return Frequency::QUARTERLY;
            case 'm': return Frequency::MONTHLY;
            default: throw;
            }
        }();

    int num_years = args.num_period / periods_per_year(freq);

    string outputFolderName (args.out_path);

    scn_gen.generateAllScenarios(freq, num_years, num_scenarios, start_date, generateForStochExclTest, useNaicMeanRevPoint, params, correlationMatrix, args.num_threads, args.out_path);
    return 0;
}