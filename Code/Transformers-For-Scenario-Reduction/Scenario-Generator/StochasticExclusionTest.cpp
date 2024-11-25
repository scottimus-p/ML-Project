#include "StochasticExclusionTest.h"

#include <cmath>


bool isOdd(int aNumber)
{
    // This function is included in the Analysis Toolpack under the name ISODD
    // Added here so the Analysis Toolpack is not required

    return !(aNumber = ((aNumber / 2) * 2));
}


// This function provides the "random" number shocks that are used in the
// AAA scenario generator to define the scenarios used in the
// Stochastic Exclusion Test (SET).

double testShock(int scenarioNum, int durMonths, shockType whichShock)
{
    // Arguments:
    // scenarioNum must be from 1 to 16
    // durMonths is the number of months since the scenario started.  The first scenario month is durMonths = 1.
    // whichShock must be a member of the shockType enum defined at the top of this module

    double result = 0;

    if (scenarioNum > 16 || scenarioNum < 1)
    {
        throw;
    }

    if (durMonths < 1)
    {
        throw;
    }

    if (whichShock == LongIntShock)
    {

        switch (scenarioNum)
        {
        case 1:
        case 2: // pop-up
            result = 1.282 * (sqrt(durMonths) - sqrt(durMonths - 1));
            break;

        case 3:
        case 4:
            result = -1.282 * (sqrt(durMonths) - sqrt(durMonths - 1));
            break;

        case 5:
        case 6:  // Up-down
            result = 1.282 / sqrt(60);
            if (isOdd((durMonths - 1) / 60))
                result = -result;

            break;

        case 7:
        case 8:   // Down-up
            result = -1.282 / sqrt(60);
            if (isOdd((durMonths - 1) / 60))
                result = -result;

            break;

        case 9:
        case 10:
        case 11:  // Base, volatile slope, volatile equity
            result = 0;
            break;

        case 12:  // Deterministic
            if (durMonths < 241)
                result = -1 / sqrt(240);
            else
                result = 0;

            break;

        case 13:
        case 14:  // Delayed, more severe pop-up
            if (durMonths < 121)
                result = 0;
            else if (durMonths < 241)
                result = 1.414 * testShock(1, durMonths - 120, LongIntShock);
            else
                result = testShock(1, durMonths, LongIntShock);

            break;

        case 15:
        case 16:  // Delayed, more severe pop-down
            if (durMonths < 121)
                result = 0;
            else if (durMonths < 241)
                result = 1.414 * testShock(3, durMonths - 120, LongIntShock);
            else
                result = testShock(3, durMonths, LongIntShock);

            break;

        }
    }
    else if (whichShock == IntDiffShock)
    {
        // In all scenarios except no. 10, this shock is equal and opposite to
        // the shock to the long rate.
        if (scenarioNum == 10)
            result = -2 / sqrt(24.);
        
        if (isOdd((durMonths - 1) / 36))
            result = -result;
        else
            result = -testShock(scenarioNum, durMonths, LongIntShock);
    }
    else if (whichShock == EquityShock)
    {
        switch (scenarioNum)
        {
        case 1:
        case 3:
        case 5:
        case 7:
            result = 1.282 * (sqrt(durMonths) - sqrt(durMonths - 1));
            break;
                
        case 2:
        case 4:
        case 6:
        case 8:
            result = -1.282 * (sqrt(durMonths) - sqrt(durMonths - 1));
            break;

        case 9:
        case 10:
            result = 0;
            break;

        case 11:  // Volatile equity returns
            result = -1 / sqrt(12);
                
            if (isOdd((durMonths - 1) / 24))
                result = -result;

            break;
                
        case 12:  // Deterministic
            if (durMonths < 241)
                result = -1 / sqrt(240);
            else 
                result = 0;

            break;

        case 13:
        case 15:
            result = testShock(13, durMonths, LongIntShock);
            break;

        case 14:
        case 16:
            result = -testShock(13, durMonths, LongIntShock);
            break;
        }
    }

    return result;
}
