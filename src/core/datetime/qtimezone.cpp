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

#if defined Q_OS_DARWIN
   return new QMacTimeZonePrivate();

#elif defined Q_OS_ANDROID
   return new QAndroidTimeZonePrivate();

#elif defined Q_OS_UNIX
   return new QTzTimeZonePrivate();

#elif defined Q_OS_WIN
   return new QWinTimeZonePrivate();

#else
   return new QUtcTimeZonePrivate();

#endif

#endif // QT_NO_SYSTEMLOCALE
}

// Create named time zone using appropriate backend
static QTimeZonePrivate *newBackendTimeZone(const QByteArray &ianaId)
{
#ifdef QT_NO_SYSTEMLOCALE
   return new QUtcTimeZonePrivate(ianaId);

#else

#if defined Q_OS_DARWIN
   return new QMacTimeZonePrivate(ianaId);

#elif defined Q_OS_ANDROID
   return new QAndroidTimeZonePrivate(ianaId);

#elif defined Q_OS_UNIX
   return new QTzTimeZonePrivate(ianaId);

#elif defined Q_OS_WIN
   return new QWinTimeZonePrivate(ianaId);

#else
   return new QUtcTimeZonePrivate(ianaId);

#endif

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

static QTimeZoneSingleton *global_tz()
{
   static QTimeZoneSingleton retval;
   return &retval;
}

QTimeZone::QTimeZone()
   : d(nullptr)
{
}

QTimeZone::QTimeZone(const QByteArray &ianaId)
{
   // see if ianaId is a valid UTC offset ID
   d = new QUtcTimeZonePrivate(ianaId);

   if (! d->isValid()) {
      // create with the system backend
      d = newBackendTimeZone(ianaId);
   }
}

QTimeZone::QTimeZone(int offsetSeconds)
{
   // offsetSeconds must fall between -14:00 and +14:00 hours
   if (offsetSeconds >= -50400 && offsetSeconds <= 50400) {
      d = new QUtcTimeZonePrivate(offsetSeconds);
   } else {
      d = nullptr;
   }
}

QTimeZone::QTimeZone(const QByteArray &ianaId, int offsetSeconds, const QString &name,
            const QString &abbreviation, QLocale::Country country, const QString &comment)
   : d()
{
   if (! isTimeZoneIdAvailable(ianaId)) {
      d = new QUtcTimeZonePrivate(ianaId, offsetSeconds, name, abbreviation, country, comment);
   }
}

// internal
QTimeZone::QTimeZone(QTimeZonePrivate &dd)
   : d(&dd)
{
}

QTimeZone::QTimeZone(const QTimeZone &other)
   : d(other.d)
{
}

QTimeZone::~QTimeZone()
{
}

QTimeZone &QTimeZone::operator=(const QTimeZone &other)
{
   d = other.d;
   return *this;
}

bool QTimeZone::operator==(const QTimeZone &other) const
{
   if (d && other.d) {
      return (*d == *other.d);
   } else {
      return (d == other.d);
   }
}

bool QTimeZone::operator!=(const QTimeZone &other) const
{
   if (d && other.d) {
      return (*d != *other.d);
   } else {
      return (d != other.d);
   }
}

bool QTimeZone::isValid() const
{
   if (d) {
      return d->isValid();
   } else {
      return false;
   }
}

QByteArray QTimeZone::id() const
{
   if (d) {
      return d->id();
   } else {
      return QByteArray();
   }
}

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

int QTimeZone::offsetFromUtc(const QDateTime &atDateTime) const
{
   if (isValid()) {
      return d->offsetFromUtc(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

int QTimeZone::standardTimeOffset(const QDateTime &atDateTime) const
{
   if (isValid()) {
      return d->standardTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

int QTimeZone::daylightTimeOffset(const QDateTime &atDateTime) const
{
   if (hasDaylightTime()) {
      return d->daylightTimeOffset(atDateTime.toMSecsSinceEpoch());
   } else {
      return 0;
   }
}

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

QList<QByteArray> QTimeZone::availableTimeZoneIds()
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(),
         global_tz()->backend->availableTimeZoneIds());
}

QList<QByteArray> QTimeZone::availableTimeZoneIds(QLocale::Country country)
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(country),
         global_tz()->backend->availableTimeZoneIds(country));
}

QList<QByteArray> QTimeZone::availableTimeZoneIds(int offsetSeconds)
{
   return set_union(QUtcTimeZonePrivate().availableTimeZoneIds(offsetSeconds),
         global_tz()->backend->availableTimeZoneIds(offsetSeconds));
}

QByteArray QTimeZone::ianaIdToWindowsId(const QByteArray &ianaId)
{
   return QTimeZonePrivate::ianaIdToWindowsId(ianaId);
}

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

QDataStream &operator<<(QDataStream &stream, const QTimeZone &tz)
{
   tz.d->serialize(stream);
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QTimeZone &tz)
{
   QByteArray ianaId;            // must be a QByteArray
   stream >> ianaId;

   if (ianaId == "OffsetFromUtc") {
      int utcOffset;
      QString name;
      QString abbreviation;
      int country;               // QLocale::Country
      QString comment;

      stream >> ianaId >> utcOffset >> name >> abbreviation >> country >> comment;
      tz = QTimeZone(ianaId, utcOffset, name, abbreviation,
                  static_cast<QLocale::Country>(country), comment);

   } else {
      tz = QTimeZone(ianaId);
   }

   return stream;
}

QDebug operator<<(QDebug dbg, const QTimeZone &tz)
{
   QDebugStateSaver saver(dbg);

   // TODO Include backend and data version details?
   dbg.nospace() << "QTimeZone(" << QString::fromUtf8(tz.id()) << ')';

   return dbg;
}
