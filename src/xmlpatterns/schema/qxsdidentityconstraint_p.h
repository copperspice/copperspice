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

#ifndef QXsdIdentityConstraint_P_H
#define QXsdIdentityConstraint_P_H

#include <qstringlist.h>

#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>
#include <qxsdxpathexpression_p.h>

namespace QPatternist {

class XsdIdentityConstraint : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdIdentityConstraint> Ptr;
   typedef QList<XsdIdentityConstraint::Ptr> List;

   enum Category {
      Key = 1,        // constraint is a key constraint
      KeyReference,   // constraint is a keyref constraint
      Unique          // constraint is an unique constraint
   };

   void setCategory(Category category);
   Category category() const;

   void setSelector(const XsdXPathExpression::Ptr &selector);
   XsdXPathExpression::Ptr selector() const;

   void setFields(const XsdXPathExpression::List &fields);
   void addField(const XsdXPathExpression::Ptr &field);

   XsdXPathExpression::List fields() const;
   void setReferencedKey(const XsdIdentityConstraint::Ptr &key);

   XsdIdentityConstraint::Ptr referencedKey() const;

 private:
   Category                   m_category;
   XsdXPathExpression::Ptr    m_selector;
   XsdXPathExpression::List   m_fields;
   XsdIdentityConstraint::Ptr m_referencedKey;
};

}

#endif
