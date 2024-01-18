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

#ifndef QXsdAttribute_P_H
#define QXsdAttribute_P_H

#include <qlist.h>

#include <qanysimpletype_p.h>
#include <qnamedschemacomponent_p.h>
#include <qxsdannotated_p.h>

namespace QPatternist {

class XsdAttribute : public NamedSchemaComponent, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdAttribute> Ptr;
   typedef QList<XsdAttribute::Ptr> List;

   /**
    * @short Describes the scope of an attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#sc_a">Scope Definition</a>
    */
   class Scope : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<Scope> Ptr;

      /**
       * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#ad-scope">scope</a> of an attribute.
       */
      enum Variety {
         Global,    ///< The attribute is defined globally as child of the <em>schema</em> object.
         Local      ///< The attribute is defined locally as child of a complex type or attribute group definition.
      };

      /**
       * Sets the @p variety of the attribute scope.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#sc_a-variety">Variety Definition</a>
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the attribute scope.
       */
      Variety variety() const;

      /**
       * Sets the @p parent complex type or attribute group definition of the attribute scope.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#sc_a-parent">Parent Definition</a>
       */
      void setParent(const NamedSchemaComponent::Ptr &parent);

      /**
       * Returns the parent complex type or attribute group definition of the attribute scope.
       */
      NamedSchemaComponent::Ptr parent() const;

    private:
      Variety                   m_variety;
      NamedSchemaComponent      *m_parent;
   };


   /**
    * @short Describes the value constraint of an attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_a">Value Constraint Definition</a>
    */
   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      /**
       * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#ad-value_constraint">value constraint</a> of an attribute.
       */
      enum Variety {
         Default,  ///< The attribute has a default value set.
         Fixed     ///< The attribute has a fixed value set.
      };

      /**
       * Sets the @p variety of the attribute value constraint.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_a-variety">Variety Definition</a>
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the attribute value constraint.
       */
      Variety variety() const;

      /**
       * Sets the @p value of the constraint.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_a-value">Value Definition</a>
       */
      void setValue(const QString &value);

      /**
       * Returns the value of the constraint.
       */
      QString value() const;

      /**
       * Sets the lexical @p form of the constraint.
       *
       * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_a-lexical_form">Lexical Form Definition</a>
       */
      void setLexicalForm(const QString &form);

      /**
       * Returns the lexical form of the constraint.
       */
      QString lexicalForm() const;

    private:
      Variety m_variety;
      QString m_value;
      QString m_lexicalForm;
   };

   /**
    * Sets the simple @p type definition of the attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ad-type_definition">Simple Type Definition</a>
    */
   void setType(const AnySimpleType::Ptr &type);

   /**
    * Returns the simple type definition of the attribute.
    */
   AnySimpleType::Ptr type() const;

   /**
    * Sets the @p scope of the attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ad-scope">Scope Definition</a>
    */
   void setScope(const Scope::Ptr &scope);

   /**
    * Returns the scope of the attribute.
    */
   Scope::Ptr scope() const;

   /**
    * Sets the value @p constraint of the attribute.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ad-value_constraint">Value Constraint Definition</a>
    */
   void setValueConstraint(const ValueConstraint::Ptr &constraint);

   /**
    * Returns the value constraint of the attribute.
    */
   ValueConstraint::Ptr valueConstraint() const;

 private:
   AnySimpleType::Ptr   m_type;
   Scope::Ptr           m_scope;
   ValueConstraint::Ptr m_valueConstraint;
};

}


#endif
