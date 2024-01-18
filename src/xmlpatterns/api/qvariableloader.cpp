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

#include <qvariant.h>
#include <qstringlist.h>

#include "qanyuri_p.h"
#include "qatomicstring_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qinteger_p.h"
#include "qitem_p.h"
#include "qsequencetype_p.h"
#include "qvariableloader_p.h"
#include "qxmlquery_p.h"

namespace QPatternist {

class VariantListIterator : public ListIteratorPlatform<QVariant, Item, VariantListIterator>
{
 public:
   inline VariantListIterator(const QVariantList &list) : ListIteratorPlatform<QVariant, Item, VariantListIterator>(list) {
   }

 private:
   friend class ListIteratorPlatform<QVariant, Item, VariantListIterator>;

   inline Item inputToOutputItem(const QVariant &inputType) const {
      return AtomicValue::toXDM(inputType);
   }
};

class StringListIterator : public ListIteratorPlatform<QString, Item, StringListIterator>
{
 public:
   inline StringListIterator(const QStringList &list) : ListIteratorPlatform<QString, Item, StringListIterator>(list) {
   }

 private:
   friend class ListIteratorPlatform<QString, Item, StringListIterator>;

   static inline Item inputToOutputItem(const QString &inputType) {
      return AtomicString::fromValue(inputType);
   }
};

class TemporaryTreesRedirectingContext : public DelegatingDynamicContext
{
 public:
   TemporaryTreesRedirectingContext(const DynamicContext::Ptr &other, const DynamicContext::Ptr &modelStorage)
                  : DelegatingDynamicContext(other), m_modelStorage(modelStorage) {
      Q_ASSERT(m_modelStorage);
   }

   void addNodeModel(const QAbstractXmlNodeModel::Ptr &nodeModel) override {
      m_modelStorage->addNodeModel(nodeModel);
   }

 private:
   const DynamicContext::Ptr m_modelStorage;
};

}  // namespace

using namespace QPatternist;

SequenceType::Ptr VariableLoader::announceExternalVariable(const QXmlName name,
      const SequenceType::Ptr &declaredType)
{
   (void) declaredType;

   const QVariant &variant = m_bindingHash.value(name);

   if (! variant.isValid()) {
      return SequenceType::Ptr();

   } else if (variant.userType() == QVariant::typeToTypeId<QIODevice *>()) {
      return CommonSequenceTypes::ExactlyOneAnyURI;

   } else if (variant.userType() == QVariant::typeToTypeId<QXmlQuery>()) {
      const QXmlQuery variableQuery(variant.value<QXmlQuery>());
      return variableQuery.d->expression()->staticType();

   } else {
      return makeGenericSequenceType(AtomicValue::qtToXDMType(
               variant.value<QXmlItem>()), Cardinality::exactlyOne());
   }
}

Item::Iterator::Ptr VariableLoader::evaluateSequence(const QXmlName name,
      const DynamicContext::Ptr &context)
{

   const QVariant &variant = m_bindingHash.value(name);
   Q_ASSERT_X(variant.isValid(), Q_FUNC_INFO, "There was no binding.");

   /* Same code as in the default clause below. */
   if (variant.userType() == QVariant::typeToTypeId<QIODevice *>()) {
      return makeSingletonIterator(itemForName(name));

   } else if (variant.userType() == QVariant::typeToTypeId<QXmlQuery>()) {
      const QXmlQuery variableQuery(variant.value<QXmlQuery>());

      return variableQuery.d->expression()->evaluateSequence(DynamicContext::Ptr(new TemporaryTreesRedirectingContext(
                variableQuery.d->dynamicContext(), context)));
   }

   const QVariant v(variant.value<QXmlItem>().toAtomicValue());

   switch (v.type()) {
      case QVariant::StringList:
         return Item::Iterator::Ptr(new StringListIterator(v.toStringList()));

      case QVariant::List:
         return Item::Iterator::Ptr(new VariantListIterator(v.toList()));

      default:
         return makeSingletonIterator(itemForName(name));
   }
}

Item VariableLoader::itemForName(const QXmlName &name) const
{
   const QVariant &variant = m_bindingHash.value(name);

   if (variant.userType() == QVariant::typeToTypeId<QIODevice *>()) {

      return Item(AnyURI::fromValue("tag:copperspice.com,2007:QtXmlPatterns:QIODeviceVariable:" +
                  m_namePool->stringForLocalName(name.localName()) ));
   }

   const QXmlItem item(variant.value<QXmlItem>());

   if (item.isNode()) {
      return Item::fromPublic(item);

   } else {
      const QVariant atomicValue(item.toAtomicValue());

      /* If the atomicValue is null it means it doesn't exist in m_bindingHash, and therefore it must
       * be a QIODevice, since Patternist guarantees to only ask for variables that announceExternalVariable()
       * has accepted. */

      if (! atomicValue.isValid()) {
         return Item(AnyURI::fromValue("tag:copperspice.com,2007:QtXmlPatterns:QIODeviceVariable:" +
                                       m_namePool->stringForLocalName(name.localName())));
      } else {
         return AtomicValue::toXDM(atomicValue);
      }
   }
}

Item VariableLoader::evaluateSingleton(const QXmlName name, const DynamicContext::Ptr &)
{
   return itemForName(name);
}

bool VariableLoader::isSameType(const QVariant &v1, const QVariant &v2) const
{
   /* Are both of type QIODevice *? */

   if (v1.userType() == QVariant::typeToTypeId<QIODevice *>() && v1.userType() == v2.userType()) {
      return true;
   }

   /* Ok, we have two QXmlItems. */
   const QXmlItem i1(v1.value<QXmlItem>());
   const QXmlItem i2(v2.value<QXmlItem>());

   if (i1.isNode()) {
      Q_ASSERT(false);
      return false;

   } else if (i2.isAtomicValue()) {
      return i1.toAtomicValue().type() == i2.toAtomicValue().type();

   } else {
      /* One is an atomic, the other is a node or they are null. */
      return false;
   }
}

void VariableLoader::removeBinding(const QXmlName &name)
{
   m_bindingHash.remove(name);
}

bool VariableLoader::hasBinding(const QXmlName &name) const
{
   return m_bindingHash.contains(name) || (m_previousLoader && m_previousLoader->hasBinding(name));
}

QVariant VariableLoader::valueFor(const QXmlName &name) const
{
   if (m_bindingHash.contains(name)) {
      return m_bindingHash.value(name);
   } else if (m_previousLoader) {
      return m_previousLoader->valueFor(name);
   } else {
      return QVariant();
   }
}

void VariableLoader::addBinding(const QXmlName &name, const QVariant &value)
{
   m_bindingHash.insert(name, value);
}

bool VariableLoader::invalidationRequired(const QXmlName &name, const QVariant &variant) const
{
   return hasBinding(name) && !isSameType(valueFor(name), variant);
}

