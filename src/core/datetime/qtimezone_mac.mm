/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qdatetime.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qtimezone.h>

#include <qcore_mac_p.h>
#include <qtimezone_p.h>

#include <Foundation/NSTimeZone.h>

#include <algorithm>

// Create the system default time zone
QMacTimeZonePrivate::QMacTimeZonePrivate()
   : m_nstz(nullptr)
{
   // Reset the cached system timezone
   [NSTimeZone resetSystemTimeZone];

   // use system timezone
   m_nstz = [NSTimeZone.systemTimeZone retain];
   m_id   = QString::fromNSString(m_nstz.name).toUtf8();
}

// Create a named time zone
QMacTimeZonePrivate::QMacTimeZonePrivate(const QByteArray &ianaId)
   : m_nstz(nullptr)
{
   init(ianaId);
}

QMacTimeZonePrivate::QMacTimeZonePrivate(const QMacTimeZonePrivate &other)
   : QTimeZonePrivate(other), m_nstz(nullptr)
{
   m_nstz = [other.m_nstz copy];
}

QMacTimeZonePrivate::~QMacTimeZonePrivate()
{
   [m_nstz release];
}

QTimeZonePrivate *QMacTimeZonePrivate::clone()
{
   return new QMacTimeZonePrivate(*this);
}

void QMacTimeZonePrivate::init(const QByteArray &ianaId)
{
   if (availableTimeZoneIds().contains(ianaId)) {
      m_nstz = [[NSTimeZone timeZoneWithName:QCFString::toNSString(QString::fromUtf8(ianaId))] retain];

      if (m_nstz != nullptr) {
         m_id = ianaId;
      }
   }

   if (m_nstz == nullptr) {
      m_nstz = [NSTimeZone.systemTimeZone retain];
      m_id   = QString::fromNSString(m_nstz.name).toUtf8();

   }
}

QString QMacTimeZonePrivate::comment() const
{
   return QCFString::toQString([m_nstz description]);
}

QString QMacTimeZonePrivate::displayName(QTimeZone::TimeType timeType,
      QTimeZone::NameType nameType, const QLocale &locale) const
{
   // Mac does not support OffsetName yet so use standard offset name
   if (nameType == QTimeZone::OffsetName) {
      const Data nowData = data(QDateTime::currentMSecsSinceEpoch());

      // assume if dst the offset if 1 hour
      if (timeType == QTimeZone::DaylightTime && hasDaylightTime()) {
         return isoOffsetFormat(nowData.standardTimeOffset + 3600);
      } else {
         return isoOffsetFormat(nowData.standardTimeOffset);
      }
   }

   NSTimeZoneNameStyle style = NSTimeZoneNameStyleStandard;

   switch (nameType) {
      case QTimeZone::ShortName:
         if (timeType == QTimeZone::DaylightTime) {
            style = NSTimeZoneNameStyleShortDaylightSaving;

         } else if (timeType == QTimeZone::GenericTime) {
            style = NSTimeZoneNameStyleShortGeneric;

         } else {
            style = NSTimeZoneNameStyleShortStandard;
         }

         break;

      case QTimeZone::DefaultName:
      case QTimeZone::LongName:
         if (timeType == QTimeZone::DaylightTime) {
            style = NSTimeZoneNameStyleDaylightSaving;

         } else if (timeType == QTimeZone::GenericTime) {
            style = NSTimeZoneNameStyleGeneric;

         } else {
            style = NSTimeZoneNameStyleStandard;
         }

         break;

      case QTimeZone::OffsetName:
         // Unreachable
         break;
   }

   NSString *macLocaleCode = QCFString::toNSString(locale.name());
   NSLocale *macLocale     = [[NSLocale alloc] initWithLocaleIdentifier:macLocaleCode];
   const QString result    = QCFString::toQString([m_nstz localizedName:style locale:macLocale]);
   [macLocale release];

   return result;
}

QString QMacTimeZonePrivate::abbreviation(qint64 atMSecsSinceEpoch) const
{
   const NSTimeInterval seconds = atMSecsSinceEpoch / 1000.0;
   return QCFString::toQString([m_nstz abbreviationForDate:[NSDate dateWithTimeIntervalSince1970:seconds]]);
}

int QMacTimeZonePrivate::offsetFromUtc(qint64 atMSecsSinceEpoch) const
{
   const NSTimeInterval seconds = atMSecsSinceEpoch / 1000.0;
   return [m_nstz secondsFromGMTForDate:[NSDate dateWithTimeIntervalSince1970:seconds]];
}

int QMacTimeZonePrivate::standardTimeOffset(qint64 atMSecsSinceEpoch) const
{
   return offsetFromUtc(atMSecsSinceEpoch) - daylightTimeOffset(atMSecsSinceEpoch);
}

int QMacTimeZonePrivate::daylightTimeOffset(qint64 atMSecsSinceEpoch) const
{
   const NSTimeInterval seconds = atMSecsSinceEpoch / 1000.0;
   return [m_nstz daylightSavingTimeOffsetForDate:[NSDate dateWithTimeIntervalSince1970:seconds]];
}

