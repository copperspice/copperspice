/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qdatastream.h>
#include <qdebug.h>

#include <qtimezone_p.h>
#include <qtimezone_data_p.h>

#include <algorithm>

enum {
   MSECS_TRAN_WINDOW = 21600000 // 6 hour window for possible recent transitions
};

/*
    Static utilities for looking up Windows ID tables
*/

static const int windowsDataTableSize = sizeof(windowsDataTable) / sizeof(QWindowsData) - 1;
static const int zoneDataTableSize = sizeof(zoneDataTable) / sizeof(QZoneData) - 1;
static const int utcDataTableSize = sizeof(utcDataTable) / sizeof(QUtcData) - 1;

static const QZoneData *zoneData(quint16 index)
{
   Q_ASSERT(index < zoneDataTableSize);
   return &zoneDataTable[index];
}

static const QWindowsData *windowsData(quint16 index)
{
   Q_ASSERT(index < windowsDataTableSize);
   return &windowsDataTable[index];
}

static const QUtcData *utcData(quint16 index)
{
   Q_ASSERT(index < utcDataTableSize);
   return &utcDataTable[index];
}

// Return the Windows ID literal for a given QWindowsData
static QByteArray windowsId(const QWindowsData *windowsData)
{
   return (windowsIdData + windowsData->windowsIdIndex);
}

// Return the IANA ID literal for a given QWindowsData
static QByteArray ianaId(const QWindowsData *windowsData)
{
   return (ianaIdData + windowsData->ianaIdIndex);
}

// Return the IANA ID literal for a given QZoneData
static QByteArray ianaId(const QZoneData *zoneData)
{
   return (ianaIdData + zoneData->ianaIdIndex);
}

static QByteArray utcId(const QUtcData *utcData)
{
   return (ianaIdData + utcData->ianaIdIndex);
}

static quint16 toWindowsIdKey(const QByteArray &winId)
{
   for (quint16 i = 0; i < windowsDataTableSize; ++i) {
      const QWindowsData *data = windowsData(i);
      if (windowsId(data) == winId) {
         return data->windowsIdKey;
      }
   }
   return 0;
}

static QByteArray toWindowsIdLiteral(quint16 windowsIdKey)
{
   for (quint16 i = 0; i < windowsDataTableSize; ++i) {
      const QWindowsData *data = windowsData(i);
      if (data->windowsIdKey == windowsIdKey) {
         return windowsId(data);
      }
   }
   return QByteArray();
}

/*
    Base class implementing common utility routines, only intantiate for a null tz.
*/

QTimeZonePrivate::QTimeZonePrivate()
{
}

QTimeZonePrivate::QTimeZonePrivate(const QTimeZonePrivate &other)
   : QSharedData(other), m_id(other.m_id)
{
}

QTimeZonePrivate::~QTimeZonePrivate()
{
}

QTimeZonePrivate *QTimeZonePrivate::clone()
{
   return new QTimeZonePrivate(*this);
}

bool QTimeZonePrivate::operator==(const QTimeZonePrivate &other) const
{
   // TODO Too simple, but need to solve problem of comparing different derived classes
   // Should work for all System and ICU classes as names guaranteed unique, but not for Simple.
   // Perhaps once all classes have working transitions can compare full list?
   return (m_id == other.m_id);
}

bool QTimeZonePrivate::operator!=(const QTimeZonePrivate &other) const
{
   return !(*this == other);
}

bool QTimeZonePrivate::isValid() const
{
   return !m_id.isEmpty();
}

QByteArray QTimeZonePrivate::id() const
{
   return m_id;
}

QLocale::Country QTimeZonePrivate::country() const
{
   // Default fall-back mode, use the zoneTable to find Region of known Zones
   for (int i = 0; i < zoneDataTableSize; ++i) {
      const QZoneData *data = zoneData(i);
      if (ianaId(data).split(' ').contains(m_id)) {
         return (QLocale::Country)data->country;
      }
   }
   return QLocale::AnyCountry;
}

QString QTimeZonePrivate::comment() const
{
   return QString();
}

