/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <algorithm>

// Create default time zone using appropriate backend
static QTimeZonePrivate *newBackendTimeZone()
{
#ifdef QT_NO_SYSTEMLOCALE
   return new QUtcTimeZonePrivate();

#else

#if defined Q_OS_MAC
   return new QMacTimeZonePrivate();

#elif defined Q_OS_ANDROID
   return new QAndroidTimeZonePrivate();

#elif defined Q_OS_UNIX
   return new QTzTimeZonePrivate();
   // Registry based timezone backend not available on WinRT

#elif defined Q_OS_WIN
   return new QWinTimeZonePrivate();

#else
   return new QUtcTimeZonePrivate();

#endif // System Locales

#endif // QT_NO_SYSTEMLOCALE
}

// Create named time zone using appropriate backend
static QTimeZonePrivate *newBackendTimeZone(const QByteArray &ianaId)
{
#ifdef QT_NO_SYSTEMLOCALE
   return new QUtcTimeZonePrivate(ianaId);

#else

#if defined Q_OS_MAC
   return new QMacTimeZonePrivate(ianaId);

#elif defined Q_OS_ANDROID
   return new QAndroidTimeZonePrivate(ianaId);

#elif defined Q_OS_UNIX
   return new QTzTimeZonePrivate(ianaId);
   // Registry based timezone backend not available on WinRT

#elif defined Q_OS_WIN
   return new QWinTimeZonePrivate(ianaId);

#else
   return new QUtcTimeZonePrivate(ianaId);

#endif // System Locales

#endif // QT_NO_SYSTEMLOCALE
}

class QTimeZoneSingleton
{
 public:
   QTimeZoneSingleton() : backend(newBackendTimeZone()) {}

   // The backend_tz is the tz to use in static methods such as availableTimeZoneIds() and
   // isTimeZoneIdAvailable() and to create named IANA time zones.  This is usually the host
   // system, but may be different if the host resources are insufficient or if
   // QT_NO_SYSTEMLOCALE is set.  A simple UTC backend is used if no alternative is available.
   QSharedDataPointer<QTimeZonePrivate> backend;
};

Q_GLOBAL_STATIC(QTimeZoneSingleton, global_tz);


QTimeZone::QTimeZone()
   : d(0)
{
}

QTimeZone::QTimeZone(const QByteArray &ianaId)
{
   // Try and see if it's a valid UTC offset ID, just as quick to try create as look-up
   d = new QUtcTimeZonePrivate(ianaId);
   // If not a valid UTC offset ID then try create it with the system backend
   // Relies on backend not creating valid tz with invalid name
   if (!d->isValid()) {
      d = newBackendTimeZone(ianaId);
   }
}

QTimeZone::QTimeZone(int offsetSeconds)
{
   // offsetSeconds must fall between -14:00 and +14:00 hours
   if (offsetSeconds >= -50400 && offsetSeconds <= 50400) {
      d = new QUtcTimeZonePrivate(offsetSeconds);
   } else {
      d = 0;
   }
}

/*!
    Creates a custom time zone with an ID of \a ianaId and an offset from UTC
    of \a offsetSeconds.  The \a name will be the name used by displayName()
    for the LongName, the \a abbreviation will be used by displayName() for the
    ShortName and by abbreviation(), and the optional \a country will be used
    by country().  The \a comment is an optional note that may be displayed in
    a GUI to assist users in selecting a time zone.

    The \a ianaId must not be one of the available system IDs returned by
    availableTimeZoneIds().  The \a offsetSeconds from UTC must be in the range
    -14 hours to +14 hours.

    If the custom time zone does not have a specific country then set it to the
    default value of QLocale::AnyCountry.
*/

QTimeZone::QTimeZone(const QByteArray &ianaId, int offsetSeconds, const QString &name,
   const QString &abbreviation, QLocale::Country country, const QString &comment)
   : d()
{
   if (!isTimeZoneIdAvailable(ianaId)) {
      d = new QUtcTimeZonePrivate(ianaId, offsetSeconds, name, abbreviation, country, comment);
   }
}

