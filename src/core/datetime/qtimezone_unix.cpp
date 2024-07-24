/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qtimezone.h>
#include <qtimezone_p.h>

#include <qdatastream.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qfile.h>
#include <qhash.h>
#include <qstringlist.h>

#include <qlocale_tools_p.h>

#include <algorithm>

struct QTzTimeZone {
   QLocale::Country country;
   QByteArray comment;
};

using QTzTimeZoneHash = QHash<QByteArray, QTzTimeZone>;

// Parse zone.tab table, assume it contains all installed zones, if not will need to read directories
static QTzTimeZoneHash loadTzTimeZones()
{
   QString path = "/usr/share/zoneinfo/zone.tab";

   if (! QFile::exists(path)) {
      path = "/usr/lib/zoneinfo/zone.tab";
   }

   QFile tzif(path);

   if (! tzif.open(QIODevice::ReadOnly)) {
      return QTzTimeZoneHash();
   }

   QTzTimeZoneHash zonesHash;

   // TODO QTextStream inefficient, replace later
   QTextStream ts(&tzif);

   while (! ts.atEnd()) {
      const QString line = ts.readLine();

      // Comment lines are prefixed with a #
      if (! line.isEmpty() && line.at(0) != '#') {
         // Data rows are tab-separated columns Region, Coordinates, ID, Optional Comments
         const QStringList parts = line.split('\t');
         QTzTimeZone zone;

         zone.country = QLocalePrivate::codeToCountry(parts.at(0));

         if (parts.size() > 3) {
            zone.comment = parts.at(3).toUtf8();
         }

         zonesHash.insert(parts.at(2).toUtf8(), zone);
      }
   }

   return zonesHash;
}

// Hash of available system tz files as loaded by loadTzTimeZones()

static QTzTimeZoneHash *tzZones()
{
   static QTzTimeZoneHash retval(loadTzTimeZones());
   return &retval;
}

/*
    The following is copied and modified from tzfile.h which is in the public domain.
    Copied as no compatibility guarantee and is never system installed.
    See https://github.com/eggert/tz/blob/master/tzfile.h
*/

#define TZ_MAGIC      "TZif"
#define TZ_MAX_TIMES  1200
#define TZ_MAX_TYPES   256  // Limited by what (unsigned char)'s can hold
#define TZ_MAX_CHARS    50  // Maximum number of abbreviation characters
#define TZ_MAX_LEAPS    50  // Maximum number of leap second corrections

struct QTzHeader {
   char       tzh_magic[4];        // TZ_MAGIC
   char       tzh_version;         // '\0' or '2' as of 2005
   char       tzh_reserved[15];    // reserved--must be zero
   quint32    tzh_ttisgmtcnt;      // number of trans. time flags
   quint32    tzh_ttisstdcnt;      // number of trans. time flags
   quint32    tzh_leapcnt;         // number of leap seconds
   quint32    tzh_timecnt;         // number of transition times
   quint32    tzh_typecnt;         // number of local time types
   quint32    tzh_charcnt;         // number of abbr. chars
};

struct QTzTransition {
   qint64 tz_time;        // Transition time
   quint8 tz_typeind;     // Type Index
};

struct QTzType {
   int tz_gmtoff;        // UTC offset in seconds
   bool   tz_isdst;      // Is DST
   quint8 tz_abbrind;    // abbreviation list index
   bool   tz_ttisgmt;    // Is in UTC time
   bool   tz_ttisstd;    // Is in Standard time
};

// TZ File parsing

static QTzHeader parseTzHeader(QDataStream &stream, bool *ok)
{
   QTzHeader hdr;
   quint8 ch;
   *ok = false;

   // Parse Magic, 4 bytes
   stream.readRawData(hdr.tzh_magic, 4);

   if (memcmp(hdr.tzh_magic, TZ_MAGIC, 4) != 0 || stream.status() != QDataStream::Ok) {
      return hdr;
   }

   // Parse Version, 1 byte, before 2005 was '\0', since 2005 a '2', since 2013 a '3'
   stream >> ch;
   hdr.tzh_version = ch;

   if (stream.status() != QDataStream::Ok
         || (hdr.tzh_version != '2' && hdr.tzh_version != '\0' && hdr.tzh_version != '3')) {
      return hdr;
   }

   // Parse reserved space, 15 bytes
   stream.readRawData(hdr.tzh_reserved, 15);

   if (stream.status() != QDataStream::Ok) {
      return hdr;
   }

   // Parse rest of header, 6 x 4-byte transition counts
   stream >> hdr.tzh_ttisgmtcnt >> hdr.tzh_ttisstdcnt >> hdr.tzh_leapcnt >> hdr.tzh_timecnt
         >> hdr.tzh_typecnt >> hdr.tzh_charcnt;

   // Check defined maximums
   if (stream.status() != QDataStream::Ok
         || hdr.tzh_timecnt > TZ_MAX_TIMES
         || hdr.tzh_typecnt > TZ_MAX_TYPES
         || hdr.tzh_charcnt > TZ_MAX_CHARS
         || hdr.tzh_leapcnt > TZ_MAX_LEAPS
         || hdr.tzh_ttisgmtcnt > hdr.tzh_typecnt
         || hdr.tzh_ttisstdcnt > hdr.tzh_typecnt) {

      return hdr;
   }

   *ok = true;

   return hdr;
}