QString QTimeZonePrivate::displayName(qint64 atMSecsSinceEpoch,
   QTimeZone::NameType nameType,
   const QLocale &locale) const
{
   if (nameType == QTimeZone::OffsetName) {
      return isoOffsetFormat(offsetFromUtc(atMSecsSinceEpoch));
   }

   if (isDaylightTime(atMSecsSinceEpoch)) {
      return displayName(QTimeZone::DaylightTime, nameType, locale);
   } else {
      return displayName(QTimeZone::StandardTime, nameType, locale);
   }
}

QString QTimeZonePrivate::displayName(QTimeZone::TimeType timeType,
   QTimeZone::NameType nameType, const QLocale &locale) const
{
   (void) timeType;
   (void) nameType;
   (void) locale;

   return QString();
}

QString QTimeZonePrivate::abbreviation(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;

   return QString();
}

int QTimeZonePrivate::offsetFromUtc(qint64 atMSecsSinceEpoch) const
{
   return standardTimeOffset(atMSecsSinceEpoch) + daylightTimeOffset(atMSecsSinceEpoch);
}

int QTimeZonePrivate::standardTimeOffset(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;

   return invalidSeconds();
}

int QTimeZonePrivate::daylightTimeOffset(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;
   return invalidSeconds();
}

bool QTimeZonePrivate::hasDaylightTime() const
{
   return false;
}

bool QTimeZonePrivate::isDaylightTime(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;
   return false;
}

QTimeZonePrivate::Data QTimeZonePrivate::data(qint64 forMSecsSinceEpoch) const
{
   (void) forMSecsSinceEpoch;
   return invalidData();
}

// Private only method for use by QDateTime to convert local msecs to epoch msecs
// TODO Could be platform optimised if needed
QTimeZonePrivate::Data QTimeZonePrivate::dataForLocalTime(qint64 forLocalMSecs) const
{
   if (!hasDaylightTime() || !hasTransitions()) {
      // No DST means same offset for all local msecs
      // Having DST but no transitions means we can't calculate, so use nearest
      return data(forLocalMSecs - (standardTimeOffset(forLocalMSecs) * 1000));
   }

   // Get the transition for the local msecs which most of the time should be the right one
   // Only around the transition times might it not be the right one
   Data tran = previousTransition(forLocalMSecs);
   Data nextTran;

   // If the local msecs is less than the real local time of the transition
   // then get the previous transition to use instead
   if (forLocalMSecs < tran.atMSecsSinceEpoch + (tran.offsetFromUtc * 1000)) {
      while (tran.atMSecsSinceEpoch != invalidMSecs()
         && forLocalMSecs < tran.atMSecsSinceEpoch + (tran.offsetFromUtc * 1000)) {
         nextTran = tran;
         tran = previousTransition(tran.atMSecsSinceEpoch);
      }
   } else {
      // The zone msecs is after the transition, so check it is before the next tran
      // If not try use the next transition instead
      nextTran = nextTransition(tran.atMSecsSinceEpoch);
      while (nextTran.atMSecsSinceEpoch != invalidMSecs()
         && forLocalMSecs >= nextTran.atMSecsSinceEpoch + (nextTran.offsetFromUtc * 1000)) {
         tran = nextTran;
         nextTran = nextTransition(tran.atMSecsSinceEpoch);
      }
   }

   if (tran.daylightTimeOffset == 0) {
      // If tran is in StandardTime, then need to check if falls close to either DST transition.
      // If it does, then it may need adjusting for missing hour or for second occurrence
      qint64 diffPrevTran = forLocalMSecs
         - (tran.atMSecsSinceEpoch + (tran.offsetFromUtc * 1000));
      qint64 diffNextTran = nextTran.atMSecsSinceEpoch + (nextTran.offsetFromUtc * 1000)
         - forLocalMSecs;
      if (diffPrevTran >= 0 && diffPrevTran < MSECS_TRAN_WINDOW) {
         // If tran picked is for standard time check if changed from DST in last 6 hours,
         // as the local msecs may be ambiguous and represent two valid utc msecs.
         // If in last 6 hours then get prev tran and if diff falls within the DST offset
         // then use the prev tran as we default to the FirstOccurrence
         // TODO Check if faster to just always get prev tran, or if faster using 6 hour check.
         Data dstTran = previousTransition(tran.atMSecsSinceEpoch);
         if (dstTran.atMSecsSinceEpoch != invalidMSecs()
            && dstTran.daylightTimeOffset > 0 && diffPrevTran < (dstTran.daylightTimeOffset * 1000)) {
            tran = dstTran;
         }
      } else if (diffNextTran >= 0 && diffNextTran <= (nextTran.daylightTimeOffset * 1000)) {
         // If time falls within last hour of standard time then is actually the missing hour
         // So return the next tran instead and adjust the local time to be valid
         tran = nextTran;
         forLocalMSecs = forLocalMSecs + (nextTran.daylightTimeOffset * 1000);
      }
   }

   // tran should now hold the right transition offset to use
   tran.atMSecsSinceEpoch = forLocalMSecs - (tran.offsetFromUtc * 1000);
   return tran;
}

