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

#include "qxsdfacet_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

XsdFacet::XsdFacet()
   : m_type(None)
{
}

void XsdFacet::setType(Type type)
{
   m_type = type;
}

XsdFacet::Type XsdFacet::type() const
{
   return m_type;
}

void XsdFacet::setValue(const AtomicValue::Ptr &value)
{
   m_value = value;
}

AtomicValue::Ptr XsdFacet::value() const
{
   return m_value;
}

void XsdFacet::setMultiValue(const AtomicValue::List &value)
{
   m_multiValue = value;
}

AtomicValue::List XsdFacet::multiValue() const
{
   return m_multiValue;
}

void XsdFacet::setAssertions(const XsdAssertion::List &assertions)
{
   m_assertions = assertions;
}

XsdAssertion::List XsdFacet::assertions() const
{
   return m_assertions;
}

void XsdFacet::setFixed(bool fixed)
{
   m_fixed = fixed;
}

bool XsdFacet::fixed() const
{
   return m_fixed;
}

QString XsdFacet::typeName(Type type)
{
   switch (type) {
      case Length:
         return QLatin1String("length");
         break;
      case MinimumLength:
         return QLatin1String("minLength");
         break;
      case MaximumLength:
         return QLatin1String("maxLength");
         break;
      case Pattern:
         return QLatin1String("pattern");
         break;
      case WhiteSpace:
         return QLatin1String("whiteSpace");
         break;
      case MaximumInclusive:
         return QLatin1String("maxInclusive");
         break;
      case MaximumExclusive:
         return QLatin1String("maxExclusive");
         break;
      case MinimumInclusive:
         return QLatin1String("minInclusive");
         break;
      case MinimumExclusive:
         return QLatin1String("minExclusive");
         break;
      case TotalDigits:
         return QLatin1String("totalDigits");
         break;
      case FractionDigits:
         return QLatin1String("fractionDigits");
         break;
      case Enumeration:
         return QLatin1String("enumeration");
         break;
      case Assertion:
         return QLatin1String("assertion");
         break;
      case None: // fall through
      default:
         return QLatin1String("none");
         break;
   }
}

QT_END_NAMESPACE
