/*
 * Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 */

#ifndef DateMath_h
#define DateMath_h

#include <cmath>
#include <string.h>
#include <time.h>
#include <wtf/CurrentTime.h>
#include <wtf/Noncopyable.h>
#include <wtf/UnusedParam.h>

namespace WTF {

struct LocalTimeOffset {
    LocalTimeOffset()
        : isDST(false)
        , offset(0)
    {
    }

    LocalTimeOffset(bool isDST, int offset)
        : isDST(isDST)
        , offset(offset)
    {
    }

    bool operator==(const LocalTimeOffset& other)
    {
        return isDST == other.isDST && offset == other.offset;
    }

    bool operator!=(const LocalTimeOffset& other)
    {
        return isDST != other.isDST || offset != other.offset;
    }

    bool isDST;
    int offset;
};

void initializeDates();
int equivalentYearForDST(int year);

// Not really math related, but this is currently the only shared place to put these.
double parseDateFromNullTerminatedCharacters(const char* dateString);
double timeClip(double);

inline double jsCurrentTime()
{
    // JavaScript doesn't recognize fractions of a millisecond.
    return floor(WTF::currentTimeMS());
}

const char * const weekdayName[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const char * const monthName[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

const double hoursPerDay = 24.0;
const double minutesPerHour = 60.0;
const double secondsPerHour = 60.0 * 60.0;
const double secondsPerMinute = 60.0;
const double msPerSecond = 1000.0;
const double msPerMinute = 60.0 * 1000.0;
const double msPerHour = 60.0 * 60.0 * 1000.0;
const double msPerDay = 24.0 * 60.0 * 60.0 * 1000.0;
const double msPerMonth = 2592000000.0;

// Returns the number of days from 1970-01-01 to the specified date.
double dateToDaysFrom1970(int year, int month, int day);
int msToYear(double ms);
int dayInYear(double ms, int year);
int monthFromDayInYear(int dayInYear, bool leapYear);
int dayInMonthFromDayInYear(int dayInYear, bool leapYear);

// Returns combined offset in millisecond (UTC + DST).
LocalTimeOffset calculateLocalTimeOffset(double utcInMilliseconds);

} // namespace WTF

using WTF::dateToDaysFrom1970;
using WTF::dayInMonthFromDayInYear;
using WTF::dayInYear;
using WTF::minutesPerHour;
using WTF::monthFromDayInYear;
using WTF::msPerDay;
using WTF::msPerSecond;
using WTF::msToYear;
using WTF::secondsPerMinute;
using WTF::LocalTimeOffset;
using WTF::calculateLocalTimeOffset;

#if USE(JSC)
namespace JSC {
class ExecState;
struct GregorianDateTime;

void msToGregorianDateTime(ExecState*, double, bool outputIsUTC, GregorianDateTime&);
double gregorianDateTimeToMS(ExecState*, const GregorianDateTime&, double, bool inputIsUTC);
double getUTCOffset(ExecState*);
double parseDateFromNullTerminatedCharacters(ExecState*, const char* dateString);

// Intentionally overridding the default tm of the system.
// The members of tm differ on various operating systems.
struct GregorianDateTime : Noncopyable {
    GregorianDateTime()
        : second(0)
        , minute(0)
        , hour(0)
        , weekDay(0)
        , monthDay(0)
        , yearDay(0)
        , month(0)
        , year(0)
        , isDST(0)
        , utcOffset(0)
        , timeZone(nullptr)
    {
    }

    ~GregorianDateTime()
    {
        delete [] timeZone;
    }

    operator tm() const
    {
        tm ret;
        memset(&ret, 0, sizeof(ret));

        ret.tm_sec   =  second;
        ret.tm_min   =  minute;
        ret.tm_hour  =  hour;
        ret.tm_wday  =  weekDay;
        ret.tm_mday  =  monthDay;
        ret.tm_yday  =  yearDay;
        ret.tm_mon   =  month;
        ret.tm_year  =  year;
        ret.tm_isdst =  isDST;

#if HAVE(TM_GMTOFF)
        ret.tm_gmtoff = static_cast<long>(utcOffset);
#endif
#if HAVE(TM_ZONE)
        ret.tm_zone = timeZone;
#endif

        return ret;
    }

    void copyFrom(const GregorianDateTime& rhs)
    {
        second = rhs.second;
        minute = rhs.minute;
        hour = rhs.hour;
        weekDay = rhs.weekDay;
        monthDay = rhs.monthDay;
        yearDay = rhs.yearDay;
        month = rhs.month;
        year = rhs.year;
        isDST = rhs.isDST;
        utcOffset = rhs.utcOffset;
        if (rhs.timeZone) {
            int inZoneSize = strlen(rhs.timeZone) + 1;
            timeZone = new char[inZoneSize];
            strncpy(timeZone, rhs.timeZone, inZoneSize);
        } else
            timeZone = nullptr;
    }

    int second;
    int minute;
    int hour;
    int weekDay;
    int monthDay;
    int yearDay;
    int month;
    int year;
    int isDST;
    int utcOffset;
    char* timeZone;
};

static inline int gmtoffset(const GregorianDateTime& t)
{
    return t.utcOffset;
}
} // namespace JSC
#endif // USE(JSC)

#endif // DateMath_h
