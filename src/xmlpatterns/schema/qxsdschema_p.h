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

#ifndef QXsdSchema_P_H
#define QXsdSchema_P_H

#include <QHash>
#include <QReadWriteLock>

#include <qschematype_p.h>
#include <qxsdannotated_p.h>
#include <qxsdattribute_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdcomplextype_p.h>
#include <qxsdelement_p.h>
#include <qxsdidentityconstraint_p.h>
#include <qxsdmodelgroup_p.h>
#include <qxsdnotation_p.h>
#include <qxsdsimpletype_p.h>

namespace QPatternist {

class XsdSchema : public QSharedData, public XsdAnnotated
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSchema> Ptr;
   typedef QList<XsdSchema::Ptr> List;

   XsdSchema(const NamePool::Ptr &namePool);

   ~XsdSchema();

   NamePool::Ptr namePool() const;

   void setTargetNamespace(const QString &targetNamespace);
   QString targetNamespace() const;

   void addElement(const XsdElement::Ptr &element);
   XsdElement::Ptr element(const QXmlName &name) const;

   XsdElement::List elements() const;

   void addAttribute(const XsdAttribute::Ptr &attribute);
   XsdAttribute::Ptr attribute(const QXmlName &name) const;

   XsdAttribute::List attributes() const;

   void addType(const SchemaType::Ptr &type);

   SchemaType::Ptr type(const QXmlName &name) const;
   SchemaType::List types() const;

   XsdSimpleType::List simpleTypes() const;
   XsdComplexType::List complexTypes() const;

   void addAnonymousType(const SchemaType::Ptr &type);
   SchemaType::List anonymousTypes() const;

   void addAttributeGroup(const XsdAttributeGroup::Ptr &group);
   XsdAttributeGroup::Ptr attributeGroup(const QXmlName name) const;
   XsdAttributeGroup::List attributeGroups() const;

   void addElementGroup(const XsdModelGroup::Ptr &group);
   XsdModelGroup::Ptr elementGroup(const QXmlName &name) const;

   XsdModelGroup::List elementGroups() const;

   void addNotation(const XsdNotation::Ptr &notation);

   XsdNotation::Ptr notation(const QXmlName &name) const;
   XsdNotation::List notations() const;

   void addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint);
   XsdIdentityConstraint::Ptr identityConstraint(const QXmlName &name) const;
   XsdIdentityConstraint::List identityConstraints() const;

 private:
   NamePool::Ptr                               m_namePool;
   QString                                     m_targetNamespace;
   QHash<QXmlName, XsdElement::Ptr>            m_elements;
   QHash<QXmlName, XsdAttribute::Ptr>          m_attributes;
   QHash<QXmlName, SchemaType::Ptr>            m_types;
   QHash<QXmlName, SchemaType::Ptr>            m_anonymousTypes;
   QHash<QXmlName, XsdAttributeGroup::Ptr>     m_attributeGroups;
   QHash<QXmlName, XsdModelGroup::Ptr>         m_elementGroups;
   QHash<QXmlName, XsdNotation::Ptr>           m_notations;
   QHash<QXmlName, XsdIdentityConstraint::Ptr> m_identityConstraints;
   mutable QReadWriteLock                      m_lock;
};

}

#endif
