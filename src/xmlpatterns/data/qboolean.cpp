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

#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qdynamiccontext_p.h"
#include "qpatternistlocale_p.h"
#include "qvalidationerror_p.h"

#include "qboolean_p.h"

using namespace QPatternist;

bool Boolean::evaluateEBV(const Item::Iterator::Ptr &it,
                          const QExplicitlySharedDataPointer<DynamicContext> &context)
{
   return evaluateEBV(it->next(), it, context);
}

bool Boolean::evaluateEBV(const Item &first,
                          const Item::Iterator::Ptr &it,
                          const QExplicitlySharedDataPointer<DynamicContext> &context)
{
   Q_ASSERT(it);
   Q_ASSERT(context);

   if (!first) {
      return false;
   } else if (first.isNode()) {
      return true;
   }

   const Item second(it->next());

   if (second) {
      Q_ASSERT(context);
      context->error(QtXmlPatterns::tr("Effective Boolean Value cannot be calculated for a sequence "
                                       "containing two or more atomic values."),
                     ReportContext::FORG0006,
                     QSourceLocation());
      return false;
   } else {
      return first.as<AtomicValue>()->evaluateEBV(context);
   }
}

bool Boolean::evaluateEBV(const Item &item,
                          const QExplicitlySharedDataPointer<DynamicContext> &context)
{
   if (!item) {
      return false;
   } else if (item.isNode()) {
      return true;
   } else {
      return item.as<AtomicValue>()->evaluateEBV(context);
   }
}

Boolean::Boolean(const bool value) : m_value(value)
{
}

QString Boolean::stringValue() const
{
   return m_value
          ? CommonValues::TrueString->stringValue()
          : CommonValues::FalseString->stringValue();
}

bool Boolean::evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const
{
   return m_value;
}

Boolean::Ptr Boolean::fromValue(const bool value)
{
   return value ? CommonValues::BooleanTrue : CommonValues::BooleanFalse;
}

AtomicValue::Ptr Boolean::fromLexical(const QString &lexical)
{
   const QString val(lexical.trimmed()); /* Apply the whitespace facet. */

   if (val == "true" || val == "1") {
      return CommonValues::BooleanTrue;

   } else if (val == "false" || val == "0") {
      return CommonValues::BooleanFalse;

   } else {
      return ValidationError::createError();
   }
}

ItemType::Ptr Boolean::type() const
{
   return BuiltinTypes::xsBoolean;
}
