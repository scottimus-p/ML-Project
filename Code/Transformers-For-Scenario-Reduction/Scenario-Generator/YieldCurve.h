#pragma once

#include <string>

#include "Vector.h"

#include "HistCurves.h"

using std::string;

class YieldCurve
{
    actlib::vector<double> interpolatedRates;
    actlib::vector<double> generatedRates;
    double logVolatility;

    actlib::vector<double> spotRates;
    bool spotRatesAvailable;

    actlib::vector<double> maturities;

    void interpolate(HistCurves& HistData);

public:

    YieldCurve();

    double generatedRate(int i) const;
    double spotRateAtIndex(int index);
    double rateAtIndex(int index) const;
    double rateAtMaturity(double maturityYrs) const;
    double getLogVolatility() const;

    void Initialize(double shortRate, double longRate, double logVol);

    void interpolateNS();
    void perturb(actlib::vector<double> adj, double portion);
    void calcSpotRates();

    _NODISCARD string serializeToJson() const;
};