static QVector<QTzTransition> parseTzTransitions(QDataStream &stream, int tzh_timecnt, bool longTran)
{
   QVector<QTzTransition> transitions(tzh_timecnt);

   if (longTran) {
      // Parse tzh_timecnt x 8-byte transition times
      for (int i = 0; i < tzh_timecnt && stream.status() == QDataStream::Ok; ++i) {
         stream >> transitions[i].tz_time;

         if (stream.status() != QDataStream::Ok) {
            transitions.resize(i);
         }
      }
   } else {
      // Parse tzh_timecnt x 4-byte transition times
      int val;

      for (int i = 0; i < tzh_timecnt && stream.status() == QDataStream::Ok; ++i) {
         stream >> val;
         transitions[i].tz_time = val;

         if (stream.status() != QDataStream::Ok) {
            transitions.resize(i);
         }
      }
   }

   // Parse tzh_timecnt x 1-byte transition type index
   for (int i = 0; i < tzh_timecnt && stream.status() == QDataStream::Ok; ++i) {
      quint8 typeind;
      stream >> typeind;

      if (stream.status() == QDataStream::Ok) {
         transitions[i].tz_typeind = typeind;
      }
   }

   return transitions;
}

static QVector<QTzType> parseTzTypes(QDataStream &stream, int tzh_typecnt)
{
   QVector<QTzType> types(tzh_typecnt);

   // Parse tzh_typecnt x transition types
   for (int i = 0; i < tzh_typecnt && stream.status() == QDataStream::Ok; ++i) {
      QTzType &type = types[i];
      // Parse UTC Offset, 4 bytes
      stream >> type.tz_gmtoff;

      // Parse Is DST flag, 1 byte
      if (stream.status() == QDataStream::Ok) {
         stream >> type.tz_isdst;
      }

      // Parse Abbreviation Array Index, 1 byte
      if (stream.status() == QDataStream::Ok) {
         stream >> type.tz_abbrind;
      }

      // Set defaults in case not populated later
      type.tz_ttisgmt = false;
      type.tz_ttisstd = false;

      if (stream.status() != QDataStream::Ok) {
         types.resize(i);
      }
   }

   return types;
}

static QMap<int, QByteArray> parseTzAbbreviations(QDataStream &stream, int tzh_charcnt, const QVector<QTzType> &types)
{
   // Parse the abbreviation list which is tzh_charcnt long with '\0' separated strings. The
   // QTzType.tz_abbrind index points to the first char of the abbreviation in the array, not the
   // occurrence in the list. It can also point to a partial string so we need to use the actual typeList
   // index values when parsing.  By using a map with tz_abbrind as ordered key we get both index
   // methods in one data structure and can convert the types afterwards.
   QMap<int, QByteArray> map;
   quint8 ch;
   QByteArray input;

   // First parse the full abbrev string
   for (int i = 0; i < tzh_charcnt && stream.status() == QDataStream::Ok; ++i) {
      stream >> ch;

      if (stream.status() == QDataStream::Ok) {
         input.append(char(ch));
      } else {
         return map;
      }
   }

   // Then extract all the substrings pointed to by types
   for (const QTzType &type : types) {
      QByteArray abbrev;

      for (int i = type.tz_abbrind; input.at(i) != '\0'; ++i) {
         abbrev.append(input.at(i));
      }

      // Have reached end of an abbreviation, so add to map
      map[type.tz_abbrind] = abbrev;
   }

   return map;
}

static void parseTzLeapSeconds(QDataStream &stream, int tzh_leapcnt, bool longTran)
{
   // Parse tzh_leapcnt x pairs of leap seconds
   // We don't use leap seconds, so only read and don't store
   qint64 val;

   if (longTran) {
      qint64 time;

      for (int i = 0; i < tzh_leapcnt && stream.status() == QDataStream::Ok; ++i) {
         // Parse Leap Occurrence Time, 8 bytes
         stream >> time;

         // Parse Leap Seconds To Apply, 4 bytes
         if (stream.status() == QDataStream::Ok) {
            stream >> val;
         }
      }

   } else {
      for (int i = 0; i < tzh_leapcnt && stream.status() == QDataStream::Ok; ++i) {
         // Parse Leap Occurrence Time, 4 bytes
         stream >> val;

         // Parse Leap Seconds To Apply, 4 bytes
         if (stream.status() == QDataStream::Ok) {
            stream >> val;
         }
      }
   }
}

