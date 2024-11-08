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

#ifndef QXsdValidatedXmlNodeModel_P_H
#define QXsdValidatedXmlNodeModel_P_H

#include <qabstractxmlnodemodel.h>
#include <qabstractxmlforwarditerator_p.h>
#include <qitem_p.h>
#include <qschematype_p.h>
#include <qxsdelement_p.h>

namespace QPatternist {

class XsdValidatedXmlNodeModel : public QAbstractXmlNodeModel
{
 public:
   typedef QExplicitlySharedDataPointer<XsdValidatedXmlNodeModel> Ptr;
   typedef QList<Ptr> List;

   XsdValidatedXmlNodeModel(const QAbstractXmlNodeModel *model);

   virtual ~XsdValidatedXmlNodeModel();

   QUrl baseUri(const QXmlNodeModelIndex &ni) const override;
   QUrl documentUri(const QXmlNodeModelIndex &ni) const override;
   QXmlNodeModelIndex::NodeKind kind(const QXmlNodeModelIndex &ni) const override;

   QXmlNodeModelIndex::DocumentOrder compareOrder(const QXmlNodeModelIndex &ni1,
                  const QXmlNodeModelIndex &ni2) const override;

   QXmlNodeModelIndex root(const QXmlNodeModelIndex &n) const override;
   QXmlName name(const QXmlNodeModelIndex &ni) const override;
   QString stringValue(const QXmlNodeModelIndex &n) const override;
   QVariant typedValue(const QXmlNodeModelIndex &n) const override;

   QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> >
         iterate(const QXmlNodeModelIndex &ni, QXmlNodeModelIndex::Axis axis) const override;

   QPatternist::ItemIteratorPtr sequencedTypedValue(const QXmlNodeModelIndex &ni) const override;
   QPatternist::ItemType::Ptr type(const QXmlNodeModelIndex &ni) const override;

   QXmlName::NamespaceCode namespaceForPrefix(const QXmlNodeModelIndex &ni, const QXmlName::PrefixCode prefix) const override;

   bool isDeepEqual(const QXmlNodeModelIndex &ni1, const QXmlNodeModelIndex &ni2) const override;
   void sendNamespaces(const QXmlNodeModelIndex &n, QAbstractXmlReceiver *const receiver) const override;
   QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &n) const override;
   QXmlNodeModelIndex elementById(const QXmlName &NCName) const override;
   QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &NCName) const override;

   void copyNodeTo(const QXmlNodeModelIndex &node, QAbstractXmlReceiver *const receiver, const NodeCopySettings &) const override;

   void setAssignedElement(const QXmlNodeModelIndex &index, const XsdElement::Ptr &element);

   XsdElement::Ptr assignedElement(const QXmlNodeModelIndex &index) const;
   void setAssignedAttribute(const QXmlNodeModelIndex &index, const XsdAttribute::Ptr &attribute);

   XsdAttribute::Ptr assignedAttribute(const QXmlNodeModelIndex &index) const;
   void setAssignedType(const QXmlNodeModelIndex &index, const SchemaType::Ptr &type);

   SchemaType::Ptr assignedType(const QXmlNodeModelIndex &index) const;

   void addIdIdRefBinding(const QString &id, const NamedSchemaComponent::Ptr &binding);

   QStringList idIdRefBindingIds() const;

   QSet<NamedSchemaComponent::Ptr> idIdRefBindings(const QString &id) const;

 protected:
   QXmlNodeModelIndex nextFromSimpleAxis(SimpleAxis axis, const QXmlNodeModelIndex &origin) const override;
   QVector<QXmlNodeModelIndex> attributes(const QXmlNodeModelIndex &element) const override;

 private:
   QExplicitlySharedDataPointer<const QAbstractXmlNodeModel> m_internalModel;
   QHash<QXmlNodeModelIndex, XsdElement::Ptr>                m_assignedElements;
   QHash<QXmlNodeModelIndex, XsdAttribute::Ptr>              m_assignedAttributes;
   QHash<QXmlNodeModelIndex, SchemaType::Ptr>                m_assignedTypes;
   QHash<QString, QSet<NamedSchemaComponent::Ptr> >          m_idIdRefBindings;
};

}

#endif
