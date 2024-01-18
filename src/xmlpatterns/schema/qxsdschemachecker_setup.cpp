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

#include "qxsdschemachecker_p.h"

using namespace QPatternist;

void XsdSchemaChecker::setupAllowedAtomicFacets()
{
   // string
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsString->name(m_namePool), facets);
   }

   // boolean
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsBoolean->name(m_namePool), facets);
   }

   // float
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsFloat->name(m_namePool), facets);
   }

   // double
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsDouble->name(m_namePool), facets);
   }

   // decimal
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::TotalDigits
             << XsdFacet::FractionDigits
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsDecimal->name(m_namePool), facets);
   }

   // duration
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsDuration->name(m_namePool), facets);
   }

   // dateTime
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsDateTime->name(m_namePool), facets);
   }

   // time
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsTime->name(m_namePool), facets);
   }

   // date
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsDate->name(m_namePool), facets);
   }

   // gYearMonth
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsGYearMonth->name(m_namePool), facets);
   }

   // gYear
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsGYear->name(m_namePool), facets);
   }

   // gMonthDay
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsGMonthDay->name(m_namePool), facets);
   }

   // gDay
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsGDay->name(m_namePool), facets);
   }

   // gMonth
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::MaximumInclusive
             << XsdFacet::MaximumExclusive
             << XsdFacet::MinimumInclusive
             << XsdFacet::MinimumExclusive
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsGMonth->name(m_namePool), facets);
   }

   // hexBinary
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsHexBinary->name(m_namePool), facets);
   }

   // base64Binary
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsBase64Binary->name(m_namePool), facets);
   }

   // anyURI
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsAnyURI->name(m_namePool), facets);
   }

   // QName
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsQName->name(m_namePool), facets);
   }

   // NOTATION
   {
      QSet<XsdFacet::Type> facets;
      facets << XsdFacet::Length
             << XsdFacet::MinimumLength
             << XsdFacet::MaximumLength
             << XsdFacet::Pattern
             << XsdFacet::Enumeration
             << XsdFacet::WhiteSpace
             << XsdFacet::Assertion;

      m_allowedAtomicFacets.insert(BuiltinTypes::xsNOTATION->name(m_namePool), facets);
   }
}