static QVector<QTzType> parseTzIndicators(QDataStream &stream, const QVector<QTzType> &types, int tzh_ttisstdcnt, int tzh_ttisgmtcnt)
{
   QVector<QTzType> result = types;
   bool temp;

   // Parse tzh_ttisstdcnt x 1-byte standard/wall indicators
   for (int i = 0; i < tzh_ttisstdcnt && stream.status() == QDataStream::Ok; ++i) {
      stream >> temp;

      if (stream.status() == QDataStream::Ok) {
         result[i].tz_ttisstd = temp;
      }
   }

   // Parse tzh_ttisgmtcnt x 1-byte UTC/local indicators
   for (int i = 0; i < tzh_ttisgmtcnt && stream.status() == QDataStream::Ok; ++i) {
      stream >> temp;

      if (stream.status() == QDataStream::Ok) {
         result[i].tz_ttisgmt = temp;
      }
   }

   return result;
}

static QByteArray parseTzPosixRule(QDataStream &stream)
{
   // Parse POSIX rule, variable length '\n' enclosed
   QByteArray rule;

   quint8 ch;
   stream >> ch;

   if (ch != '\n' || stream.status() != QDataStream::Ok) {
      return rule;
   }

   stream >> ch;

   while (ch != '\n' && stream.status() == QDataStream::Ok) {
      rule.append((char)ch);
      stream >> ch;
   }

   return rule;
}

static QDate calculateDowDate(int year, int month, int dayOfWeek, int week)
{
   QDate date(year, month, 1);
   int startDow = date.dayOfWeek();

   if (startDow <= dayOfWeek) {
      date = date.addDays(dayOfWeek - startDow - 7);
   } else {
      date = date.addDays(dayOfWeek - startDow);
   }

   date = date.addDays(week * 7);

   while (date.month() != month) {
      date = date.addDays(-7);
   }

   return date;
}

static QDate calculatePosixDate(const QByteArray &dateRule, int year)
{
   // Can start with M, J, or a digit
   if (dateRule.at(0) == 'M') {
      // nth week in month format "Mmonth.week.dow"
      QList<QByteArray> dateParts = dateRule.split('.');

      int month = dateParts.at(0).mid(1).toInt();
      int week  = dateParts.at(1).toInt();
      int dow   = dateParts.at(2).toInt();

      if (dow == 0) {
         ++dow;
      }

      return calculateDowDate(year, month, dow, week);

   } else if (dateRule.at(0) == 'J') {
      // Day of Year ignores Feb 29
      int doy = dateRule.mid(1).toInt();
      QDate date = QDate(year, 1, 1).addDays(doy - 1);

      if (QDate::isLeapYear(date.year())) {
         date = date.addDays(-1);
      }

      return date;

   } else {
      // Day of Year includes Feb 29
      int doy = dateRule.toInt();
      return QDate(year, 1, 1).addDays(doy - 1);
   }
}

// returns the time in seconds, INT_MIN if we failed to parse
static int parsePosixTime(const char *begin, const char *end)
{
   // Format "hh[:mm[:ss]]"
   int hour;
   int min = 0;
   int sec = 0;

   // Note that the calls to qstrtoll do *not* check the end pointer, which
   // means they proceed until they find a non-digit. We check that we're
   // still in range at the end, but we may have read from past end. It's the
   // caller's responsibility to ensure that begin is part of a null-terminated string.

   bool ok = false;
   hour = qstrtoll(begin, &begin, 10, &ok);

   if (!ok || hour < 0) {
      return INT_MIN;
   }

   if (begin < end && *begin == ':') {
      // minutes
      ++begin;

      min = qstrtoll(begin, &begin, 10, &ok);

      if (! ok || min < 0) {
         return INT_MIN;
      }

      if (begin < end && *begin == ':') {
         // seconds
         ++begin;

         sec = qstrtoll(begin, &begin, 10, &ok);

         if (!ok || sec < 0) {
            return INT_MIN;
         }
      }
   }

   // we must have consumed everything
   if (begin != end) {
      return INT_MIN;
   }

   return (hour * 60 + min) * 60 + sec;
}

static QTime parsePosixTransitionTime(const QByteArray &timeRule)
{
   // Format "hh[:mm[:ss]]"
   int value = parsePosixTime(timeRule.constBegin(), timeRule.constEnd());

   if (value == INT_MIN) {
      // if we failed to parse, return 02:00
      return QTime(2, 0, 0);
   }

   return QTime::fromMSecsSinceStartOfDay(value * 1000);
}

static int parsePosixOffset(const char *begin, const char *end)
{
   // Format "[+|-]hh[:mm[:ss]]"
   // note that the sign is inverted because POSIX counts in hours West of GMT
   bool negate = true;

   if (*begin == '+') {
      ++begin;
   } else if (*begin == '-') {
      negate = false;
      ++begin;
   }

   int value = parsePosixTime(begin, end);

   if (value == INT_MIN) {
      return value;
   }

   return negate ? -value : value;
}