/*!
    \internal

    Private. Create time zone with given private backend
*/

QTimeZone::QTimeZone(QTimeZonePrivate &dd)
   : d(&dd)
{
}

/*!
    Copy constructor, copy \a other to this.
*/

QTimeZone::QTimeZone(const QTimeZone &other)
   : d(other.d)
{
}

/*!
    Destroys the time zone.
*/

QTimeZone::~QTimeZone()
{
}

/*!
    \fn QTimeZone::swap(QTimeZone &other)

    Swaps this time zone instance with \a other. This function is very
    fast and never fails.
*/

/*!
    Assignment operator, assign \a other to this.
*/

QTimeZone &QTimeZone::operator=(const QTimeZone &other)
{
   d = other.d;
   return *this;
}

/*
    \fn void QTimeZone::swap(QTimeZone &other)

    Swaps this timezone with \a other. This function is very fast and
    never fails.
*/

/*!
    \fn QTimeZone &QTimeZone::operator=(QTimeZone &&other)

    Move-assigns \a other to this QTimeZone instance, transferring the
    ownership of the managed pointer to this instance.
*/

/*!
    Returns \c true if this time zone is equal to the \a other time zone.
*/

bool QTimeZone::operator==(const QTimeZone &other) const
{
   if (d && other.d) {
      return (*d == *other.d);
   } else {
      return (d == other.d);
   }
}

/*!
    Returns \c true if this time zone is not equal to the \a other time zone.
*/

bool QTimeZone::operator!=(const QTimeZone &other) const
{
   if (d && other.d) {
      return (*d != *other.d);
   } else {
      return (d != other.d);
   }
}

/*!
    Returns \c true if this time zone is valid.
*/

bool QTimeZone::isValid() const
{
   if (d) {
      return d->isValid();
   } else {
      return false;
   }
}

/*!
    Returns the IANA ID for the time zone.

    IANA IDs are used on all platforms.  On Windows these are translated
    from the Windows ID into the closest IANA ID for the time zone and country.
*/

QByteArray QTimeZone::id() const
{
   if (d) {
      return d->id();
   } else {
      return QByteArray();
   }
}

/*!
    Returns the country for the time zone.
*/

QLocale::Country QTimeZone::country() const
{
   if (isValid()) {
      return d->country();
   } else {
      return QLocale::AnyCountry;
   }
}

QString QTimeZone::comment() const
{
   if (isValid()) {
      return d->comment();
   } else {
      return QString();
   }
}

QString QTimeZone::displayName(const QDateTime &atDateTime, NameType nameType, const QLocale &locale) const
{
   if (isValid()) {
      return d->displayName(atDateTime.toMSecsSinceEpoch(), nameType, locale);
   } else {
      return QString();
   }
}

QString QTimeZone::displayName(TimeType timeType, NameType nameType, const QLocale &locale) const
{
   if (isValid()) {
      return d->displayName(timeType, nameType, locale);
   } else {
      return QString();
   }
}

QString QTimeZone::abbreviation(const QDateTime &atDateTime) const
{
   if (isValid()) {
      return d->abbreviation(atDateTime.toMSecsSinceEpoch());
   } else {
      return QString();
   }
}

/*!
    Returns the total effective offset at the given \a atDateTime, i.e. the
    number of seconds to add to UTC to obtain the local time.  This includes
    any DST offset that may be in effect, i.e. it is the sum of
    standardTimeOffset() and daylightTimeOffset() for the given datetime.

    For example, for the time zone "Europe/Berlin" the standard time offset is
    +3600 seconds and the DST offset is +3600 seconds.  During standard time
    offsetFromUtc() will return +3600 (UTC+01:00), and during DST it will
    return +7200 (UTC+02:00).

    \sa standardTimeOffset(), daylightTimeOffset()
*/

