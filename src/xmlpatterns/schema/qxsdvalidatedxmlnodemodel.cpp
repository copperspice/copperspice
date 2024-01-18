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

#include <qurl.h>
#include <qvariant.h>
#include <qvector.h>

#include <qxsdvalidatedxmlnodemodel_p.h>

using namespace QPatternist;

XsdValidatedXmlNodeModel::XsdValidatedXmlNodeModel(const QAbstractXmlNodeModel *model)
   : m_internalModel(model)
{
}

XsdValidatedXmlNodeModel::~XsdValidatedXmlNodeModel()
{
}

QUrl XsdValidatedXmlNodeModel::baseUri(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->baseUri(index);
}

QUrl XsdValidatedXmlNodeModel::documentUri(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->documentUri(index);
}

QXmlNodeModelIndex::NodeKind XsdValidatedXmlNodeModel::kind(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->kind(index);
}

QXmlNodeModelIndex::DocumentOrder XsdValidatedXmlNodeModel::compareOrder(const QXmlNodeModelIndex &index,
      const QXmlNodeModelIndex &otherIndex) const
{
   return m_internalModel->compareOrder(index, otherIndex);
}

QXmlNodeModelIndex XsdValidatedXmlNodeModel::root(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->root(index);
}

QXmlName XsdValidatedXmlNodeModel::name(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->name(index);
}

QString XsdValidatedXmlNodeModel::stringValue(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->stringValue(index);
}

QVariant XsdValidatedXmlNodeModel::typedValue(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->typedValue(index);
}

QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> > XsdValidatedXmlNodeModel::iterate(
   const QXmlNodeModelIndex &index, QXmlNodeModelIndex::Axis axis) const
{
   return m_internalModel->iterate(index, axis);
}

QPatternist::ItemIteratorPtr XsdValidatedXmlNodeModel::sequencedTypedValue(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->sequencedTypedValue(index);
}

QPatternist::ItemType::Ptr XsdValidatedXmlNodeModel::type(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->type(index);
}

QXmlName::NamespaceCode XsdValidatedXmlNodeModel::namespaceForPrefix(const QXmlNodeModelIndex &index,
      const QXmlName::PrefixCode prefix) const
{
   return m_internalModel->namespaceForPrefix(index, prefix);
}

bool XsdValidatedXmlNodeModel::isDeepEqual(const QXmlNodeModelIndex &index, const QXmlNodeModelIndex &otherIndex) const
{
   return m_internalModel->isDeepEqual(index, otherIndex);
}

void XsdValidatedXmlNodeModel::sendNamespaces(const QXmlNodeModelIndex &index,
      QAbstractXmlReceiver *const receiver) const
{
   m_internalModel->sendNamespaces(index, receiver);
}

QVector<QXmlName> XsdValidatedXmlNodeModel::namespaceBindings(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->namespaceBindings(index);
}

QXmlNodeModelIndex XsdValidatedXmlNodeModel::elementById(const QXmlName &name) const
{
   return m_internalModel->elementById(name);
}

QVector<QXmlNodeModelIndex> XsdValidatedXmlNodeModel::nodesByIdref(const QXmlName &name) const
{
   return m_internalModel->nodesByIdref(name);
}

void XsdValidatedXmlNodeModel::copyNodeTo(const QXmlNodeModelIndex &index, QAbstractXmlReceiver *const receiver,
      const NodeCopySettings &settings) const
{
   return m_internalModel->copyNodeTo(index, receiver, settings);
}

QXmlNodeModelIndex XsdValidatedXmlNodeModel::nextFromSimpleAxis(SimpleAxis axis, const QXmlNodeModelIndex &origin) const
{
   return m_internalModel->nextFromSimpleAxis(axis, origin);
}

QVector<QXmlNodeModelIndex> XsdValidatedXmlNodeModel::attributes(const QXmlNodeModelIndex &index) const
{
   return m_internalModel->attributes(index);
}

void XsdValidatedXmlNodeModel::setAssignedElement(const QXmlNodeModelIndex &index, const XsdElement::Ptr &element)
{
   m_assignedElements.insert(index, element);
}

XsdElement::Ptr XsdValidatedXmlNodeModel::assignedElement(const QXmlNodeModelIndex &index) const
{
   if (m_assignedElements.contains(index)) {
      return m_assignedElements.value(index);
   } else {
      return XsdElement::Ptr();
   }
}

void XsdValidatedXmlNodeModel::setAssignedAttribute(const QXmlNodeModelIndex &index, const XsdAttribute::Ptr &attribute)
{
   m_assignedAttributes.insert(index, attribute);
}

XsdAttribute::Ptr XsdValidatedXmlNodeModel::assignedAttribute(const QXmlNodeModelIndex &index) const
{
   if (m_assignedAttributes.contains(index)) {
      return m_assignedAttributes.value(index);
   } else {
      return XsdAttribute::Ptr();
   }
}

void XsdValidatedXmlNodeModel::setAssignedType(const QXmlNodeModelIndex &index, const SchemaType::Ptr &type)
{
   m_assignedTypes.insert(index, type);
}

SchemaType::Ptr XsdValidatedXmlNodeModel::assignedType(const QXmlNodeModelIndex &index) const
{
   if (m_assignedTypes.contains(index)) {
      return m_assignedTypes.value(index);
   } else {
      return SchemaType::Ptr();
   }
}

void XsdValidatedXmlNodeModel::addIdIdRefBinding(const QString &id, const NamedSchemaComponent::Ptr &binding)
{
   m_idIdRefBindings[id].insert(binding);
}

QStringList XsdValidatedXmlNodeModel::idIdRefBindingIds() const
{
   return m_idIdRefBindings.keys();
}

QSet<NamedSchemaComponent::Ptr> XsdValidatedXmlNodeModel::idIdRefBindings(const QString &id) const
{
   return m_idIdRefBindings.value(id);
}