static inline bool asciiIsLetter(char ch)
{
   return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static QPair<QString, int> parsePosixZoneNameAndOffset(const char *&pos, const char *end)
{
   static const char offsetChars[] = "0123456789:";
   QPair<QString, int> result = qMakePair(QString(), INT_MIN);

   const char *nameBegin = pos;
   const char *nameEnd;
   Q_ASSERT(pos < end);

   if (*pos == '<') {
      nameBegin = pos + 1;    // skip the '<'
      nameEnd = nameBegin;

      while (nameEnd < end && *nameEnd != '>') {
         // POSIX says only alphanumeric, but we allow anything
         ++nameEnd;
      }

      pos = nameEnd + 1;      // skip the '>'

   } else {
      nameBegin = pos;
      nameEnd = nameBegin;

      while (nameEnd < end && asciiIsLetter(*nameEnd)) {
         ++nameEnd;
      }

      pos = nameEnd;
   }

   if (nameEnd - nameBegin < 3) {
      return result;   // name must be at least 3 characters long
   }

   // zone offset, form [+-]hh:mm:ss
   const char *zoneBegin = pos;
   const char *zoneEnd   = pos;

   if (zoneEnd < end && (zoneEnd[0] == '+' || zoneEnd[0] == '-')) {
      ++zoneEnd;
   }

   while (zoneEnd < end) {
      if (strchr(offsetChars, char(*zoneEnd)) == nullptr) {
         break;
      }

      ++zoneEnd;
   }

   result.first = QString::fromUtf8(nameBegin, nameEnd - nameBegin);

   if (zoneEnd > zoneBegin) {
      result.second = parsePosixOffset(zoneBegin, zoneEnd);
   }

   pos = zoneEnd;

   return result;
}

static QVector<QTimeZonePrivate::Data> calculatePosixTransitions(const QByteArray &posixRule,
      int startYear, int endYear, int lastTranMSecs)
{
   QVector<QTimeZonePrivate::Data> result;

   // Limit year by qint64 max size for msecs
   if (startYear > 292278994) {
      startYear = 292278994;
   }

   if (endYear > 292278994) {
      endYear = 292278994;
   }

   // POSIX Format is like "TZ=CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
   // i.e. "std offset dst [offset],start[/time],end[/time]"
   // See the section about TZ at http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap08.html
   QList<QByteArray> parts = posixRule.split(',');

   QPair<QString, int> stdZone, dstZone;
   {
      const QByteArray &zoneinfo = parts.at(0);
      const char *begin = zoneinfo.constBegin();

      stdZone = parsePosixZoneNameAndOffset(begin, zoneinfo.constEnd());

      if (stdZone.second == INT_MIN) {
         stdZone.second = 0;     // reset to UTC if we failed to parse
      } else if (begin < zoneinfo.constEnd()) {
         dstZone = parsePosixZoneNameAndOffset(begin, zoneinfo.constEnd());

         if (dstZone.second == INT_MIN) {
            // if the dst offset isn't provided, it is 1 hour ahead of the standard offset
            dstZone.second = stdZone.second + (60 * 60);
         }
      }
   }

   // If only the name part then no transitions
   if (parts.count() == 1) {
      QTimeZonePrivate::Data data;
      data.atMSecsSinceEpoch = lastTranMSecs;
      data.offsetFromUtc = stdZone.second;
      data.standardTimeOffset = stdZone.second;
      data.daylightTimeOffset = 0;
      data.abbreviation = stdZone.first;
      result << data;
      return result;
   }

   // Get the std to dst transtion details
   QList<QByteArray> dstParts = parts.at(1).split('/');
   QByteArray dstDateRule = dstParts.at(0);
   QTime dstTime;

   if (dstParts.count() > 1) {
      dstTime = parsePosixTransitionTime(dstParts.at(1));
   } else {
      dstTime = QTime(2, 0, 0);
   }

   // Get the dst to std transtion details
   QList<QByteArray> stdParts = parts.at(2).split('/');
   QByteArray stdDateRule = stdParts.at(0);
   QTime stdTime;

   if (stdParts.count() > 1) {
      stdTime = parsePosixTransitionTime(stdParts.at(1));
   } else {
      stdTime = QTime(2, 0, 0);
   }

   for (int year = startYear; year <= endYear; ++year) {
      QTimeZonePrivate::Data dstData;
      QDateTime dst(calculatePosixDate(dstDateRule, year), dstTime, QTimeZone::utc());
      dstData.atMSecsSinceEpoch = dst.toMSecsSinceEpoch() - (stdZone.second * 1000);
      dstData.offsetFromUtc = dstZone.second;
      dstData.standardTimeOffset = stdZone.second;
      dstData.daylightTimeOffset = dstZone.second - stdZone.second;
      dstData.abbreviation = dstZone.first;

      QTimeZonePrivate::Data stdData;
      QDateTime std(calculatePosixDate(stdDateRule, year), stdTime, QTimeZone::utc());
      stdData.atMSecsSinceEpoch = std.toMSecsSinceEpoch() - (dstZone.second * 1000);
      stdData.offsetFromUtc = stdZone.second;
      stdData.standardTimeOffset = stdZone.second;
      stdData.daylightTimeOffset = 0;
      stdData.abbreviation = stdZone.first;

      // Part of the high year will overflow
      if (year == 292278994 && (dstData.atMSecsSinceEpoch < 0 || stdData.atMSecsSinceEpoch < 0)) {
         if (dstData.atMSecsSinceEpoch > 0) {
            result << dstData;
         } else if (stdData.atMSecsSinceEpoch > 0) {
            result << stdData;
         }

      } else if (dst < std) {
         result << dstData << stdData;
      } else {
         result << stdData << dstData;
      }
   }

   return result;
}

// Create the system default time zone
QTzTimeZonePrivate::QTzTimeZonePrivate()
{
   init(systemTimeZoneId());
}

// Create a named time zone
QTzTimeZonePrivate::QTzTimeZonePrivate(const QByteArray &ianaId)
{
   init(ianaId);
}

QTzTimeZonePrivate::QTzTimeZonePrivate(const QTzTimeZonePrivate &other)
   : QTimeZonePrivate(other), m_tranTimes(other.m_tranTimes),
     m_tranRules(other.m_tranRules), m_abbreviations(other.m_abbreviations),
     m_posixRule(other.m_posixRule)
{
}

QTzTimeZonePrivate::~QTzTimeZonePrivate()
{
}

QTimeZonePrivate *QTzTimeZonePrivate::clone()
{
   return new QTzTimeZonePrivate(*this);
}

void QTzTimeZonePrivate::init(const QByteArray &ianaId)
{
   QFile tzif;

   if (ianaId.isEmpty()) {
      // Open system tz
      tzif.setFileName("/etc/localtime");

      if (! tzif.open(QIODevice::ReadOnly)) {
         return;
      }

   } else {
      // Open named tz, try modern path first, if fails try legacy path
      tzif.setFileName("/usr/share/zoneinfo/" + QString::fromUtf8(ianaId));

      if (!tzif.open(QIODevice::ReadOnly)) {
         tzif.setFileName("/usr/lib/zoneinfo/" + QString::fromUtf8(ianaId));

         if (! tzif.open(QIODevice::ReadOnly)) {
            return;
         }
      }
   }

   QDataStream ds(&tzif);

   // Parse the old version block of data
   bool ok = false;

   QTzHeader hdr = parseTzHeader(ds, &ok);

   if (! ok || ds.status() != QDataStream::Ok) {
      return;
   }

   QVector<QTzTransition> tranList = parseTzTransitions(ds, hdr.tzh_timecnt, false);

   if (ds.status() != QDataStream::Ok) {
      return;
   }

   QVector<QTzType> typeList = parseTzTypes(ds, hdr.tzh_typecnt);

   if (ds.status() != QDataStream::Ok) {
      return;
   }

   QMap<int, QByteArray> abbrevMap = parseTzAbbreviations(ds, hdr.tzh_charcnt, typeList);

   if (ds.status() != QDataStream::Ok) {
      return;
   }

   parseTzLeapSeconds(ds, hdr.tzh_leapcnt, false);

   if (ds.status() != QDataStream::Ok) {
      return;
   }

   typeList = parseTzIndicators(ds, typeList, hdr.tzh_ttisstdcnt, hdr.tzh_ttisgmtcnt);

   if (ds.status() != QDataStream::Ok) {
      return;
   }

   // If version 2 then parse the second block of data
   if (hdr.tzh_version == '2' || hdr.tzh_version == '3') {
      ok = false;

      QTzHeader hdr2 = parseTzHeader(ds, &ok);

      if (! ok || ds.status() != QDataStream::Ok) {
         return;
      }

      tranList = parseTzTransitions(ds, hdr2.tzh_timecnt, true);

      if (ds.status() != QDataStream::Ok) {
         return;
      }

      typeList = parseTzTypes(ds, hdr2.tzh_typecnt);

      if (ds.status() != QDataStream::Ok) {
         return;
      }

      abbrevMap = parseTzAbbreviations(ds, hdr2.tzh_charcnt, typeList);

      if (ds.status() != QDataStream::Ok) {
         return;
      }

      parseTzLeapSeconds(ds, hdr2.tzh_leapcnt, true);

      if (ds.status() != QDataStream::Ok) {
         return;
      }

      typeList = parseTzIndicators(ds, typeList, hdr2.tzh_ttisstdcnt, hdr2.tzh_ttisgmtcnt);

      if (ds.status() != QDataStream::Ok) {
         return;
      }

      m_posixRule = parseTzPosixRule(ds);

      if (ds.status() != QDataStream::Ok) {
         return;
      }
   }

   // Translate the TZ file into internal format

   // Translate the array index based tz_abbrind into list index
   m_abbreviations = abbrevMap.values();
   QList<int> abbrindList = abbrevMap.keys();

   for (int i = 0; i < typeList.size(); ++i) {
      typeList[i].tz_abbrind = abbrindList.indexOf(typeList.at(i).tz_abbrind);
   }

   // Offsets are stored as total offset, want to know separate UTC and DST offsets
   // so find the first non-dst transition to use as base UTC Offset
   int utcOffset = 0;

   for (const QTzTransition &tran : tranList) {
      if (!typeList.at(tran.tz_typeind).tz_isdst) {
         utcOffset = typeList.at(tran.tz_typeind).tz_gmtoff;
         break;
      }
   }

   // Now for each transition time calculate our rule and save them
   m_tranTimes.reserve(tranList.count());

   for (const QTzTransition &tz_tran : tranList) {
      QTzTransitionTime tran;
      QTzTransitionRule rule;
      const QTzType tz_type = typeList.at(tz_tran.tz_typeind);

      // Calculate the associated Rule
      if (!tz_type.tz_isdst) {
         utcOffset = tz_type.tz_gmtoff;
      }

      rule.stdOffset = utcOffset;
      rule.dstOffset = tz_type.tz_gmtoff - utcOffset;
      rule.abbreviationIndex = tz_type.tz_abbrind;
      // If the rule already exist then use that, otherwise add it
      int ruleIndex = m_tranRules.indexOf(rule);

      if (ruleIndex == -1) {
         m_tranRules.append(rule);
         tran.ruleIndex = m_tranRules.size() - 1;
      } else {
         tran.ruleIndex = ruleIndex;
      }

      // TODO convert to UTC if not in UTC
      if (tz_type.tz_ttisgmt) {
         tran.atMSecsSinceEpoch = tz_tran.tz_time * 1000;
      } else if (tz_type.tz_ttisstd) {
         tran.atMSecsSinceEpoch = tz_tran.tz_time * 1000;
      } else {
         tran.atMSecsSinceEpoch = tz_tran.tz_time * 1000;
      }

      m_tranTimes.append(tran);
   }

   if (ianaId.isEmpty()) {
      m_id = systemTimeZoneId();
   } else {
      m_id = ianaId;
   }
}

QLocale::Country QTzTimeZonePrivate::country() const
{
   return tzZones()->value(m_id).country;
}

QString QTzTimeZonePrivate::comment() const
{
   return QString::fromUtf8(tzZones()->value(m_id).comment);
}

QString QTzTimeZonePrivate::displayName(qint64 atMSecsSinceEpoch, QTimeZone::NameType nameType, const QLocale &locale) const
{
   (void) nameType;
   (void) locale;

   return abbreviation(atMSecsSinceEpoch);
}

QString QTzTimeZonePrivate::displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType, const QLocale &locale) const
{
   (void) nameType;
   (void) locale;

   // If no ICU available then have to use abbreviations instead
   // Abbreviations don't have GenericTime
   if (timeType == QTimeZone::GenericTime) {
      timeType = QTimeZone::StandardTime;
   }

   // Get current tran, if valid and is what we want, then use it
   const qint64 currentMSecs = QDateTime::currentMSecsSinceEpoch();
   QTimeZonePrivate::Data tran = data(currentMSecs);

   if (tran.atMSecsSinceEpoch != invalidMSecs()
         && ((timeType == QTimeZone::DaylightTime && tran.daylightTimeOffset != 0)
         || (timeType == QTimeZone::StandardTime && tran.daylightTimeOffset == 0))) {
      return tran.abbreviation;
   }

   // Otherwise get next tran and if valid and is what we want, then use it
   tran = nextTransition(currentMSecs);

   if (tran.atMSecsSinceEpoch != invalidMSecs()
         && ((timeType == QTimeZone::DaylightTime && tran.daylightTimeOffset != 0)
         || (timeType == QTimeZone::StandardTime && tran.daylightTimeOffset == 0))) {
      return tran.abbreviation;
   }

   // Otherwise get prev tran and if valid and is what we want, then use it
   tran = previousTransition(currentMSecs);

   if (tran.atMSecsSinceEpoch != invalidMSecs()) {
      tran = previousTransition(tran.atMSecsSinceEpoch);
   }

   if (tran.atMSecsSinceEpoch != invalidMSecs()
         && ((timeType == QTimeZone::DaylightTime && tran.daylightTimeOffset != 0)
         || (timeType == QTimeZone::StandardTime && tran.daylightTimeOffset == 0))) {
      return tran.abbreviation;
   }

   // Otherwise is strange sequence, so work backwards through trans looking for first match, if any
   for (int i = m_tranTimes.size() - 1; i >= 0; --i) {
      if (m_tranTimes.at(i).atMSecsSinceEpoch <= currentMSecs) {
         tran = dataForTzTransition(m_tranTimes.at(i));

         if ((timeType == QTimeZone::DaylightTime && tran.daylightTimeOffset != 0)
               || (timeType == QTimeZone::StandardTime && tran.daylightTimeOffset == 0)) {
            return tran.abbreviation;
         }
      }
   }

   // Otherwise if no match use current data
   return data(currentMSecs).abbreviation;
}

