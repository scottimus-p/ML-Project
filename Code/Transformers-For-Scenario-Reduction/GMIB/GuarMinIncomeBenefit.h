#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

using std::vector;

struct GMIB_Params
{
    int rider_term;
    double compound_growth_rate;
};


inline _NODISCARD double calculate_gmib_maturity_value(GMIB_Params gmib_params, const vector<double>& historical_account_values)
{
    int rider_term = gmib_params.rider_term;
    double compound_growth_rate = gmib_params.compound_growth_rate;

    double max_historical_value = *std::max_element(historical_account_values.cbegin(), historical_account_values.cend());
    double compounded_initial_value = historical_account_values[0] * pow((1 + compound_growth_rate), rider_term);

    return std::max(max_historical_value, compounded_initial_value);
}


inline _NODISCARD double calculate_monthly_gmib_benefit(double gmib_value)
{
    throw;
}