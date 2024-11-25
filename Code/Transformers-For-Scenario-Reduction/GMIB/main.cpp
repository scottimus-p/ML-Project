
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include "argparse.hpp"
#include "json.hpp"

#include "FundAccount.h"
#include "GuarMinIncomeBenefit.h"
#include "Scenario.h"

using std::ifstream;
using std::ofstream;
using json = nlohmann::json;
using std::map;
using std::string;
using std::vector;


string json_param_to_string(auto param)
{
    if      (param.is_number_integer()) return std::to_string(int(param));
    else if (param.is_boolean())        return std::to_string(bool(param));
    else if (param.is_number_float())   return std::to_string(double(param));
    else if (param.is_string())         return param;

    throw;
}

struct InputArgs : public argparse::Args
{
    string& in_dir      = kwarg("in", "directory for the input").set_default("");
    string& out_dir     = kwarg("out", "directory for the output").set_default("");
    int& num_period     = kwarg("num_periods", "number of periods in each scenario").set_default(0);
    int& num_scenarios  = kwarg("num_scenarios", "number of scenarios to run").set_default(0);
    int& maturity_age   = kwarg("maturity_age", "age when the rider matures").set_default(10);
    double& growth_rate = kwarg("growth_rate", "compound growth rate").set_default(0.0);
    double& dep_amount  = kwarg("deposit", "deposit amount").set_default(100'000);
    string& param_file  = kwarg("p,params", "parameter file").set_default("");

    static InputArgs get_args(int argc, char** argv)
    {
        auto args = argparse::parse<InputArgs>(argc, argv);

        if (args.param_file.empty())
        {
            return argparse::parse<InputArgs>(argc, argv);
        }
        else
        {
            ifstream param_file(args.param_file);
            json data = json::parse(param_file);

            vector<std::basic_string<char>> shadow_params;
            vector<const char*> params;
            params.push_back(argv[0]);

            auto add_param = [&](string param_name)
            {
                if (data.contains(param_name))
                {
                    std::basic_string<char> p;

                    p += "--" + param_name + "=" + json_param_to_string(data[param_name]);

                    shadow_params.push_back(p);
                }
            };

            add_param("in");
            add_param("out");
            add_param("num_periods");
            add_param("num_scenarios");
            add_param("maturity_age");
            add_param("growth_rate");
            add_param("deposit");

            for (auto& p : shadow_params)
                params.push_back(p.c_str());

            const char* const* params_arr = params.data();
            return argparse::parse<InputArgs>(params.size(), params_arr, false);
        }
    }
};


struct PolicyInfo
{
    map<int, double> age_distribution { { 20, 0.10 },
                                        { 25, 0.10 },
                                        { 30, 0.10 },
                                        { 35, 0.10 },
                                        { 40, 0.10 },
                                        { 45, 0.10 },
                                        { 50, 0.10 },
                                        { 55, 0.10 },
                                        { 60, 0.10 },
                                        { 65, 0.10 } };

    map<char, double> gender_distribution { { 'm', 0.50 },
                                            { 'f', 0.50 }};

    struct Policy
    {
        int age;
        char gender;
        double weight;
    };

    vector<Policy> policies;

    PolicyInfo()
    {
        policies.reserve(age_distribution.size() * gender_distribution.size());

        for (auto age : age_distribution)
            for (auto gender : gender_distribution)
                policies.push_back( {age.first, gender.first, age.second * gender.second });
    }
};


vector<double> run_single_policy_single_scenario(vector<double>& cashflows, const InputArgs& args, const PolicyInfo::Policy& policy, short num_months, const Scenario& s)
{
    FundAccount f;

    int rider_term = args.maturity_age - policy.age;

    GMIB_Params gmib_params{ .rider_term = rider_term, .compound_growth_rate = args.growth_rate };

    double deposit_amount = args.dep_amount;

    vector<double> historical_values;

    f.add_deposit(deposit_amount);
    historical_values.push_back(f.get_total_fund_value());
    cashflows.at(0) = deposit_amount;

    for (auto i = 1; i < gmib_params.rider_term * 12; i++)
    {
        f.rollforward_funds_this_month(i, s);

        historical_values.push_back(f.get_total_fund_value());
    }

    double gmib_value = calculate_gmib_maturity_value(gmib_params, historical_values);
    cashflows.at(gmib_params.rider_term * 12) += gmib_value;

    return cashflows;
}



void write_pv_of_cashflows(ofstream& outfile, const vector<double>& cashflows, double discount_rate, int scenario_num, bool use_comma_separator)
{
    vector<double> discount_factors (cashflows.size());

    double monthly_discount_factor = pow(1 + discount_rate, -1./12.);

    double v = 1;

    std::generate(discount_factors.begin(), discount_factors.end(), [&]() {
        v *= monthly_discount_factor;
        return v;
    });

    auto reduce = std::plus<double> {};
    auto transform = std::multiplies<double> {};

    double pv_cf = std::transform_reduce(cashflows.begin(), cashflows.end(), discount_factors.begin(), 0., reduce, transform);

    outfile << "{\"scenario_number\": " << scenario_num << ", \"pv_cf\": " << std::to_string(pv_cf) << " }" << (use_comma_separator ? ",\n" : "\n");
}


int main(int argc, char** argv)
{
    auto args = InputArgs::get_args(argc, argv);

    args.print();

    try
    {
        ofstream outfile(args.out_dir + "result.json", std::ios::out | std::ios::trunc);
        outfile << "[\n";

        // Save the start time for use when later determining total elapsed time.
        auto StartTime = std::chrono::system_clock::now();

        for (auto i = 1; i <= args.num_scenarios; i++)
        {
            std::cout << "processing scenario " << i << std::endl;

            string scenario_filename = args.in_dir + "scenario_" + std::to_string(i) + ".json";

            ifstream scenario_file(scenario_filename);
            json data = json::parse(scenario_file);

            const int num_months = args.num_period;

            Scenario s(data, num_months);

            vector<double> cashflows(num_months + 1, 0);

            for (auto policy : PolicyInfo().policies)
            {
                run_single_policy_single_scenario(cashflows, args, policy, num_months, s);
            }

            double discount_rate = 0.05;
            write_pv_of_cashflows(outfile, cashflows, discount_rate, i, i != args.num_scenarios);
        }     
        
        auto dEndTime = std::chrono::system_clock::now();

        std::cout << "Processing time = " << std::chrono::duration_cast<std::chrono::seconds>(dEndTime - StartTime).count() << " seconds" << std::endl;

        outfile << "]";
        outfile.close();
    }
    catch (...)
    {
        std::cout << "uncaught exception" << std::endl;
        return -1;
    }

    return 0;
}