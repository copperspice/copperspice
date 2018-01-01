/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qatomiccomparator_p.h"
#include "qcommonvalues_p.h"
#include "qschemadatetime_p.h"
#include "qdaytimeduration_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"
#include "qpatternistlocale_p.h"

#include "qdatetimefn_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Item DateTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item di(m_operands.first()->evaluateSingleton(context));
   if (!di) {
      return Item();
   }

   const Item ti(m_operands.last()->evaluateSingleton(context));
   if (!ti) {
      return Item();
   }

   QDateTime date(di.as<AbstractDateTime>()->toDateTime());
   Q_ASSERT(date.isValid());
   QDateTime time(ti.as<AbstractDateTime>()->toDateTime());
   Q_ASSERT(time.isValid());

   if (date.timeSpec() == time.timeSpec() || /* Identical timezone properties. */
         time.timeSpec() == Qt::LocalTime) { /* time has no timezone, but date do. */
      date.setTime(time.time());
      Q_ASSERT(date.isValid());
      return DateTime::fromDateTime(date);
   } else if (date.timeSpec() == Qt::LocalTime) { /* date has no timezone, but time do. */
      time.setDate(date.date());
      Q_ASSERT(time.isValid());
      return DateTime::fromDateTime(time);
   } else {
      context->error(QtXmlPatterns::tr("If both values have zone offsets, "
                                       "they must have the same zone offset. "
                                       "%1 and %2 are not the same.")
                     .arg(formatData(di.stringValue()),
                          formatData(di.stringValue())),
                     ReportContext::FORG0008, this);
      return Item(); /* Silence GCC warning. */
   }
}

QT_END_NAMESPACE
