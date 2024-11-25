#pragma once

enum shockType
{
    // These values are used as arguments to the testShock() function
    LongIntShock = 1,
    IntDiffShock = 2,
    EquityShock = 3,
};

double testShock(int scenarioNum, int durMonths, shockType whichShock);