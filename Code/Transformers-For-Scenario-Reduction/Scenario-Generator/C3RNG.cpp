#include "C3RNG.h"

C3RNG::C3RNG() :
    mIX1(0),
    mIX2(0),
    mIX3(0),
    mbInitialized(false),
    mStateArray(Range{ .lo = 1, .hi = 97 }),
    mCurrentSeed(0)
{

}


void C3RNG::Reseed(long nSeed)
{
    mIX1 = cic1 - nSeed;
    mIX1 = (cia1 * mIX1 + cic1) % cm1;
    mIX2 = mIX1 % cm2;
    mIX1 = (cia1 * mIX1 + cic1) % cm1;
    mIX3 = mIX1 % cm3;

    for (auto& i : mStateArray)
    {
        mIX1 = (cia1 * mIX1 + cic1) % cm1;
        mIX2 = (cia2 * mIX2 + cic2) % cm2;
        i = (mIX1 + mIX2 * crm2) * crm1;
    }

    mCurrentSeed = nSeed;
    mbInitialized = true;
}


double C3RNG::GetNext()
{
    long j;
    double u;

    // Raise error if generator is not seeded
    if (!mbInitialized) throw;

    mIX1 = (cia1 * mIX1 + cic1) % cm1;
    mIX2 = (cia2 * mIX2 + cic2) % cm2;
    mIX3 = (cia3 * mIX3 + cic3) % cm3;

    j = int(mStateArray.lower_bound() + (mStateArray.upper_bound() * mIX3) / cm3);

    double result = mStateArray(j);

    u = (mIX1 + mIX2 * crm2) * crm1;
    if (u <= 0 || u >= 1)
        u = 1. + int(u) - u;

    mStateArray(j) = u;

    return result;
}