#include "Cholesky.h"

#include <cmath>


double Cholesky::GetRandNum(long row, long col) const
{
    return mRandom(row, col);
}


void Cholesky::SetRandNum(long row, long col, double num)
{
    mRandom(row, col) = num;
}


double Cholesky::corrNum(long row, long col) const
{
    return mCorrelated(row, col);
}

void Cholesky::setup(long nVars, long nObs, actlib::table<double> correlations)
{
    // The argument correlations() must be  a correlation matrix with
    // nVars rows and columns.

    numVars = nVars;
    numObs = nObs;
    mRandom     = actlib::table<double>(numVars, numObs);
    mCorrelated = actlib::table<double>(numVars, numObs);
    mCorr       = actlib::table<double>(numVars, numVars);
    mA          = actlib::table<double>(numVars, numVars);

    mCorr = correlations;

    calcConversionMatrix();
}

void Cholesky::calcConversionMatrix()
{
    // Performs Cholesky factorization of mCorr() to get mA()

    mA(0, 0) = sqrt(mCorr(0, 0));

    for (int i = 1; i < numVars; i++)
    {
        for (int j = 1; j < i; j++)
        {
            mA(i, j) = mCorr(i, j);

            for (int k = 0; k < j; k++)
                mA(i, j) = mA(i, j) - mA(i, k) * mA(j, k);

            mA(i, j) = mA(i, j) / mA(j, j);
        }

        mA(i, i) = mCorr(i, i);

        for (int k = 0; k < i; k++)
            mA(i, i) = mA(i, i) - mA(i, k) * mA(i, k);

        mA(i, i) = sqrt(mA(i, i));
    }
}

void Cholesky::correlate()
{
    // Multiplies the factored correlation matrix by the uncorrelated
    // random number array

    for (int obs = 0; obs < numObs; obs++)
    {
        for (int var = 0; var < numVars; var++)
        {
            mCorrelated(var, obs) = 0.;
            for (int i = 0; i < numVars; i++)
                mCorrelated(var, obs) = mCorrelated(var, obs) + mA(var, i) * mRandom(i, obs);

        }
    }
}