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

#ifndef QXsdValidatingInstanceReader_P_H
#define QXsdValidatingInstanceReader_P_H

#include <qxsdidchelper_p.h>
#include <qxsdinstancereader_p.h>
#include <qxsdstatemachine_p.h>
#include <qxsdvalidatedxmlnodemodel_p.h>
#include <QStack>

class QXmlQuery;

namespace QPatternist {
class XsdValidatingInstanceReader : public XsdInstanceReader
{
 public:
   typedef QExplicitlySharedDataPointer<XsdValidatingInstanceReader> Ptr;

   XsdValidatingInstanceReader(XsdValidatedXmlNodeModel *model, const QUrl &documentUri,
                               const XsdSchemaContext::Ptr &context);

   void addSchema(const XsdSchema::Ptr &schema, const QUrl &url);
   bool read();

 private:
   bool loadSchema(const QString &targetNamespace, const QUrl &location);
   void error(const QString &msg) const;

   bool validate(bool &hasStateMachine, XsdElement::Ptr &element);

   bool validateElement(const XsdElement::Ptr &declaration, bool &hasStateMachine);
   bool validateElementType(const XsdElement::Ptr &declaration, const SchemaType::Ptr &type, bool isNilled,
                            bool &hasStateMachine);

   bool validateElementSimpleType(const XsdElement::Ptr &declaration, const SchemaType::Ptr &type, bool isNilled);

   bool validateElementComplexType(const XsdElement::Ptr &declaration, const SchemaType::Ptr &type, bool isNilled,
                                   bool &hasStateMachine);

   bool validateAttribute(const XsdAttributeUse::Ptr &declaration, const QString &value);

   bool validateAttribute(const XsdAttribute::Ptr &declaration, const QString &value);

   bool validateAttributeWildcard(const QXmlName &attributeName, const XsdWildcard::Ptr &wildcard);

   bool validateIdentityConstraint(const XsdElement::Ptr &element, const QXmlItem &currentItem);

   bool validateUniqueIdentityConstraint(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &constraint,
                                         const TargetNode::Set &qualifiedNodeSet);

   bool validateKeyIdentityConstraint(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &constraint,
                                      const TargetNode::Set &targetNodeSet, const TargetNode::Set &qualifiedNodeSet);

   bool validateKeyRefIdentityConstraint(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &constraint,
                                         const TargetNode::Set &qualifiedNodeSet);

   bool selectNodeSets(const XsdElement::Ptr &element, const QXmlItem &currentItem,
                       const XsdIdentityConstraint::Ptr &constraint, TargetNode::Set &targetNodeSet, TargetNode::Set &qualifiedNodeSet);

   QXmlQuery createXQuery(const QList<QXmlName> &namespaceBindings, const QXmlItem &contextNode,
                          const QString &query) const;

   XsdElement::Ptr elementByName(const QXmlName &name) const;
   XsdAttribute::Ptr attributeByName(const QXmlName &name) const;

   SchemaType::Ptr typeByName(const QXmlName &name) const;

   void addIdIdRefBinding(const QString &id, const NamedSchemaComponent::Ptr &binding);

   QString qNameAttribute(const QXmlName &attributeName);

   XsdComplexType::Ptr anyType();

   void createAndPushStateMachine(const XsdParticle::Ptr &particle);

   typedef QHash<QUrl, QStringList> MergedSchemas;
   typedef QHashIterator<QUrl, QStringList> MergedSchemasIterator;

   XsdValidatedXmlNodeModel::Ptr               m_model;
   MergedSchemas                               m_mergedSchemas;
   XsdSchema::Ptr                              m_schema;
   const NamePool::Ptr                         m_namePool;
   const QXmlName                              m_xsiNilName;
   const QXmlName                              m_xsiTypeName;
   const QXmlName                              m_xsiSchemaLocationName;
   const QXmlName                              m_xsiNoNamespaceSchemaLocationName;

   QStack<XsdStateMachine<XsdTerm::Ptr> >      m_stateMachines;
   QUrl                                        m_documentUri;
   XsdComplexType::Ptr                         m_anyType;
   QSet<QString>                               m_processedNamespaces;
   QSet<QString>                               m_processedSchemaLocations;
   QSet<QString>                               m_idRefs;
   QHash<QXmlName, TargetNode::Set>            m_idcKeys;
   SchemaType::Ptr                             m_idRefsType;
};

}

#endif