QString QTzTimeZonePrivate::abbreviation(qint64 atMSecsSinceEpoch) const
{
   return data(atMSecsSinceEpoch).abbreviation;
}

int QTzTimeZonePrivate::offsetFromUtc(qint64 atMSecsSinceEpoch) const
{
   const QTimeZonePrivate::Data tran = data(atMSecsSinceEpoch);
   return tran.standardTimeOffset + tran.daylightTimeOffset;
}

int QTzTimeZonePrivate::standardTimeOffset(qint64 atMSecsSinceEpoch) const
{
   return data(atMSecsSinceEpoch).standardTimeOffset;
}

int QTzTimeZonePrivate::daylightTimeOffset(qint64 atMSecsSinceEpoch) const
{
   return data(atMSecsSinceEpoch).daylightTimeOffset;
}

bool QTzTimeZonePrivate::hasDaylightTime() const
{
   // TODO Perhaps cache as frequently accessed?
   for (const QTzTransitionRule &rule : m_tranRules) {
      if (rule.dstOffset != 0) {
         return true;
      }
   }

   return false;
}

bool QTzTimeZonePrivate::isDaylightTime(qint64 atMSecsSinceEpoch) const
{
   return (daylightTimeOffset(atMSecsSinceEpoch) != 0);
}

QTimeZonePrivate::Data QTzTimeZonePrivate::dataForTzTransition(QTzTransitionTime tran) const
{
   QTimeZonePrivate::Data data;
   data.atMSecsSinceEpoch = tran.atMSecsSinceEpoch;

   QTzTransitionRule rule = m_tranRules.at(tran.ruleIndex);
   data.standardTimeOffset = rule.stdOffset;
   data.daylightTimeOffset = rule.dstOffset;
   data.offsetFromUtc = rule.stdOffset + rule.dstOffset;
   data.abbreviation = QString::fromUtf8(m_abbreviations.at(rule.abbreviationIndex));

   return data;
}

