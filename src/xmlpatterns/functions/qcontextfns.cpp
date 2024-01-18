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

#include "qanyuri_p.h"
#include "qdate_p.h"
#include "qschemadatetime_p.h"
#include "qdaytimeduration_p.h"
#include "qinteger_p.h"
#include "qliteral_p.h"
#include "qatomicstring_p.h"
#include "qschematime_p.h"
#include "qcontextfns_p.h"

using namespace QPatternist;

Item PositionFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context);
   return Integer::fromValue(context->contextPosition());
}

Item LastFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context);
   return Integer::fromValue(context->contextSize());
}

Item ImplicitTimezoneFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return toItem(context->implicitTimezone());
}

Item CurrentDateTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return toItem(DateTime::fromDateTime(context->currentDateTime()));
}

Item CurrentDateFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return toItem(Date::fromDateTime(context->currentDateTime()));
}

Item CurrentTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return toItem(SchemaTime::fromDateTime(context->currentDateTime()));
}

Expression::Ptr StaticBaseURIFN::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   /* Our base URI can never be undefined. */
   return wrapLiteral(toItem(AnyURI::fromValue(context->baseURI())), context, this)->typeCheck(context, reqType);
}

Expression::Ptr DefaultCollationFN::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   return wrapLiteral(AtomicString::fromValue(context->defaultCollation().toString()), context, this)->typeCheck(context,
          reqType);
}
