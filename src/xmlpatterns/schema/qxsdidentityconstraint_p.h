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

   /**
    * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#icd-identity-constraint_category">category</a> of the identity constraint.
    */
   enum Category {
      Key = 1,        ///< The constraint is a key constraint
      KeyReference,   ///< The constraint is a keyref constraint
      Unique          ///< The constraint is an unique constraint
   };

   /**
    * Sets the @p category of the identity constraint.
    *
    * @see Category
    */
   void setCategory(Category category);

   /**
    * Returns the category of the identity constraint.
    */
   Category category() const;

   /**
    * Sets the @p selector of the identity constraint.
    *
    * The selector is a restricted XPath 1.0 expression,
    * that selects a set of nodes.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-selector"></a>
    */
   void setSelector(const XsdXPathExpression::Ptr &selector);

   /**
    * Returns the selector of the identity constraint.
    */
   XsdXPathExpression::Ptr selector() const;

   /**
    * Sets the @p fields of the identity constraint.
    *
    * Each field is a restricted XPath 1.0 expression,
    * that selects a set of nodes.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-fields"></a>
    */
   void setFields(const XsdXPathExpression::List &fields);

   /**
    * Adds a new @p field to the identity constraint.
    */
   void addField(const XsdXPathExpression::Ptr &field);

   /**
    * Returns all fields of the identity constraint.
    */
   XsdXPathExpression::List fields() const;

   /**
    * Sets the referenced @p key of the identity constraint.
    *
    * The key points to a identity constraint of type Key or Unique.
    *
    * The identity constraint has only a referenced key if its
    * type is KeyReference.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-referenced_key"></a>
    */
   void setReferencedKey(const XsdIdentityConstraint::Ptr &key);

   /**
    * Returns the referenced key of the identity constraint or an empty
    * pointer if its type is not KeyReference.
    */
   XsdIdentityConstraint::Ptr referencedKey() const;

 private:
   Category                   m_category;
   XsdXPathExpression::Ptr    m_selector;
   XsdXPathExpression::List   m_fields;
   XsdIdentityConstraint::Ptr m_referencedKey;
};

}

#endif
