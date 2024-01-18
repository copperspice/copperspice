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

#ifndef QDateTimeFNs_P_H
#define QDateTimeFNs_P_H

#include <qtimezone.h>

#include <qatomiccomparator_p.h>
#include <qcommonvalues_p.h>
#include <qschemadatetime_p.h>
#include <qdaytimeduration_p.h>
#include <qdecimal_p.h>
#include <qinteger_p.h>
#include <qfunctioncall_p.h>

namespace QPatternist {

template<typename TSubClass>
class ExtractFromDurationFN : public FunctionCall
{
 public:
   /**
    * Takes care of the argument handling, and, if applicable,
    * calls extract() with the value of the operand.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};


class YearsFromDurationFN : public ExtractFromDurationFN<YearsFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};

class MonthsFromDurationFN : public ExtractFromDurationFN<MonthsFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};


class DaysFromDurationFN : public ExtractFromDurationFN<DaysFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};


class HoursFromDurationFN : public ExtractFromDurationFN<HoursFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};


class MinutesFromDurationFN : public ExtractFromDurationFN<MinutesFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};


class SecondsFromDurationFN : public ExtractFromDurationFN<SecondsFromDurationFN>
{
 public:
   inline Item extract(const AbstractDuration *const duration) const;
};


template<typename TSubClass>
class ExtractFromDateTimeFN : public FunctionCall
{
 public:
   /**
    * Takes care of the argument handling, and, if applicable,
    * calls extract() with the value of the operand.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
};


class YearFromAbstractDateTimeFN : public ExtractFromDateTimeFN<YearFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};


class DayFromAbstractDateTimeFN : public ExtractFromDateTimeFN<DayFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};


class HoursFromAbstractDateTimeFN : public ExtractFromDateTimeFN<HoursFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};

class MinutesFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MinutesFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};


class SecondsFromAbstractDateTimeFN : public ExtractFromDateTimeFN<SecondsFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};


class TimezoneFromAbstractDateTimeFN : public ExtractFromDateTimeFN<TimezoneFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};


class MonthFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MonthFromAbstractDateTimeFN>
{
 public:
   inline Item extract(const QDateTime &dt) const;
};

#include "qdatetimefns.cpp"

}

#endif
