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

   class Scope : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<Scope> Ptr;

      enum Variety {
         Global,       // defined globally as child of the <em>schema</em> object.
         Local         // defined locally as child of a complex type or attribute group definition.
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setParent(const NamedSchemaComponent::Ptr &parent);
      NamedSchemaComponent::Ptr parent() const;

    private:
      Variety                   m_variety;
      NamedSchemaComponent      *m_parent;
   };


   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      enum Variety {
         Default,     // attribute has a default value set.
         Fixed        // attribute has a fixed value set.
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

   void setType(const AnySimpleType::Ptr &type);
   AnySimpleType::Ptr type() const;
   void setScope(const Scope::Ptr &scope);

   Scope::Ptr scope() const;

   void setValueConstraint(const ValueConstraint::Ptr &constraint);

   ValueConstraint::Ptr valueConstraint() const;

 private:
   AnySimpleType::Ptr   m_type;
   Scope::Ptr           m_scope;
   ValueConstraint::Ptr m_valueConstraint;
};

}


#endif
