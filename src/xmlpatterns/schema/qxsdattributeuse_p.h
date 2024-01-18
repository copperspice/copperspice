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

#ifndef QXsdAttributeUse_P_H
#define QXsdAttributeUse_P_H

#include <qlist.h>

#include <qxsdattribute_p.h>
#include <qxsdattributeterm_p.h>

namespace QPatternist {

class XsdAttributeUse : public XsdAttributeTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAttributeUse> Ptr;
   typedef QList<XsdAttributeUse::Ptr> List;

   /**
    * Describes the value constraint of an attribute use.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_au">Value Constraint Definition</a>
    */
   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      /**
       * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#au-value_constraint">value constraint</a> of an attribute use.
       */
      enum Variety {
         Default,  ///< The attribute use has a default value set.
         Fixed     ///< The attribute use has a fixed value set.
      };

      /**
       * Sets the @p variety of the attribute use value constraint.
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the attribute use value constraint.
       */
      Variety variety() const;

      /**
       * Sets the @p value of the constraint.
       */
      void setValue(const QString &value);

      /**
       * Returns the value of the constraint.
       */
      QString value() const;

      /**
       * Sets the lexical @p form of the constraint.
       */
      void setLexicalForm(const QString &form);

      /**
       * Returns the lexical form of the constraint.
       */
      QString lexicalForm() const;

      /**
       * Creates a new value constraint from a XsdAttribute::ValueConstraint.
       */
      static ValueConstraint::Ptr fromAttributeValueConstraint(const XsdAttribute::ValueConstraint::Ptr &constraint);

    private:
      Variety m_variety;
      QString m_value;
      QString m_lexicalForm;
   };

   /**
    * Describes the use type of the attribute use.
    */
   enum UseType {
      OptionalUse,        ///< The attribute can be there but doesn't need to.
      RequiredUse,        ///< The attribute must be there.
      ProhibitedUse       ///< The attribute is not allowed to be there.
   };

   /**
    * Creates a new attribute use object.
    */
   XsdAttributeUse();

   /**
    * Always returns true, used to avoid dynamic casts.
    */
   bool isAttributeUse() const override;

   /**
    * Sets the use @p type of the attribute use.
    *
    * @see UseType
    */
   void setUseType(UseType type);

   /**
    * Returns the use type of the attribute use.
    */
   UseType useType() const;

   /**
    * Returns whether the attribute use is required.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#au-required">Required Definition</a>
    */
   bool isRequired() const;

   /**
    * Sets the @p attribute the attribute use is referring to.
    * That is either a local definition as child of a complexType
    * or attributeGroup object, or a reference defined by the
    * 'ref' attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#au-attribute_declaration">Attribute Declaration</a>
    */
   void setAttribute(const XsdAttribute::Ptr &attribute);

   /**
    * Returns the attribute the attribute use is referring to.
    */
   XsdAttribute::Ptr attribute() const;

   /**
    * Sets the value @p constraint of the attribute use.
    *
    * @see http://www.w3.org/TR/xmlschema11-1/#vc_au
    */
   void setValueConstraint(const ValueConstraint::Ptr &constraint);

   /**
    * Returns the value constraint of the attribute use.
    */
   ValueConstraint::Ptr valueConstraint() const;

 private:
   UseType              m_useType;
   XsdAttribute::Ptr    m_attribute;
   ValueConstraint::Ptr m_valueConstraint;
};

}

#endif
