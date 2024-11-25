#pragma once

#include "Date.h"
#include "Table.h"
#include "Vector.h"

#include <map>
#include <string>

using std::map;
using std::string;

void load_historical_data (map<Date, map<double, double>> &out, Date start_date, double maturity, string file_path);


class HistCurves
{
    Date firstDate;
    int numCurves = 0;

    static constexpr int numCurvePoints = 10;

    actlib::vector<double> maturities = actlib::vector<double>(Range{ .lo = 1, .hi = numCurvePoints });
    actlib::table<double> curves;

    double histCurveRateByDate(Date date, int maturityIndex) const;

public:

    void Initialize(map<Date, map<double, double>> historicalData);
    int BestFittingCurve(double d_shortRate, double d_midRate, double d_longRate, int l_fitMethod);
    double histCurveRateByIndex(int curveIndex, int maturityIndex) const;

    Date getFirstDate() const
    {
        return firstDate;
    }

    Date getLastDate() const
    {
        return futureDate(firstDate, numCurves - 1);
    }

    map<double, double> getCurveByDate(Date date) const;

    actlib::vector<double> getCurveVecByDate(Date date) const;
};