QTimeZonePrivate::Data QTzTimeZonePrivate::data(qint64 forMSecsSinceEpoch) const
{
   // If the required time is after the last transition and we have a POSIX rule then use it
   if (m_tranTimes.size() > 0 && m_tranTimes.last().atMSecsSinceEpoch < forMSecsSinceEpoch
         &&  !m_posixRule.isEmpty() && forMSecsSinceEpoch >= 0) {

      const int year = QDateTime::fromMSecsSinceEpoch(forMSecsSinceEpoch, QTimeZone::utc()).date().year();

      QVector<QTimeZonePrivate::Data> posixTrans =
            calculatePosixTransitions(m_posixRule, year - 1, year + 1, m_tranTimes.last().atMSecsSinceEpoch);

      for (int i = posixTrans.size() - 1; i >= 0; --i) {
         if (posixTrans.at(i).atMSecsSinceEpoch <= forMSecsSinceEpoch) {
            QTimeZonePrivate::Data data = posixTrans.at(i);
            data.atMSecsSinceEpoch = forMSecsSinceEpoch;
            return data;
         }
      }
   }

   // Otherwise if we can find a valid tran then use its rule
   for (int i = m_tranTimes.size() - 1; i >= 0; --i) {
      if (m_tranTimes.at(i).atMSecsSinceEpoch <= forMSecsSinceEpoch) {
         Data data = dataForTzTransition(m_tranTimes.at(i));
         data.atMSecsSinceEpoch = forMSecsSinceEpoch;
         return data;
      }
   }

   // Otherwise use the earliest transition we have
   if (m_tranTimes.size() > 0) {
      Data data = dataForTzTransition(m_tranTimes.at(0));
      data.atMSecsSinceEpoch = forMSecsSinceEpoch;
      return data;
   }

   // Otherwise we have no rules, so probably an invalid tz, so return invalid data
   return invalidData();
}

