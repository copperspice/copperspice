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

template<typename TSubClass>
Item ExtractFromDurationFN<TSubClass>::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));

   if (item) {
      return static_cast<const TSubClass *>(this)->extract(item.as<AbstractDuration>());
   } else {
      return Item();
   }
}

Item YearsFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return Integer::fromValue(duration->years() * (duration->isPositive() ? 1 : -1));
}

Item MonthsFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return Integer::fromValue(duration->months() * (duration->isPositive() ? 1 : -1));
}

Item DaysFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return Integer::fromValue(duration->days() * (duration->isPositive() ? 1 : -1));
}

Item HoursFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return Integer::fromValue(duration->hours() * (duration->isPositive() ? 1 : -1));
}

Item MinutesFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return Integer::fromValue(duration->minutes() * (duration->isPositive() ? 1 : -1));
}

Item SecondsFromDurationFN::extract(const AbstractDuration *const duration) const
{
   return toItem(Decimal::fromValue((duration->seconds() + duration->mseconds() / 1000.0) *
                                    (duration->isPositive() ? 1 : -1)));
}

template<typename TSubClass>
Item ExtractFromDateTimeFN<TSubClass>::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operands.first()->evaluateSingleton(context));
   if (item) {
      return static_cast<const TSubClass *>(this)->
             extract(item.as<AbstractDateTime>()->toDateTime());
   } else {
      return Item();
   }
}

Item YearFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   return Integer::fromValue(dt.date().year());
}

Item DayFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   return Integer::fromValue(dt.date().day());
}

Item MinutesFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   return Integer::fromValue(dt.time().minute());
}

Item SecondsFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   const QTime time(dt.time());
   return toItem(Decimal::fromValue(time.second() + time.msec() / 1000.0));
}

Item TimezoneFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   if (dt.timeZone() == QTimeZone::utc()) {
      return toItem(CommonValues::DayTimeDurationZero);

   } else {
      return toItem(DayTimeDuration::fromSeconds(dt.offsetFromUtc()));

   }
}

Item MonthFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   return Integer::fromValue(dt.date().month());
}

Item HoursFromAbstractDateTimeFN::extract(const QDateTime &dt) const
{
   return Integer::fromValue(dt.time().hour());
}