bool QTimeZonePrivate::hasTransitions() const
{
   return false;
}

QTimeZonePrivate::Data QTimeZonePrivate::nextTransition(qint64 afterMSecsSinceEpoch) const
{
   (void) afterMSecsSinceEpoch;
   return invalidData();
}

QTimeZonePrivate::Data QTimeZonePrivate::previousTransition(qint64 beforeMSecsSinceEpoch) const
{
   (void) beforeMSecsSinceEpoch;
   return invalidData();
}

QTimeZonePrivate::DataList QTimeZonePrivate::transitions(qint64 fromMSecsSinceEpoch,
   qint64 toMSecsSinceEpoch) const
{
   DataList list;
   if (toMSecsSinceEpoch >= fromMSecsSinceEpoch) {
      // fromMSecsSinceEpoch is inclusive but nextTransitionTime() is exclusive so go back 1 msec
      Data next = nextTransition(fromMSecsSinceEpoch - 1);
      while (next.atMSecsSinceEpoch != invalidMSecs()
         && next.atMSecsSinceEpoch <= toMSecsSinceEpoch) {
         list.append(next);
         next = nextTransition(next.atMSecsSinceEpoch);
      }
   }
   return list;
}

QByteArray QTimeZonePrivate::systemTimeZoneId() const
{
   return QByteArray();
}

QList<QByteArray> QTimeZonePrivate::availableTimeZoneIds() const
{
   return QList<QByteArray>();
}

QList<QByteArray> QTimeZonePrivate::availableTimeZoneIds(QLocale::Country country) const
{
   // Default fall-back mode, use the zoneTable to find Region of know Zones
   QList<QByteArray> regions;

   // First get all Zones in the Zones table belonging to the Region
   for (int i = 0; i < zoneDataTableSize; ++i) {
      if (zoneData(i)->country == country) {
         regions += ianaId(zoneData(i)).split(' ');
      }
   }

   std::sort(regions.begin(), regions.end());
   regions.erase(std::unique(regions.begin(), regions.end()), regions.end());

   // Then select just those that are available
   const QList<QByteArray> all = availableTimeZoneIds();
   QList<QByteArray> result;

   std::set_intersection(all.begin(), all.end(), regions.cbegin(), regions.cend(),
      std::back_inserter(result));
   return result;
}

QList<QByteArray> QTimeZonePrivate::availableTimeZoneIds(int offsetFromUtc) const
{
   // Default fall-back mode, use the zoneTable to find Offset of know Zones
   QList<QByteArray> offsets;
   // First get all Zones in the table using the Offset
   for (int i = 0; i < windowsDataTableSize; ++i) {
      const QWindowsData *winData = windowsData(i);
      if (winData->offsetFromUtc == offsetFromUtc) {
         for (int j = 0; j < zoneDataTableSize; ++j) {
            const QZoneData *data = zoneData(j);
            if (data->windowsIdKey == winData->windowsIdKey) {
               offsets += ianaId(data).split(' ');
            }
         }
      }
   }

   std::sort(offsets.begin(), offsets.end());
   offsets.erase(std::unique(offsets.begin(), offsets.end()), offsets.end());

   // Then select just those that are available
   const QList<QByteArray> all = availableTimeZoneIds();
   QList<QByteArray> result;

   std::set_intersection(all.begin(), all.end(), offsets.cbegin(), offsets.cend(),
      std::back_inserter(result));
   return result;
}

void QTimeZonePrivate::serialize(QDataStream &stream) const
{
   // leave this as a QByteArray
   stream << m_id;
}

// Static Utility Methods

