/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QDATETIME_P_H
#define QDATETIME_P_H

#include <qplatformdefs.h>
#include <qatomic.h>
#include <qdatetime.h>
#include <qtimezone.h>

class QDateTimePrivate : public QSharedData
{
 public:
   enum Spec {
      LocalUnknown  = -1,
      LocalStandard = 0,
      LocalDST      = 1,
      UTC           = 2,
      OffsetFromUTC = 3,
      TimeZone      = 4
   };

   // Daylight Time Status
   enum DaylightStatus {
      NoDaylightTime = -2,
      UnknownDaylightTime = -1,
      StandardTime = 0,
      DaylightTime = 1
   };

   // Status of date/time
   enum StatusFlag {
      NullDate            = 0x01,
      NullTime            = 0x02,
      ValidDate           = 0x04, // just the date field
      ValidTime           = 0x08, // just the time field
      ValidDateTime       = 0x10, // the whole object (including timezone)
      SetToStandardTime   = 0x40,
      SetToDaylightTime   = 0x80
   };
   using StatusFlags = QFlags<StatusFlag>;



   QDateTimePrivate() : m_msecs(0),
      m_spec(Qt::LocalTime),
      m_offsetFromUtc(0),
      m_status(NullDate | NullTime)
   {}

   QDateTimePrivate(const QDate &toDate, const QTime &toTime, Qt::TimeSpec toSpec, int offsetSeconds);

   QDateTimePrivate(const QDate &toDate, const QTime &toTime, const QTimeZone &timeZone);


   qint64 m_msecs;
   Qt::TimeSpec m_spec;
   int m_offsetFromUtc;

   QTimeZone m_timeZone;
   StatusFlags m_status;

   void setTimeSpec(Qt::TimeSpec spec, int offsetSeconds);
   void setDateTime(const QDate &date, const QTime &time);
   QPair<QDate, QTime> getDateTime() const;
   void setDaylightStatus(DaylightStatus status);
   DaylightStatus daylightStatus() const;

   // Returns msecs since epoch, assumes offset value is current
   inline qint64 toMSecsSinceEpoch() const;

   void checkValidDateTime();

   void refreshDateTime();

   // Get/set date and time status
   bool isNullDate() const {
      return m_status & NullDate;
   }

   bool isNullTime() const {
      return m_status & NullTime;
   }

   bool isValidDate() const {
      return m_status & ValidDate;
   }

   bool isValidTime() const {
      return m_status & ValidTime;
   }


   bool isValidDateTime() const {
      return m_status & ValidDateTime;
   }

   void setValidDateTime() {
      m_status |= ValidDateTime;
   }

   void clearValidDateTime() {
      m_status &= ~ValidDateTime;
   }

   inline void clearSetToDaylightStatus() {
      m_status &= ~(SetToStandardTime | SetToDaylightTime);
   }

   static qint64 zoneMSecsToEpochMSecs(qint64 msecs, const QTimeZone &zone,
      QDate *localDate = nullptr, QTime *localTime = nullptr);

   static inline qint64 minJd() {
      return QDate::minJd();
   }
   static inline qint64 maxJd() {
      return QDate::maxJd();
   }
};

#endif
