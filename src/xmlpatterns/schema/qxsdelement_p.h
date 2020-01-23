/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QXsdElement_P_H
#define QXsdElement_P_H

#include <qschemacomponent_p.h>
#include <qschematype_p.h>
#include <qxsdalternative_p.h>
#include <qxsdidentityconstraint_p.h>
#include <qxsdcomplextype_p.h>
#include <QtCore/QList>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class XsdElement : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdElement> Ptr;
   typedef QList<XsdElement::Ptr> List;
   typedef QList<XsdElement *> WeakList;

   /**
    * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#ed-value_constraint">constraint type</a> of the element.
    */
   enum ConstraintType {
      NoneConstraint,     ///< The value of the element has no constraints.
      DefaultConstraint,  ///< The element has a default value set.
      FixedConstraint     ///< The element has a fixed value set.
   };

   /**
    * Describes the scope of an element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#sc_e">Scope Definition</a>
    */
   class Scope : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<Scope> Ptr;

      /**
       * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#ad-scope">scope</a> of an attribute.
       */
      enum Variety {
         Global,    ///< The element is defined globally as child of the <em>schema</em> object.
         Local      ///< The element is defined locally as child of a complex type or model group definition.
      };

      /**
       * Sets the @p variety of the element scope.
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the element scope.
       */
      Variety variety() const;

      /**
       * Sets the @p parent complex type or model group definition of the element scope.
       */
      void setParent(const NamedSchemaComponent::Ptr &parent);

      /**
       * Returns the parent complex type or model group definition of the element scope.
       */
      NamedSchemaComponent::Ptr parent() const;

    private:
      Variety                   m_variety;
      NamedSchemaComponent      *m_parent;
   };

   /**
    * Describes a type table of an element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#tt">Type Table Definition</a>
    */
   class TypeTable : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<TypeTable> Ptr;

      /**
       * Adds an @p alternative to the type table.
       */
      void addAlternative(const XsdAlternative::Ptr &alternative);

      /**
       * Returns the alternatives of the type table.
       */
      XsdAlternative::List alternatives() const;

      /**
       * Sets the default @p type definition.
       */
      void setDefaultTypeDefinition(const XsdAlternative::Ptr &type);

      /**
       * Returns the default type definition.
       */
      XsdAlternative::Ptr defaultTypeDefinition() const;

    private:
      XsdAlternative::List m_alternatives;
      XsdAlternative::Ptr  m_defaultTypeDefinition;
   };


   /**
    * Describes the value constraint of an element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_e">Value Constraint Definition</a>
    */
   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      /**
       * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#ed-value_constraint">value constraint</a> of an element.
       */
      enum Variety {
         Default,  ///< The element has a default value set.
         Fixed     ///< The element has a fixed value set.
      };

      /**
       * Sets the @p variety of the element value constraint.
       */
      void setVariety(Variety variety);

      /**
       * Returns the variety of the element value constraint.
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

    private:
      Variety m_variety;
      QString m_value;
      QString m_lexicalForm;
   };

   /**
    * Creates a new element object.
    */
   XsdElement();

   /**
    * Always returns @c true, used to avoid dynamic casts.
    */
   bool isElement() const override;

   /**
    * Sets the @p type of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-type_definition">Type Definition</a>
    */
   void setType(const SchemaType::Ptr &type);

   /**
    * Returns the type of the element.
    */
   SchemaType::Ptr type() const;

   /**
    * Sets the @p scope of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-scope">Scope Definition</a>
    */
   void setScope(const Scope::Ptr &scope);

   /**
    * Returns the scope of the element.
    */
   Scope::Ptr scope() const;

   /**
    * Sets the value @p constraint of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-value_constraint">Value Constraint Definition</a>
    */
   void setValueConstraint(const ValueConstraint::Ptr &constraint);

   /**
    * Returns the value constraint of the element.
    */
   ValueConstraint::Ptr valueConstraint() const;

   /**
    * Sets the type table of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-type_table">Type Table Definition</a>
    */
   void setTypeTable(const TypeTable::Ptr &table);

   /**
    * Returns the type table of the element.
    */
   TypeTable::Ptr typeTable() const;

   /**
    * Sets whether the element is @p abstract.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-abstract">Abstract Definition</a>
    */
   void setIsAbstract(bool abstract);

   /**
    * Returns whether the element is abstract.
    */
   bool isAbstract() const;

   /**
    * Sets whether the element is @p nillable.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-nillable">Nillable Definition</a>
    */
   void setIsNillable(bool nillable);

   /**
    * Returns whether the element is nillable.
    */
   bool isNillable() const;

   /**
    * Sets the disallowed @p substitutions of the element.
    *
    * Only ExtensionConstraint, RestrictionConstraint and SubstitutionConstraint are allowed.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-disallowed_substitutions">Disallowed Substitutions Definition</a>
    */
   void setDisallowedSubstitutions(const BlockingConstraints &substitutions);

   /**
    * Returns the disallowed substitutions of the element.
    */
   BlockingConstraints disallowedSubstitutions() const;

   /**
    * Sets the substitution group @p exclusions of the element.
    *
    * Only SchemaType::ExtensionConstraint and SchemaType::RestrictionConstraint are allowed.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-substitution_group_exclusions">Substitution Group Exclusions Definition</a>
    */
   void setSubstitutionGroupExclusions(const SchemaType::DerivationConstraints &exclusions);

   /**
    * Returns the substitution group exclusions of the element.
    */
   SchemaType::DerivationConstraints substitutionGroupExclusions() const;

   /**
    * Sets the identity @p constraints of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-identity-constraint_definitions">Identity Constraint Definition</a>
    */
   void setIdentityConstraints(const XsdIdentityConstraint::List &constraints);

   /**
    * Adds a new identity @p constraint to the element.
    */
   void addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint);

   /**
    * Returns a list of all identity constraints of the element.
    */
   XsdIdentityConstraint::List identityConstraints() const;

   /**
    * Sets the substitution group @p affiliations of the element.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#ed-substituion_group_affiliations">Substitution Group Affiliations</a>
    */
   void setSubstitutionGroupAffiliations(const XsdElement::List &affiliations);

   /**
    * Returns the substitution group affiliations of the element.
    */
   XsdElement::List substitutionGroupAffiliations() const;

   /**
    * Adds a substitution group to the element.
    */
   void addSubstitutionGroup(const XsdElement::Ptr &elements);

   /**
    * Returns the substitution groups of the element.
    */
   XsdElement::WeakList substitutionGroups() const;

 private:
   SchemaType                        *m_type;
   Scope::Ptr                        m_scope;
   ValueConstraint::Ptr              m_valueConstraint;
   TypeTable::Ptr                    m_typeTable;
   bool                              m_isAbstract;
   bool                              m_isNillable;
   BlockingConstraints               m_disallowedSubstitutions;
   SchemaType::DerivationConstraints m_substitutionGroupExclusions;
   XsdIdentityConstraint::List       m_identityConstraints;
   XsdElement::List                  m_substitutionGroupAffiliations;
   QSet<XsdElement *>                m_substitutionGroups;
};
}

QT_END_NAMESPACE

#endif