bool QTzTimeZonePrivate::hasTransitions() const
{
   return true;
}

QTimeZonePrivate::Data QTzTimeZonePrivate::nextTransition(qint64 afterMSecsSinceEpoch) const
{
   // If the required time is after the last transition and we have a POSIX rule then use it
   if (m_tranTimes.size() > 0 && m_tranTimes.last().atMSecsSinceEpoch < afterMSecsSinceEpoch
         && ! m_posixRule.isEmpty() && afterMSecsSinceEpoch >= 0) {

      const int year = QDateTime::fromMSecsSinceEpoch(afterMSecsSinceEpoch, QTimeZone::utc()).date().year();

      QVector<QTimeZonePrivate::Data> posixTrans =
            calculatePosixTransitions(m_posixRule, year - 1, year + 1, m_tranTimes.last().atMSecsSinceEpoch);

      for (int i = 0; i < posixTrans.size(); ++i) {
         if (posixTrans.at(i).atMSecsSinceEpoch > afterMSecsSinceEpoch) {
            return posixTrans.at(i);
         }
      }
   }

   // Otherwise if we can find a valid tran then use its rule
   for (int i = 0; i < m_tranTimes.size(); ++i) {
      if (m_tranTimes.at(i).atMSecsSinceEpoch > afterMSecsSinceEpoch) {
         return dataForTzTransition(m_tranTimes.at(i));
      }
   }

   // Otherwise we have no rule, or there is no next transition, so return invalid data
   return invalidData();
}

