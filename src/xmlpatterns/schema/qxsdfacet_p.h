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

#ifndef QXsdFacet_P_H
#define QXsdFacet_P_H

#include <qlist.h>

#include <qitem_p.h>
#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>
#include <qxsdassertion_p.h>

namespace QPatternist {

class XsdFacet : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdFacet> Ptr;

   enum Type {
      None             = 0,        // An invalid facet.
      Length           = 1 << 0,   // Match the exact length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-length">Length Definition</a>)
      MinimumLength    = 1 << 1,   // Match the minimum length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minLength">Minimum Length Definition</a>)
      MaximumLength    = 1 << 2,   // Match the maximum length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxLength">Maximum Length Definition</a>)
      Pattern          = 1 << 3,   // Match a regular expression (<a href="http://www.w3.org/TR/xmlschema-2/#rf-pattern">Pattern Definition</a>)
      WhiteSpace       = 1 << 4,   // Match a whitespace rule (<a href="http://www.w3.org/TR/xmlschema-2/#rf-whiteSpace">White Space Definition</a>)
      MaximumInclusive = 1 << 5,   // Match a maximum inclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxInclusive">Maximum Inclusive Definition</a>)
      MaximumExclusive = 1 << 6,   // Match a maximum exclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxExclusive">Maximum Exclusive Definition</a>)
      MinimumInclusive = 1 << 7,   // Match a minimum inclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minInclusive">Minimum Inclusive Definition</a>)
      MinimumExclusive = 1 << 8,   // Match a minimum exclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minExclusive">Minimum Exclusive Definition</a>)
      TotalDigits      = 1 << 9,   // Match some integer digits (<a href="http://www.w3.org/TR/xmlschema-2/#rf-totalDigits">Total Digits Definition</a>)
      FractionDigits   = 1 << 10,  // Match some double digits (<a href="http://www.w3.org/TR/xmlschema-2/#rf-fractionDigits">Fraction Digits Definition</a>)
      Enumeration      = 1 << 11,  // Match an enumeration (<a href="http://www.w3.org/TR/xmlschema-2/#rf-enumeration">Enumeration Definition</a>)
      Assertion        = 1 << 12,  // Match an assertion (<a href="http://www.w3.org/TR/xmlschema-2/#rf-assertion">Assertion Definition</a>)
   };

   typedef QHash<XsdFacet::Type, XsdFacet::Ptr> Hash;
   typedef QHashIterator<XsdFacet::Type, XsdFacet::Ptr> HashIterator;

   XsdFacet();

   void setType(Type type);
   Type type() const;

   void setValue(const AtomicValue::Ptr &value);
   AtomicValue::Ptr value() const;

   void setMultiValue(const AtomicValue::List &value);
   AtomicValue::List multiValue() const;

   void setAssertions(const XsdAssertion::List &assertions);
   XsdAssertion::List assertions() const;

   void setFixed(bool fixed);
   bool fixed() const;

   static QString typeName(Type type);

 private:
   Type               m_type;
   AtomicValue::Ptr   m_value;
   AtomicValue::List  m_multiValue;
   XsdAssertion::List m_assertions;
   bool               m_fixed;
};

}

#endif