int QTimeZone::offsetFromUtc(const QDateTime &atDateTime) const
{
   if (isValid()) {
      return d->offsetFromUtc(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

/*!
    Returns the standard time offset at the given \a atDateTime, i.e. the
    number of seconds to add to UTC to obtain the local Standard Time.  This
    excludes any DST offset that may be in effect.

    For example, for the time zone "Europe/Berlin" the standard time offset is
    +3600 seconds.  During both standard and DST offsetFromUtc() will return
    +3600 (UTC+01:00).

    \sa offsetFromUtc(), daylightTimeOffset()
*/

int QTimeZone::standardTimeOffset(const QDateTime &atDateTime) const
{
   if (isValid()) {
      return d->standardTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

/*!
    Returns the daylight-saving time offset at the given \a atDateTime,
    i.e. the number of seconds to add to the standard time offset to obtain the
    local daylight-saving time.

    For example, for the time zone "Europe/Berlin" the DST offset is +3600
    seconds.  During standard time daylightTimeOffset() will return 0, and when
    daylight-saving is in effect it will return +3600.

    \sa offsetFromUtc(), standardTimeOffset()
*/

int QTimeZone::daylightTimeOffset(const QDateTime &atDateTime) const
{
   if (hasDaylightTime()) {
      return d->daylightTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

/*!
    Returns \c true if the time zone has practiced daylight-saving at any time.

    \sa isDaylightTime(), daylightTimeOffset()
*/

bool QTimeZone::hasDaylightTime() const
{
   if (isValid()) {
      return d->hasDaylightTime();
   } else {
      return false;
   }
}

bool QTimeZone::isDaylightTime(const QDateTime &atDateTime) const
{
   if (hasDaylightTime()) {
      return d->isDaylightTime(atDateTime.toMSecsSinceEpoch());
   } else {
      return false;
   }
}

QTimeZone::OffsetData QTimeZone::offsetData(const QDateTime &forDateTime) const
{
   if (hasTransitions()) {
      return QTimeZonePrivate::toOffsetData(d->data(forDateTime.toMSecsSinceEpoch()));
   } else {
      return QTimeZonePrivate::invalidOffsetData();
   }
}

bool QTimeZone::hasTransitions() const
{
   if (isValid()) {
      return d->hasTransitions();
   } else {
      return false;
   }
}

QTimeZone::OffsetData QTimeZone::nextTransition(const QDateTime &afterDateTime) const
{
   if (hasTransitions()) {
      return QTimeZonePrivate::toOffsetData(d->nextTransition(afterDateTime.toMSecsSinceEpoch()));
   } else {
      return QTimeZonePrivate::invalidOffsetData();
   }
}

QTimeZone::OffsetData QTimeZone::previousTransition(const QDateTime &beforeDateTime) const
{
   if (hasTransitions()) {
      return QTimeZonePrivate::toOffsetData(d->previousTransition(beforeDateTime.toMSecsSinceEpoch()));
   } else {
      return QTimeZonePrivate::invalidOffsetData();
   }
}

QTimeZone::OffsetDataList QTimeZone::transitions(const QDateTime &fromDateTime,
   const QDateTime &toDateTime) const
{
   OffsetDataList list;
   if (hasTransitions()) {
      QTimeZonePrivate::DataList plist = d->transitions(fromDateTime.toMSecsSinceEpoch(),
            toDateTime.toMSecsSinceEpoch());
      list.reserve(plist.count());

      for (const QTimeZonePrivate::Data &pdata : plist) {
         list.append(QTimeZonePrivate::toOffsetData(pdata));
      }
   }
   return list;
}

QByteArray QTimeZone::systemTimeZoneId()
{
   return global_tz()->backend->systemTimeZoneId();
}


QTimeZone QTimeZone::systemTimeZone()
{
   return QTimeZone(QTimeZone::systemTimeZoneId());
}


QTimeZone QTimeZone::utc()
{
   return QTimeZone(QTimeZonePrivate::utcQByteArray());
}

bool QTimeZone::isTimeZoneIdAvailable(const QByteArray &ianaId)
{
   // isValidId is not strictly required, but faster to weed out invalid
   // IDs as availableTimeZoneIds() may be slow
   if (!QTimeZonePrivate::isValidId(ianaId)) {
      return false;
   }
   const QList<QByteArray> tzIds = availableTimeZoneIds();
   return std::binary_search(tzIds.begin(), tzIds.end(), ianaId);
}

static QList<QByteArray> set_union(const QList<QByteArray> &l1, const QList<QByteArray> &l2)
{
   QList<QByteArray> result;
   std::set_union(l1.begin(), l1.end(), l2.begin(), l2.end(), std::back_inserter(result));
   return result;
}

/*!
    Returns a list of all available IANA time zone IDs on this system.

    \sa isTimeZoneIdAvailable()
*/

QList<QByteArray> QTimeZone::availableTimeZoneIds()
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(),
         global_tz()->backend->availableTimeZoneIds());
}

/*!
    Returns a list of all available IANA time zone IDs for a given \a country.

    As a special case, a \a country of Qt::AnyCountry returns those time zones
    that do not have any country related to them, such as UTC.  If you require
    a list of all time zone IDs for all countries then use the standard
    availableTimeZoneIds() method.

    \sa isTimeZoneIdAvailable()
*/

QList<QByteArray> QTimeZone::availableTimeZoneIds(QLocale::Country country)
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(country),
         global_tz()->backend->availableTimeZoneIds(country));
}

/*!
    Returns a list of all available IANA time zone IDs with a given standard
    time offset of \a offsetSeconds.

    \sa isTimeZoneIdAvailable()
*/

QList<QByteArray> QTimeZone::availableTimeZoneIds(int offsetSeconds)
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(offsetSeconds),
         global_tz()->backend->availableTimeZoneIds(offsetSeconds));
}

/*!
    Returns the Windows ID equivalent to the given \a ianaId.

    \sa windowsIdToDefaultIanaId(), windowsIdToIanaIds()
*/

QByteArray QTimeZone::ianaIdToWindowsId(const QByteArray &ianaId)
{
   return QTimeZonePrivate::ianaIdToWindowsId(ianaId);
}

/*!
    Returns the default IANA ID for a given \a windowsId.

    Because a Windows ID can cover several IANA IDs in several different
    countries, this function returns the most frequently used IANA ID with no
    regard for the country and should thus be used with care.  It is usually
    best to request the default for a specific country.

    \sa ianaIdToWindowsId(), windowsIdToIanaIds()
*/

QByteArray QTimeZone::windowsIdToDefaultIanaId(const QByteArray &windowsId)
{
   return QTimeZonePrivate::windowsIdToDefaultIanaId(windowsId);
}

QByteArray QTimeZone::windowsIdToDefaultIanaId(const QByteArray &windowsId, QLocale::Country country)
{
   return QTimeZonePrivate::windowsIdToDefaultIanaId(windowsId, country);
}

QList<QByteArray> QTimeZone::windowsIdToIanaIds(const QByteArray &windowsId)
{
   return QTimeZonePrivate::windowsIdToIanaIds(windowsId);
}


QList<QByteArray> QTimeZone::windowsIdToIanaIds(const QByteArray &windowsId, QLocale::Country country)
{
   return QTimeZonePrivate::windowsIdToIanaIds(windowsId, country);
}

QDataStream &operator<<(QDataStream &ds, const QTimeZone &tz)
{
   tz.d->serialize(ds);
   return ds;
}

QDataStream &operator>>(QDataStream &ds, QTimeZone &tz)
{
   QString ianaId;
   ds >> ianaId;

   if (ianaId == QLatin1String("OffsetFromUtc")) {
      int utcOffset;
      QString name;
      QString abbreviation;
      int country;
      QString comment;
      ds >> ianaId >> utcOffset >> name >> abbreviation >> country >> comment;
      tz = QTimeZone(ianaId.toUtf8(), utcOffset, name, abbreviation, (QLocale::Country) country, comment);
   } else {
      tz = QTimeZone(ianaId.toUtf8());
   }
   return ds;
}

QDebug operator<<(QDebug dbg, const QTimeZone &tz)
{
   QDebugStateSaver saver(dbg);
   //TODO Include backend and data version details?
   dbg.nospace() << "QTimeZone(" << QString::fromUtf8(tz.id()) << ')';
   return dbg;
}