QTimeZonePrivate::Data QTzTimeZonePrivate::previousTransition(qint64 beforeMSecsSinceEpoch) const
{
   // If the required time is after the last transition and we have a POSIX rule then use it
   if (m_tranTimes.size() > 0 && m_tranTimes.last().atMSecsSinceEpoch < beforeMSecsSinceEpoch
         && !m_posixRule.isEmpty() && beforeMSecsSinceEpoch > 0) {
      const int year = QDateTime::fromMSecsSinceEpoch(beforeMSecsSinceEpoch, QTimeZone::utc()).date().year();

      QVector<QTimeZonePrivate::Data> posixTrans =
            calculatePosixTransitions(m_posixRule, year - 1, year + 1, m_tranTimes.last().atMSecsSinceEpoch);

      for (int i = posixTrans.size() - 1; i >= 0; --i) {
         if (posixTrans.at(i).atMSecsSinceEpoch < beforeMSecsSinceEpoch) {
            return posixTrans.at(i);
         }
      }
   }

   // Otherwise if we can find a valid tran then use its rule
   for (int i = m_tranTimes.size() - 1; i >= 0; --i) {
      if (m_tranTimes.at(i).atMSecsSinceEpoch < beforeMSecsSinceEpoch) {
         return dataForTzTransition(m_tranTimes.at(i));
      }
   }

   // Otherwise we have no rule, so return invalid data
   return invalidData();
}

// TODO Could cache the value and monitor the required files for any changes
QByteArray QTzTimeZonePrivate::systemTimeZoneId() const
{
   // Check TZ env var first, if not populated try find it
   QByteArray ianaId = qgetenv("TZ");

   if (! ianaId.isEmpty() && ianaId.at(0) == ':') {
      ianaId = ianaId.mid(1);
   }

   // the TZ value can be ":/etc/localtime" which libc considers to be a "default timezone",
   // in which case it will be read by one of the blocks below,
   // unset it here so it is not considered as a valid/found ianaId

   if (ianaId == ":/etc/localtime") {
      ianaId.clear();
   }

   // file /etc/localtime maybe the real file with name held in /etc/timezone
   if (ianaId.isEmpty()) {
      QFile tzif("/etc/timezone");

      if (tzif.open(QIODevice::ReadOnly)) {
         // TODO QTextStream inefficient, replace later
         QTextStream ts(&tzif);

         if (! ts.atEnd()) {
            ianaId = ts.readLine().toUtf8();
         }
      }
   }

   // On other distros /etc/localtime is symlink to real file so can extract name from the path
   if (ianaId.isEmpty()) {
      const QString path = QFile::symLinkTarget("/etc/localtime");

      if (! path.isEmpty()) {
         // /etc/localtime is a symlink to the current TZ file, so extract from path
         int index = path.indexOf("/zoneinfo/") + 10;
         ianaId    = path.mid(index).toUtf8();
      }
   }

   // On some Red Hat distros /etc/localtime is real file with name held in /etc/sysconfig/clock
   // in a line like ZONE="Europe/Oslo" or TIMEZONE="Europe/Oslo"
   if (ianaId.isEmpty()) {
      QFile tzif("/etc/sysconfig/clock");

      if (tzif.open(QIODevice::ReadOnly)) {
         // TODO QTextStream inefficient, replace later
         QTextStream ts(&tzif);
         QString line;

         while (ianaId.isEmpty() && ! ts.atEnd() && ts.status() == QTextStream::Ok) {
            line = ts.readLine();

            if (line.startsWith("ZONE=")) {
               ianaId = line.mid(6, line.size() - 7).toUtf8();

            } else if (line.startsWith("TIMEZONE=")) {
               ianaId = line.mid(10, line.size() - 11).toUtf8();
            }
         }
      }
   }

   if (ianaId.isEmpty()) {
      ianaId = "UTC";
   }

   return ianaId;
}

QList<QByteArray> QTzTimeZonePrivate::availableTimeZoneIds() const
{
   QList<QByteArray> result = tzZones()->keys();
   std::sort(result.begin(), result.end());

   return result;
}

QList<QByteArray> QTzTimeZonePrivate::availableTimeZoneIds(QLocale::Country country) const
{
   QList<QByteArray> result;

   for (const QByteArray &key : tzZones()->keys()) {
      if (tzZones()->value(key).country == country) {
         result << key;
      }
   }

   std::sort(result.begin(), result.end());
   return result;
}