bool QMacTimeZonePrivate::hasDaylightTime() const
{
   // TODO No Mac API, assume if has transitions
   return hasTransitions();
}

bool QMacTimeZonePrivate::isDaylightTime(qint64 atMSecsSinceEpoch) const
{
   const NSTimeInterval seconds = atMSecsSinceEpoch / 1000.0;
   return [m_nstz isDaylightSavingTimeForDate:[NSDate dateWithTimeIntervalSince1970:seconds]];
}

QTimeZonePrivate::Data QMacTimeZonePrivate::data(qint64 forMSecsSinceEpoch) const
{
   const NSTimeInterval seconds = forMSecsSinceEpoch / 1000.0;
   NSDate *date = [NSDate dateWithTimeIntervalSince1970:seconds];

   Data data;
   data.atMSecsSinceEpoch  = forMSecsSinceEpoch;
   data.offsetFromUtc      = [m_nstz secondsFromGMTForDate:date];
   data.daylightTimeOffset = [m_nstz daylightSavingTimeOffsetForDate:date];
   data.standardTimeOffset = data.offsetFromUtc - data.daylightTimeOffset;
   data.abbreviation       = QCFString::toQString([m_nstz abbreviationForDate:date]);

   return data;
}

bool QMacTimeZonePrivate::hasTransitions() const
{
   NSDate *epoch      = [NSDate dateWithTimeIntervalSince1970:0];
   const NSDate *date = [m_nstz nextDaylightSavingTimeTransitionAfterDate:epoch];
   const bool result  = ([date timeIntervalSince1970] > [epoch timeIntervalSince1970]);

   return result;
}

QTimeZonePrivate::Data QMacTimeZonePrivate::nextTransition(qint64 afterMSecsSinceEpoch) const
{
   QTimeZonePrivate::Data tran;

   const NSTimeInterval seconds = afterMSecsSinceEpoch / 1000.0;
   NSDate *nextDate = [NSDate dateWithTimeIntervalSince1970:seconds];
   nextDate = [m_nstz nextDaylightSavingTimeTransitionAfterDate:nextDate];

   const NSTimeInterval nextSecs = [nextDate timeIntervalSince1970];

   if (nextDate == nil || nextSecs <= seconds) {
      [nextDate release];
      return invalidData();
   }

   tran.atMSecsSinceEpoch = nextSecs * 1000;
   tran.offsetFromUtc = [m_nstz secondsFromGMTForDate:nextDate];
   tran.daylightTimeOffset = [m_nstz daylightSavingTimeOffsetForDate:nextDate];
   tran.standardTimeOffset = tran.offsetFromUtc - tran.daylightTimeOffset;
   tran.abbreviation = QCFString::toQString([m_nstz abbreviationForDate:nextDate]);

   return tran;
}

QTimeZonePrivate::Data QMacTimeZonePrivate::previousTransition(qint64 beforeMSecsSinceEpoch) const
{
   // No direct Mac API, get all transitions since epoch and return the last one
   QList<int> secsList;

   if (beforeMSecsSinceEpoch > 0) {
      const int endSecs = beforeMSecsSinceEpoch / 1000.0;
      NSTimeInterval prevSecs = 0;
      NSTimeInterval nextSecs = 0;
      NSDate *nextDate = [NSDate dateWithTimeIntervalSince1970:nextSecs];

      // If invalid may return a nil date or an Epoch date
      nextDate = [m_nstz nextDaylightSavingTimeTransitionAfterDate:nextDate];
      nextSecs = [nextDate timeIntervalSince1970];

      while (nextDate != nil && nextSecs > prevSecs && nextSecs < endSecs) {
         secsList.append(nextSecs);
         prevSecs = nextSecs;
         nextDate = [m_nstz nextDaylightSavingTimeTransitionAfterDate:nextDate];
         nextSecs = [nextDate timeIntervalSince1970];
      }
   }

   if (secsList.size() >= 1) {
      return data(qint64(secsList.last()) * 1000);
   } else {
      return invalidData();
   }
}

QByteArray QMacTimeZonePrivate::systemTimeZoneId() const
{
   // Reset the cached system tz then return the name
   [NSTimeZone resetSystemTimeZone];

   return QCFString::toQString([[NSTimeZone systemTimeZone] name]).toUtf8();
}

QList<QByteArray> QMacTimeZonePrivate::availableTimeZoneIds() const
{
   NSEnumerator *enumerator = [[NSTimeZone knownTimeZoneNames] objectEnumerator];
   QByteArray tzid = QCFString::toQString([enumerator nextObject]).toUtf8();

   QList<QByteArray> list;

   while (! tzid.isEmpty()) {
      list << tzid;
      tzid = QCFString::toQString([enumerator nextObject]).toUtf8();
   }

   std::sort(list.begin(), list.end());
   list.erase(std::unique(list.begin(), list.end()), list.end());

   return list;
}
