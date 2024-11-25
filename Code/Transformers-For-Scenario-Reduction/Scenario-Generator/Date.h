#pragma once

enum Month : short
{
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
};


struct Date
{
    Month month;
    short year;
};

inline Month next_month(Month m)
{
    switch (m)
    {
    case JANUARY:   return FEBRUARY;
    case FEBRUARY:  return MARCH;
    case MARCH:     return APRIL;
    case APRIL:     return MAY;
    case MAY:       return JUNE;
    case JUNE:      return JULY;
    case JULY:      return AUGUST;
    case AUGUST:    return SEPTEMBER;
    case SEPTEMBER: return OCTOBER;
    case OCTOBER:   return NOVEMBER;
    case NOVEMBER:  return DECEMBER;
    case DECEMBER:  return JANUARY;
    }

    throw;
}

inline Date next_month(Date d)
{
    if (d.month == DECEMBER)
    {
        d.month = JANUARY;
        d.year++;
    }
    else
    {
        d.month = next_month(d.month);
    }

    return d;
}

inline Date next_year(Date d)
{
    return Date{.month = d.month, .year = short(d.year + 1)};
}


inline bool operator<(Date lhs, Date rhs)
{
    return lhs.year < rhs.year || (lhs.year == rhs.year && lhs.month < rhs.month);
}

inline bool operator>(Date lhs, Date rhs)
{
    return !(lhs < rhs);
}

inline bool operator==(Date lhs, Date rhs)
{
    return lhs.year == rhs.year && lhs.month == rhs.month;
}

inline bool operator!=(Date lhs, Date rhs)
{
    return !(lhs == rhs);
}

inline bool operator<=(Date lhs, Date rhs)
{
    return !(lhs > rhs);
}

inline bool operator>=(Date lhs, Date rhs)
{
    return !(lhs < rhs);
}


inline Date futureDate(Date currentDate, int months)
{
    // Returns a date some months in the future
    
    short futureMonth = (currentDate.month + months) % 12;

    if (futureMonth == 0)
        futureMonth = 12;

    short futureYear = currentDate.year + (months / 12);

    if (futureMonth < currentDate.month)
        futureYear += 1;

    return Date{ .month = Month(futureMonth), .year = futureYear };
}


inline int monthsBetween(Date aDate1, Date aDate2)
{
    // Returns a positive result only if aDate2 is after aDate1
    return 12 * (aDate2.year - aDate1.year) + (aDate2.month - aDate1.month);
}