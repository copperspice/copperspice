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

#include <qxsdfacet_p.h>

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
         return QString("length");
         break;

      case MinimumLength:
         return QString("minLength");
         break;

      case MaximumLength:
         return QString("maxLength");
         break;

      case Pattern:
         return QString("pattern");
         break;

      case WhiteSpace:
         return QString("whiteSpace");
         break;

      case MaximumInclusive:
         return QString("maxInclusive");
         break;

      case MaximumExclusive:
         return QString("maxExclusive");
         break;

      case MinimumInclusive:
         return QString("minInclusive");
         break;

      case MinimumExclusive:
         return QString("minExclusive");
         break;

      case TotalDigits:
         return QString("totalDigits");
         break;

      case FractionDigits:
         return QString("fractionDigits");
         break;

      case Enumeration:
         return QString("enumeration");
         break;
      case Assertion:
         return QString("assertion");
         break;

      case None:
         [[fallthrough]];

      default:
         return QString("none");
         break;
   }
}

