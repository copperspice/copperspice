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

#ifndef QXsdElement_P_H
#define QXsdElement_P_H

#include <qlist.h>
#include <qset.h>

#include <qschemacomponent_p.h>
#include <qschematype_p.h>
#include <qxsdalternative_p.h>
#include <qxsdidentityconstraint_p.h>
#include <qxsdcomplextype_p.h>

namespace QPatternist {

class XsdElement : public XsdTerm
{
 public:
   typedef QExplicitlySharedDataPointer<XsdElement> Ptr;
   typedef QList<XsdElement::Ptr> List;
   typedef QList<XsdElement *> WeakList;

   enum ConstraintType {
      NoneConstraint,     // value of the element has no constraints.
      DefaultConstraint,  // element has a default value set.
      FixedConstraint     // element has a fixed value set.
   };

   class Scope : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<Scope> Ptr;

      enum Variety {
         Global,    // element is defined globally as child of the <em>schema</em> object.
         Local      // element is defined locally as child of a complex type or model group definition.
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setParent(const NamedSchemaComponent::Ptr &parent);
      NamedSchemaComponent::Ptr parent() const;

    private:
      Variety m_variety;
      NamedSchemaComponent *m_parent;
   };

   class TypeTable : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<TypeTable> Ptr;

      void addAlternative(const XsdAlternative::Ptr &alternative);
      XsdAlternative::List alternatives() const;

      void setDefaultTypeDefinition(const XsdAlternative::Ptr &type);
      XsdAlternative::Ptr defaultTypeDefinition() const;

    private:
      XsdAlternative::List m_alternatives;
      XsdAlternative::Ptr  m_defaultTypeDefinition;
   };

   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      enum Variety {
         Default,     // element has a default value set.
         Fixed        // element has a fixed value set.
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setValue(const QString &value);
      QString value() const;

      void setLexicalForm(const QString &form);
      QString lexicalForm() const;

    private:
      Variety m_variety;
      QString m_value;
      QString m_lexicalForm;
   };

   XsdElement();

   bool isElement() const override;

   void setType(const SchemaType::Ptr &type);
   SchemaType::Ptr type() const;

   void setScope(const Scope::Ptr &scope);
   Scope::Ptr scope() const;

   void setValueConstraint(const ValueConstraint::Ptr &constraint);
   ValueConstraint::Ptr valueConstraint() const;

   void setTypeTable(const TypeTable::Ptr &table);
   TypeTable::Ptr typeTable() const;

   void setIsAbstract(bool abstract);
   bool isAbstract() const;

   void setIsNillable(bool nillable);
   bool isNillable() const;

   void setDisallowedSubstitutions(const BlockingConstraints &substitutions);
   BlockingConstraints disallowedSubstitutions() const;

   void setSubstitutionGroupExclusions(const SchemaType::DerivationConstraints &exclusions);
   SchemaType::DerivationConstraints substitutionGroupExclusions() const;

   void setIdentityConstraints(const XsdIdentityConstraint::List &constraints);
   void addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint);

   XsdIdentityConstraint::List identityConstraints() const;
   void setSubstitutionGroupAffiliations(const XsdElement::List &affiliations);

   XsdElement::List substitutionGroupAffiliations() const;
   void addSubstitutionGroup(const XsdElement::Ptr &elements);
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

#endif