QTimeZonePrivate::Data QTimeZonePrivate::invalidData()
{
   Data data;
   data.atMSecsSinceEpoch = invalidMSecs();
   data.offsetFromUtc = invalidSeconds();
   data.standardTimeOffset = invalidSeconds();
   data.daylightTimeOffset = invalidSeconds();
   return data;
}

QTimeZone::OffsetData QTimeZonePrivate::invalidOffsetData()
{
   QTimeZone::OffsetData offsetData;
   offsetData.atUtc = QDateTime();
   offsetData.offsetFromUtc = invalidSeconds();
   offsetData.standardTimeOffset = invalidSeconds();
   offsetData.daylightTimeOffset = invalidSeconds();
   return offsetData;
}

QTimeZone::OffsetData QTimeZonePrivate::toOffsetData(const QTimeZonePrivate::Data &data)
{
   QTimeZone::OffsetData offsetData = invalidOffsetData();
   if (data.atMSecsSinceEpoch != invalidMSecs()) {
      offsetData.atUtc = QDateTime::fromMSecsSinceEpoch(data.atMSecsSinceEpoch, Qt::UTC);
      offsetData.offsetFromUtc = data.offsetFromUtc;
      offsetData.standardTimeOffset = data.standardTimeOffset;
      offsetData.daylightTimeOffset = data.daylightTimeOffset;
      offsetData.abbreviation = data.abbreviation;
   }
   return offsetData;
}

// Is the format of the ID valid ?
bool QTimeZonePrivate::isValidId(const QByteArray &ianaId)
{
   /*
     Main rules for defining TZ/IANA names as per ftp://ftp.iana.org/tz/code/Theory
      1. Use only valid POSIX file name components
      2. Within a file name component, use only ASCII letters, `.', `-' and `_'.
      3. Do not use digits (except in a [+-]\d+ suffix, when used).
      4. A file name component must not exceed 14 characters or start with `-'
     However, the rules are really guidelines - a later one says
      - Do not change established names if they only marginally violate the
        above rules.
     We may, therefore, need to be a bit slack in our check here, if we hit
     legitimate exceptions in real time-zone databases.

     In particular, aliases such as "Etc/GMT+7" and "SystemV/EST5EDT" are valid
     so we need to accept digits, ':', and '+'; aliases typically have the form
     of POSIX TZ strings, which allow a suffix to a proper IANA name.  A POSIX
     suffix starts with an offset (as in GMT+7) and may continue with another
     name (as in EST5EDT, giving the DST name of the zone); a further offset is
     allowed (for DST).  The ("hard to describe and [...] error-prone in
     practice") POSIX form even allows a suffix giving the dates (and
     optionally times) of the annual DST transitions.  Hopefully, no TZ aliases
     go that far, but we at least need to accept an offset and (single
     fragment) DST-name.

     But for the legacy complications, the following would be preferable if
     QRegExp would work on QByteArrays directly:
         const QRegExp rx(QString("[a-z+._][a-z+._-]{,13}"
                                     "(?:/[a-z+._][a-z+._-]{,13})*"
                                         // Optional suffix:
                                         "(?:[+-]?\d{1,2}(?::\d{1,2}){,2}" // offset
                                            // one name fragment (DST):
                                            "(?:[a-z+._][a-z+._-]{,13})?)"),
                          Qt::CaseInsensitive);
         return rx.exactMatch(ianaId);
   */

   // Somewhat slack hand-rolled version:
   const int MinSectionLength = 1;
   const int MaxSectionLength = 14;
   int sectionLength = 0;
   for (const char *it = ianaId.begin(), * const end = ianaId.end(); it != end; ++it, ++sectionLength) {
      const char ch = *it;
      if (ch == '/') {
         if (sectionLength < MinSectionLength || sectionLength > MaxSectionLength) {
            return false;   // violates (4)
         }
         sectionLength = -1;
      } else if (ch == '-') {
         if (sectionLength == 0) {
            return false;   // violates (4)
         }
      } else if (!(ch >= 'a' && ch <= 'z')
         && !(ch >= 'A' && ch <= 'Z')
         && !(ch == '_')
         && !(ch == '.')
         // Should ideally check these only happen as an offset:
         && !(ch >= '0' && ch <= '9')
         && !(ch == '+')
         && !(ch == ':')) {
         return false; // violates (2)
      }
   }
   if (sectionLength < MinSectionLength || sectionLength > MaxSectionLength) {
      return false;   // violates (4)
   }
   return true;
}

