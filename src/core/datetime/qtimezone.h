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

#ifndef QTIMEZONE_H
#define QTIMEZONE_H

#include <qshareddata.h>
#include <qlocale.h>
#include <qdatetime.h>

class QTimeZonePrivate;

class Q_CORE_EXPORT QTimeZone
{
 public:
   enum TimeType {
      StandardTime = 0,
      DaylightTime = 1,
      GenericTime  = 2
   };

   enum NameType {
      DefaultName = 0,
      LongName    = 1,
      ShortName   = 2,
      OffsetName  = 3
   };

   struct OffsetData {
      QString abbreviation;
      QDateTime atUtc;
      int offsetFromUtc;
      int standardTimeOffset;
      int daylightTimeOffset;
   };
   typedef QVector<OffsetData> OffsetDataList;

   QTimeZone();
   explicit QTimeZone(const QByteArray &zoneId);
   explicit QTimeZone(int offsetSeconds);

   QTimeZone(const QByteArray &zoneId, int offsetSeconds, const QString &name,
      const QString &abbreviation, QLocale::Country country = QLocale::AnyCountry, const QString &comment = QString());

   QTimeZone(const QTimeZone &other);
   ~QTimeZone();

   QTimeZone &operator=(const QTimeZone &other);
   QTimeZone &operator=(QTimeZone &&other) {
      swap(other);
      return *this;
   }

   void swap(QTimeZone &other) {
      d.swap(other.d);
   }

   bool operator==(const QTimeZone &other) const;
   bool operator!=(const QTimeZone &other) const;

   bool isValid() const;

   QByteArray id() const;
   QLocale::Country country() const;
   QString comment() const;

   QString displayName(const QDateTime &atDateTime, QTimeZone::NameType nameType = QTimeZone::DefaultName,
      const QLocale &locale = QLocale()) const;

   QString displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType = QTimeZone::DefaultName,
      const QLocale &locale = QLocale()) const;

   QString abbreviation(const QDateTime &atDateTime) const;

   int offsetFromUtc(const QDateTime &atDateTime) const;
   int standardTimeOffset(const QDateTime &atDateTime) const;
   int daylightTimeOffset(const QDateTime &atDateTime) const;

   bool hasDaylightTime() const;
   bool isDaylightTime(const QDateTime &atDateTime) const;

   OffsetData offsetData(const QDateTime &forDateTime) const;

   bool hasTransitions() const;
   OffsetData nextTransition(const QDateTime &afterDateTime) const;
   OffsetData previousTransition(const QDateTime &beforeDateTime) const;
   OffsetDataList transitions(const QDateTime &fromDateTime, const QDateTime &toDateTime) const;

   static QByteArray systemTimeZoneId();
   static QTimeZone systemTimeZone();
   static QTimeZone utc();

   static bool isTimeZoneIdAvailable(const QByteArray &zoneId);

   static QList<QByteArray> availableTimeZoneIds();
   static QList<QByteArray> availableTimeZoneIds(QLocale::Country country);
   static QList<QByteArray> availableTimeZoneIds(int offsetSeconds);

   static QByteArray ianaIdToWindowsId(const QByteArray &zoneId);
   static QByteArray windowsIdToDefaultIanaId(const QByteArray &windowsId);
   static QByteArray windowsIdToDefaultIanaId(const QByteArray &windowsId, QLocale::Country country);
   static QList<QByteArray> windowsIdToIanaIds(const QByteArray &windowsId);
   static QList<QByteArray> windowsIdToIanaIds(const QByteArray &windowsId, QLocale::Country country);

 private:
   QTimeZone(QTimeZonePrivate &dd);

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QTimeZone &tz);
   friend class QTimeZonePrivate;
   friend class QDateTime;
   friend class QDateTimePrivate;

   QSharedDataPointer<QTimeZonePrivate> d;
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QTimeZone &tz);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QTimeZone &tz);

Q_CORE_EXPORT QDebug operator<<(QDebug dbg, const QTimeZone &tz);

#endif // QTIMEZONE_H
