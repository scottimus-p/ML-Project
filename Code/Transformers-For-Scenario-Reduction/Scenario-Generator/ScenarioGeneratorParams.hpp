#pragma once

#include <cmath>

struct InterestRateGeneratorParams
{
    // Parameters of the interest rate generator
    double tau1  {};
    double beta1 {};

    double tau2   {};
    double beta2  {};
    double sigma2 {};

    double tau3      {};
    double beta3     {};
    double sigma3    {};
    double initialVol{};

    double phi   {};
    double psi   {};
    double theta {};
    double kappa {};

    double initial_short_rate {};
    double initial_long_rate  {};
    double initial_volatility {};

    double min_long_rate {};
    double max_long_rate {};
    double min_short_rate {};
};


struct EquityFundParams
{
    double targetVol      {};
    double minVol         {};
    double maxVolBefore   {};
    double maxVolAfter    {};
    double meanRevStrength{};
    double volStdDev      {};
    double a              {};
    double b              {};
    double C              {};
    double current_vol    {};
    double SETmedianReturn{};
    double SETvolatility  {};
};


struct FixedFundParams
{
    double maturity      {};
    double monthlyFactor {};
    double monthlySpread {};
    double duration      {};
    double volatility    {};
};


struct ScenarioGeneratorParams
{
    InterestRateGeneratorParams int_params;
    EquityFundParams diversified_params;
    EquityFundParams international_params;
    EquityFundParams intermediate_params;
    EquityFundParams aggressive_params;

    FixedFundParams money_market;
    FixedFundParams us_intermed_govt;
    FixedFundParams us_long_corporate;

    // Correlations of the three random processes
    double correl12 {};
    double correl13 {};
    double correl23 {};

    // Some derived values that are stored to speed computation rather than being recalculated each time needed
    double const1 {};
    double const2 {};
    double const3 {};
    double const4 {};
    double const5 {};

    double DiversifiedVol   {};
    double InternationalVol {};
    double IntermediateVol  {};
    double AggressiveVol    {};

    void update_consts()
    {
        const1 = sqrt(1 - correl12 * correl12);
        const2 = (correl23 - correl12 * correl13) / sqrt(1 - correl12 * correl12);
        const3 = sqrt(1 - pow(correl23 - correl12 * correl13, 2) / (1 - pow(correl12, 2)) - pow(correl13, 2));
        const4 = int_params.beta3 * log(int_params.tau3);
        const5 = int_params.beta1 * log(int_params.tau1);
    }
};