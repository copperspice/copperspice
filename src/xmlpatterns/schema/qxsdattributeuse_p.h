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

   class ValueConstraint : public QSharedData
   {
    public:
      typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

      enum Variety {
         Default,      // attribute use has a default value set
         Fixed         // attribute use has a fixed value set
      };

      void setVariety(Variety variety);
      Variety variety() const;

      void setValue(const QString &value);
      QString value() const;

      void setLexicalForm(const QString &form);
      QString lexicalForm() const;

      static ValueConstraint::Ptr fromAttributeValueConstraint(const XsdAttribute::ValueConstraint::Ptr &constraint);

    private:
      Variety m_variety;
      QString m_value;
      QString m_lexicalForm;
   };

   enum UseType {
      OptionalUse,        // attribute can be there but doesn't need to.
      RequiredUse,        // attribute must be there.
      ProhibitedUse       // attribute is not allowed to be there.
   };

   XsdAttributeUse();

   bool isAttributeUse() const override;

   void setUseType(UseType type);
   UseType useType() const;

   bool isRequired() const;

   void setAttribute(const XsdAttribute::Ptr &attribute);
   XsdAttribute::Ptr attribute() const;

   void setValueConstraint(const ValueConstraint::Ptr &constraint);
   ValueConstraint::Ptr valueConstraint() const;

 private:
   UseType              m_useType;
   XsdAttribute::Ptr    m_attribute;
   ValueConstraint::Ptr m_valueConstraint;
};

}

#endif
