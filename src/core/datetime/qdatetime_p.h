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

#ifndef QDATETIME_P_H
#define QDATETIME_P_H

#include <qatomic.h>
#include <qdatetime.h>
#include <qplatformdefs.h>
#include <qtimezone.h>

class QDateTimePrivate
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

   enum DaylightStatus {
      NoDaylightTime      = -2,
      UnknownDaylightTime = -1,
      StandardTime        = 0,
      DaylightTime        = 1
   };

   enum StatusFlag {
      NullDate            = 0x01,
      NullTime            = 0x02,

      ValidDate           = 0x04, // only the date field
      ValidTime           = 0x08, // only the time field
      ValidDateTime       = 0x10, // whole object (including timezone)

      SetToStandardTime   = 0x40,
      SetToDaylightTime   = 0x80
   };
   using StatusFlags = QFlags<StatusFlag>;

   QDateTimePrivate()
      : m_msecs(0), m_tzUserDefined(false), m_status(NullDate | NullTime)
   { }

   QDateTimePrivate(const QDate &toDate, const QTime &toTime, const QTimeZone &timeZone);

   void checkValidDateTime();

   void clearSetToDaylightStatus() {
      m_status &= ~(SetToStandardTime | SetToDaylightTime);
   }

   void clearValidDateTime() {
      m_status &= ~ValidDateTime;
   }

   DaylightStatus daylightStatus() const;

   QPair<QDate, QTime> getDateTime() const;

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

   void refreshDateTime();

   void setDateTime(const QDate &date, const QTime &time);
   void setDaylightStatus(DaylightStatus status);

   void setValidDateTime() {
      m_status |= ValidDateTime;
   }

   static qint64 minJd() {
      return QDate::minJd();
   }

   static qint64 maxJd() {
      return QDate::maxJd();
   }

   static qint64 zoneMSecsToEpochMSecs(qint64 msecs, const QTimeZone &zone,
         QDate *localDate = nullptr, QTime *localTime = nullptr);

   qint64 m_msecs;
   QTimeZone m_timeZone;

   bool m_tzUserDefined;
   StatusFlags m_status;
};

#endif