QString QTimeZonePrivate::isoOffsetFormat(int offsetFromUtc)
{
   const int mins = offsetFromUtc / 60;
   return QString::fromUtf8("UTC%1%2:%3").formatArg(mins >= 0 ? '+' : '-')
      .formatArg(qAbs(mins) / 60, 2, 10, QLatin1Char('0'))
      .formatArg(qAbs(mins) % 60, 2, 10, QLatin1Char('0'));
}

QByteArray QTimeZonePrivate::ianaIdToWindowsId(const QByteArray &id)
{
   for (int i = 0; i < zoneDataTableSize; ++i) {
      const QZoneData *data = zoneData(i);
      if (ianaId(data).split(' ').contains(id)) {
         return toWindowsIdLiteral(data->windowsIdKey);
      }
   }
   return QByteArray();
}

QByteArray QTimeZonePrivate::windowsIdToDefaultIanaId(const QByteArray &windowsId)
{
   const quint16 windowsIdKey = toWindowsIdKey(windowsId);
   for (int i = 0; i < windowsDataTableSize; ++i) {
      const QWindowsData *data = windowsData(i);
      if (data->windowsIdKey == windowsIdKey) {
         return ianaId(data);
      }
   }
   return QByteArray();
}

QByteArray QTimeZonePrivate::windowsIdToDefaultIanaId(const QByteArray &windowsId,
   QLocale::Country country)
{
   const QList<QByteArray> list = windowsIdToIanaIds(windowsId, country);
   if (list.count() > 0) {
      return list.first();
   } else {
      return QByteArray();
   }
}

QList<QByteArray> QTimeZonePrivate::windowsIdToIanaIds(const QByteArray &windowsId)
{
   const quint16 windowsIdKey = toWindowsIdKey(windowsId);
   QList<QByteArray> list;

   for (int i = 0; i < zoneDataTableSize; ++i) {
      const QZoneData *data = zoneData(i);
      if (data->windowsIdKey == windowsIdKey) {
         list << ianaId(data).split(' ');
      }
   }

   // Return the full list in alpha order
   std::sort(list.begin(), list.end());
   return list;
}

QList<QByteArray> QTimeZonePrivate::windowsIdToIanaIds(const QByteArray &windowsId, QLocale::Country country)
{
   const quint16 windowsIdKey = toWindowsIdKey(windowsId);
   for (int i = 0; i < zoneDataTableSize; ++i) {
      const QZoneData *data = zoneData(i);
      // Return the region matches in preference order
      if (data->windowsIdKey == windowsIdKey && data->country == (quint16) country) {
         return ianaId(data).split(' ');
      }
   }

   return QList<QByteArray>();
}

/*
    UTC Offset implementation, used when QT_NO_SYSTEMLOCALE set
    or for QDateTimes with a Qt:Spec of Qt::OffsetFromUtc.
*/

// Create default UTC time zone
QUtcTimeZonePrivate::QUtcTimeZonePrivate()
{
   const QString name = utcQString();
   init(utcQByteArray(), 0, name, name, QLocale::AnyCountry, name);
}

// Create a named UTC time zone
QUtcTimeZonePrivate::QUtcTimeZonePrivate(const QByteArray &id)
{
   // Look for the name in the UTC list, if found set the values
   for (int i = 0; i < utcDataTableSize; ++i) {
      const QUtcData *data = utcData(i);
      const QByteArray uid = utcId(data);

      if (uid == id) {
         QString name = QString::fromUtf8(id);
         init(id, data->offsetFromUtc, name, name, QLocale::AnyCountry, name);
         break;
      }
   }
}

// Create offset from UTC
QUtcTimeZonePrivate::QUtcTimeZonePrivate(qint32 offsetSeconds)
{
   QString utcId;

   if (offsetSeconds == 0) {
      utcId = utcQString();
   } else {
      utcId = isoOffsetFormat(offsetSeconds);
   }

   init(utcId.toUtf8(), offsetSeconds, utcId, utcId, QLocale::AnyCountry, utcId);
}

