#pragma once

#include <random>

#include "Vector.h"

class C3RNG
{
    actlib::vector<double> mStateArray;
    long mIX1 = 0;
    long mIX2 = 0;
    long mIX3 = 0;
    bool mbInitialized = false;
    long mCurrentSeed = 0;

    static constexpr long cm1    = 259200;
    static constexpr long cia1   = 7141;
    static constexpr long cic1   = 54773;
    static constexpr double crm1 = 0.0000038580247;
    static constexpr long cm2    = 134456;
    static constexpr long cia2   = 8121;
    static constexpr long cic2   = 28411;
    static constexpr double crm2 = 0.0000074373773;
    static constexpr long cm3    = 243000;
    static constexpr long cia3   = 4561;
    static constexpr long cic3   = 51349;

public:
    C3RNG();

    void Reseed(long nSeed);

    double GetNext();
};

class MersenneTwister
{
    std::mt19937_64 gen64;
    std::uniform_real_distribution<double> distribution;

public:

    MersenneTwister() :
    distribution(0.0, 1.0)
    {
    }

    void Reseed(long nSeed)
    {
        gen64.seed(nSeed);
    }

    double GetNext()
    {
        return distribution(gen64);
    }
};