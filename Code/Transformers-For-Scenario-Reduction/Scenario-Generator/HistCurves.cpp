#include "HistCurves.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

using std::ifstream;
using std::vector;

vector<string> split(string s, const string& delim)
{
    vector<string> v;

    int idx_start = 0;
    while (idx_start < s.length())
    {
        int idx_end = s.find(delim, idx_start);

        idx_end = idx_end == string::npos ? s.length() : idx_end;

        v.emplace_back(s.substr(idx_start, idx_end - idx_start));
        
        idx_start = idx_end + 1;
    }
    return v;
}

void load_historical_data(map<Date, map<double, double>>& out, Date start_date, double maturity, string file_path)
{
    ifstream infile(file_path);

    string csv_str;
    std::getline(infile, csv_str);

    auto historical_rates = split(csv_str, ",");

    Date curr_date = start_date;

    for (const auto& val : historical_rates)
    {
        out[curr_date][maturity] = std::stod(val);
        curr_date = next_month(curr_date);
    }
}

void HistCurves::Initialize(map<Date, map<double, double>> historicalData)
{
    maturities(1) = 0.25;
    maturities(2) = 0.5;
    maturities(3) = 1;
    maturities(4) = 2;
    maturities(5) = 3;
    maturities(6) = 5;
    maturities(7) = 7;
    maturities(8) = 10;
    maturities(9) = 20;
    maturities(10) = 30;

    auto earliestCurve = historicalData.begin();
    numCurves = static_cast<int>(historicalData.size());

    curves = actlib::table<double>(X_Range{ .lo = 1, .hi = numCurves }, Y_Range{ .lo = 1, .hi = numCurvePoints });

    firstDate = historicalData.begin()->first;

    int curveNum = 1;
    for (const auto& [date, curve] : historicalData)
    {
        int maturityNum = 1;
        for (const auto& [maturity, rate] : curve)
        {
            curves(curveNum, maturityNum) = rate;
            maturityNum++;
        }
        curveNum++;
    }
}


int HistCurves::BestFittingCurve(double d_shortRate, double d_midRate, double d_longRate, int l_fitMethod)
{
    // Fit method 1 is sum of absolute differences, method 2 is sum of squares.

    int l_Best_YC = -1;
    double d_Diff_YC;

    // Weights given to the short, mid, and long rate in the fitting criteria
    constexpr double WEIGHT_SHORT = 40;
    constexpr double WEIGHT_MID = 20;
    constexpr double WEIGHT_LONG = 40;

    double d_MinDiff = DBL_MAX;

    // Iterate through all the historical yield curves
    for (int l_YC_Counter = 1; l_YC_Counter <= numCurves; l_YC_Counter++)
    {
        double d_histShortRate = curves(l_YC_Counter, 3);
        double d_histMidRate = curves(l_YC_Counter, 7);
        double d_histLongRate = curves(l_YC_Counter, 9);

        double d_Diff_S = d_shortRate - d_histShortRate;
        double d_Diff_M = d_midRate - d_histMidRate;
        double d_Diff_L = d_longRate - d_histLongRate;

        if (l_fitMethod == 1) // method 1 is sum of absolute value of differences
        {
            d_Diff_S = abs(d_Diff_S);
            d_Diff_M = abs(d_Diff_M);
            d_Diff_L = abs(d_Diff_L);
        }
        else    // method 2 is sum of squared differences
        {
            d_Diff_S = d_Diff_S * d_Diff_S;
            d_Diff_M = d_Diff_M * d_Diff_M;
            d_Diff_L = d_Diff_L * d_Diff_L;
        }

        d_Diff_YC = WEIGHT_SHORT * d_Diff_S + WEIGHT_MID * d_Diff_M + WEIGHT_LONG * d_Diff_L;

        // Replace the minimum if a smaller difference is found
        if (d_Diff_YC < d_MinDiff)
        {
            d_MinDiff = d_Diff_YC;
            l_Best_YC = l_YC_Counter;
        }

    }

    return l_Best_YC;

}


double HistCurves::histCurveRateByIndex(int curveIndex, int maturityIndex) const
{
    return curves(curveIndex, maturityIndex);
}


double HistCurves::histCurveRateByDate(Date date, int maturityIndex) const
{
    int index = 1 + monthsBetween(firstDate, date);
        
    return curves(index, maturityIndex);
}

map<double, double> HistCurves::getCurveByDate(Date date) const
{
    map<double, double> result;

    for (auto m = maturities.cbegin(); m != maturities.cend(); m++)
    {
        int idx = std::distance(maturities.cbegin(), m);

        result[*m] = histCurveRateByDate(date, idx);
    }

    return result;
}

actlib::vector<double> HistCurves::getCurveVecByDate(Date date) const
{
    actlib::vector<double> result(Range(maturities.lower_bound(), maturities.upper_bound()));

    auto result_itr = result.begin();

    for (auto m = maturities.cbegin(); m != maturities.cend(); m++)
    {
        int idx = std::distance(m, maturities.cbegin()) + 1; // the underlying data structure is 1-indexed, not 0-indexed

        *(result_itr++) = histCurveRateByDate(date, idx);
    }

    return result;
}