QUtcTimeZonePrivate::QUtcTimeZonePrivate(const QByteArray &zoneId, int offsetSeconds,
   const QString &name, const QString &abbreviation,
   QLocale::Country country, const QString &comment)
{
   init(zoneId, offsetSeconds, name, abbreviation, country, comment);
}

QUtcTimeZonePrivate::QUtcTimeZonePrivate(const QUtcTimeZonePrivate &other)
   : QTimeZonePrivate(other), m_name(other.m_name),
     m_abbreviation(other.m_abbreviation),
     m_comment(other.m_comment),
     m_country(other.m_country),
     m_offsetFromUtc(other.m_offsetFromUtc)
{
}

QUtcTimeZonePrivate::~QUtcTimeZonePrivate()
{
}

QTimeZonePrivate *QUtcTimeZonePrivate::clone()
{
   return new QUtcTimeZonePrivate(*this);
}

QTimeZonePrivate::Data QUtcTimeZonePrivate::data(qint64 forMSecsSinceEpoch) const
{
   Data d;
   d.abbreviation = m_abbreviation;
   d.atMSecsSinceEpoch = forMSecsSinceEpoch;
   d.standardTimeOffset = d.offsetFromUtc = m_offsetFromUtc;
   d.daylightTimeOffset = 0;
   return d;
}

void QUtcTimeZonePrivate::init(const QByteArray &zoneId)
{
   m_id = zoneId;
}

void QUtcTimeZonePrivate::init(const QByteArray &zoneId, int offsetSeconds, const QString &name,
   const QString &abbreviation, QLocale::Country country,
   const QString &comment)
{
   m_id = zoneId;
   m_offsetFromUtc = offsetSeconds;
   m_name = name;
   m_abbreviation = abbreviation;
   m_country = country;
   m_comment = comment;
}

QLocale::Country QUtcTimeZonePrivate::country() const
{
   return m_country;
}

QString QUtcTimeZonePrivate::comment() const
{
   return m_comment;
}

QString QUtcTimeZonePrivate::displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType, const QLocale &locale) const
{
   (void) timeType;
   (void) locale;

   if (nameType == QTimeZone::ShortName) {
      return m_abbreviation;

   } else if (nameType == QTimeZone::OffsetName) {
      return isoOffsetFormat(m_offsetFromUtc);
   }

   return m_name;
}

QString QUtcTimeZonePrivate::abbreviation(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;
   return m_abbreviation;
}

qint32 QUtcTimeZonePrivate::standardTimeOffset(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;
   return m_offsetFromUtc;
}

qint32 QUtcTimeZonePrivate::daylightTimeOffset(qint64 atMSecsSinceEpoch) const
{
   (void) atMSecsSinceEpoch;
   return 0;
}

QByteArray QUtcTimeZonePrivate::systemTimeZoneId() const
{
   return utcQByteArray();
}

QList<QByteArray> QUtcTimeZonePrivate::availableTimeZoneIds() const
{
   QList<QByteArray> result;

   for (int i = 0; i < utcDataTableSize; ++i) {
      result << utcId(utcData(i));
   }

   std::sort(result.begin(), result.end()); // ### or already sorted??
   // ### assuming no duplicates

   return result;
}

QList<QByteArray> QUtcTimeZonePrivate::availableTimeZoneIds(QLocale::Country country) const
{
   // If AnyCountry then is request for all non-region offset codes
   if (country == QLocale::AnyCountry) {
      return availableTimeZoneIds();
   }
   return QList<QByteArray>();
}

QList<QByteArray> QUtcTimeZonePrivate::availableTimeZoneIds(qint32 offsetSeconds) const
{
   QList<QByteArray> result;
   for (int i = 0; i < utcDataTableSize; ++i) {
      const QUtcData *data = utcData(i);
      if (data->offsetFromUtc == offsetSeconds) {
         result << utcId(data);
      }
   }

   std::sort(result.begin(), result.end()); // ### or already sorted??
   // ### assuming no duplicates

   return result;
}

void QUtcTimeZonePrivate::serialize(QDataStream &stream) const
{
   // first entry must be a QByteArray

   stream << QByteArray("OffsetFromUtc") << QString::fromUtf8(m_id) << m_offsetFromUtc << m_name
          << m_abbreviation << (qint32) m_country << m_comment;
}
