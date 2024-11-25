#pragma once

struct Range
{
    int lo;
    int hi;
};

struct X_Range
{
    int lo;
    int hi;
};

struct Y_Range
{
    int lo;
    int hi;
};

template <typename T>
_NODISCARD int range_size(const T& range)
{
    return range.hi - range.lo + 1;
}