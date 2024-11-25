#pragma once

#include "Table.h"

/**
 * This module uses the Choleski factorization method to
 * determine a matrix mA such that (mA)*transpose(mA) = mCorr
 * where mCorr is a correlation matrix.
 *
 * The matrix mA can then be multiplied by an array of
 * uncorrelated random numbers U to get an array of
 * correlated random numbers C as in mA * U = C.
 * The columns of U and C represent trials or observations (or time periods)
 * and each row represents a sequence of trials for one variable
 * of a set of variables, where the variables are either
 * correlated or uncorrelated.  The rows or C will be
 * correlated to the degree specified by mCorr.
 *
 * To use this object, first call setup() to initialize the
 * size of the matrices and specify correlations.  Then
 * generate uncorrelated random numbers to fill the matrix mRandom().
 * Finally, call correlate() to create the correlated matrix
 * named mCorrelated().  You can then use the values from
 * that matrix.
 */

class Cholesky
{
    long numVars;                 //number of rows and columns in the correlation matrix
    long numObs;                  // number of observations to correlate

    actlib::table<double> mCorr;       // The correlation matrix (input)
    actlib::table<double> mA;          // The Choleski factoriztion of the correlation matrix
    actlib::table<double> mRandom;     // The input uncorrelated random matrix U
    actlib::table<double> mCorrelated; // The output correlated random matrix C

    void calcConversionMatrix();

public:

    double GetRandNum(long row, long col) const;
    double corrNum(long row, long col) const;

    void SetRandNum(long row, long col, double num);
    void setup(long nVars, long nObs, actlib::table<double> correlations);
    void correlate();
};