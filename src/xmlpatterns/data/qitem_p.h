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

#ifndef QItem_P_H
#define QItem_P_H

#include <qcppcastinghelper_p.h>
#include <qitemtype_p.h>
#include <qsingletoniterator_p.h>

#include <qabstractxmlnodemodel.h>
#include <qurl.h>
#include <qvariant.h>

template<typename T>
class QList;

template<typename T>
class QVector;

template<typename T>
class QAbstractXmlForwardIterator;

class QSourceLocation;
class QAbstractXmlReceiver;

namespace QPatternist {

class DynamicContext;
class Item;
class ItemType;
class QObjectNodeModel;

template<typename T>
class EmptyIterator;

template<typename T, typename ListType>
class ListIterator;

#include <variant>

class AtomicValue : public QSharedData, public CppCastingHelper<AtomicValue>
{
 public:
   virtual ~AtomicValue();

   typedef QExplicitlySharedDataPointer<AtomicValue> Ptr;
   typedef QList<AtomicValue::Ptr> List;

   virtual bool hasError() const;

   virtual bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &context) const;

   virtual QString stringValue() const = 0;
   virtual ItemType::Ptr type() const = 0;

   static QVariant toQt(const AtomicValue *const value);

   static inline QVariant toQt(const AtomicValue::Ptr &value) {
      return toQt(value.data());
   }

   static Item toXDM(const QVariant &value);

   static ItemType::Ptr qtToXDMType(const QXmlItem &item);

 protected:
   inline AtomicValue() {
   }
};

class Item
{
 public:
   typedef QAbstractXmlForwardIterator<Item> Iterator;
   typedef QList<Item> List;
   typedef QVector<Item> Vector;
   typedef QPatternist::SingletonIterator<Item> SingletonIterator;
   typedef QPatternist::EmptyIterator<Item> EmptyIterator;

   inline Item()
   {
   }

   inline Item(const QXmlNodeModelIndex &tmp)
      : m_data(tmp.m_storage)
   {
   }

   inline Item(const Item &other)
      : m_data(other.m_data)
   {
      if (isAtomicValue()) {
         std::get<const AtomicValue *>(m_data)->ref.ref();
      }
   }

   inline Item(const AtomicValue::Ptr &a)
   {
      if (a) {
         m_data = a.data();
         std::get<const AtomicValue *>(m_data)->ref.ref();
      }
   }

   inline Item(const AtomicValue *a)
   {
      if (a) {
         m_data = a;
         std::get<const AtomicValue *>(m_data)->ref.ref();
      }
   }

   inline ~Item() {
      if (isAtomicValue() && ! std::get<const AtomicValue *>(m_data)->ref.deref()) {
         delete std::get<const AtomicValue *>(m_data);
      }
   }

   inline Item &operator=(const Item &other) {

      if (other.isAtomicValue()) {
         std::get<const AtomicValue *>(other.m_data)->ref.ref();
      }

      if (isAtomicValue()) {
         if (! std::get<const AtomicValue *>(m_data)->ref.deref()) {
            delete std::get<const AtomicValue *>(m_data);
         }
      }

      m_data = other.m_data;

      return *this;
   }

   template<typename TCastTarget>
   inline TCastTarget *as() const {
      return const_cast<TCastTarget *>(static_cast<const TCastTarget *>(std::get<const AtomicValue *>(m_data)));
   }

   inline QString stringValue() const {
      if (isAtomicValue()) {
         return std::get<const AtomicValue *>(m_data)->stringValue();
      } else {
         return asNode().stringValue();
      }
   }

   Item::Iterator::Ptr sequencedTypedValue() const;

   inline bool isAtomicValue() const {
      return std::holds_alternative<const AtomicValue *>(m_data);
   }

   inline bool isNode() const {
      return std::holds_alternative<NodeIndexStorage>(m_data);
   }

   inline QExplicitlySharedDataPointer<ItemType> type() const {
      if (isAtomicValue()) {
         return std::get<const AtomicValue *>(m_data)->type();
      } else {
         return asNode().type();
      }
   }

   inline const AtomicValue *asAtomicValue() const {
      Q_ASSERT(isAtomicValue());
      return std::get<const AtomicValue *>(m_data);
   }

   inline QXmlNodeModelIndex asNode() const {
      Q_ASSERT_X(isNode() || isNull(), Q_FUNC_INFO, "This item is not a valid QXmlNodeModelIndex.");
      Q_ASSERT_X(sizeof(QXmlNodeModelIndex) == sizeof(QPatternist::NodeIndexStorage), Q_FUNC_INFO, "Size mismatch");

      QXmlNodeModelIndex retval;

      if (isNode()) {
         retval.m_storage = std::get<NodeIndexStorage>(m_data);
      }

      return retval;
   }

   inline operator bool() const {
      return ! isNull();
   }

   inline bool isNull() const {
      return std::holds_alternative<std::monostate>(m_data);
   }

   inline void reset() {
      // Delete the atomicValue if necessary

      if (isAtomicValue() && ! std::get<const AtomicValue *>(m_data)->ref.deref()) {
         delete std::get<const AtomicValue *>(m_data);
      }

      m_data = std::monostate();
   }

   static inline Item fromPublic(const QXmlItem &tmp) {
      const Item it(tmp.m_node);

      if (it.isAtomicValue()) {
         it.asAtomicValue()->ref.ref();
      }

      return it;
   }

   static inline QXmlItem toPublic(const Item &tmp) {
      return QXmlItem(tmp);
   }

 private:
   std::variant<std::monostate, NodeIndexStorage, const AtomicValue *> m_data;

   friend class ::QXmlItem;
};

template<typename T>
inline Item toItem(const QExplicitlySharedDataPointer<T> atomicValue)
{
   return Item(atomicValue.data());
}

static inline QString formatData(const QXmlNodeModelIndex node)
{
   return node.stringValue(); // This can be improved a lot.
}
}

inline QXmlName QXmlNodeModelIndex::name() const
{
   return m_storage.model->name(*this);
}

inline QXmlNodeModelIndex QXmlNodeModelIndex::root() const
{
   return m_storage.model->root(*this);
}

inline auto QXmlNodeModelIndex::iterate(const QXmlNodeModelIndex::Axis axis) const -> Iterator::Ptr
{
   return m_storage.model->iterate(*this, axis);
}

inline QUrl QXmlNodeModelIndex::documentUri() const
{
   return m_storage.model->documentUri(*this);
}

inline QUrl QXmlNodeModelIndex::baseUri() const
{
   return m_storage.model->baseUri(*this);
}

inline QXmlNodeModelIndex::NodeKind QXmlNodeModelIndex::kind() const
{
   return m_storage.model->kind(*this);
}

inline bool QXmlNodeModelIndex::isDeepEqual(const QXmlNodeModelIndex &other) const
{
   return m_storage.model->isDeepEqual(*this, other);
}

inline QXmlNodeModelIndex::DocumentOrder QXmlNodeModelIndex::compareOrder(const QXmlNodeModelIndex &other) const
{
   Q_ASSERT_X(model() == other.model(), Q_FUNC_INFO, "The API docs guarantees the two nodes are from the same model");
   return m_storage.model->compareOrder(*this, other);
}

inline bool QXmlNodeModelIndex::is(const QXmlNodeModelIndex &other) const
{
   return m_storage.model == other.m_storage.model &&
          m_storage.data == other.m_storage.data &&
          m_storage.additionalData == other.m_storage.additionalData;
}

inline void QXmlNodeModelIndex::sendNamespaces(QAbstractXmlReceiver *const receiver) const
{
   m_storage.model->sendNamespaces(*this, receiver);
}

inline QVector<QXmlName> QXmlNodeModelIndex::namespaceBindings() const
{
   return m_storage.model->namespaceBindings(*this);
}

inline QXmlName::NamespaceCode QXmlNodeModelIndex::namespaceForPrefix(const QXmlName::PrefixCode prefix) const
{
   return m_storage.model->namespaceForPrefix(*this, prefix);
}

inline QString QXmlNodeModelIndex::stringValue() const
{
   return m_storage.model->stringValue(*this);
}

inline QExplicitlySharedDataPointer<QPatternist::ItemType> QXmlNodeModelIndex::type() const
{
   return m_storage.model->type(*this);
}

inline QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QPatternist::Item> >
QXmlNodeModelIndex::sequencedTypedValue() const
{
   return m_storage.model->sequencedTypedValue(*this);
}

inline QXmlItem::QXmlItem(const QPatternist::Item &i)
{
   if (i.isNode()) {
      m_node = std::get<QPatternist::NodeIndexStorage>(i.m_data);

   } else if (i.isAtomicValue()) {
      m_node.model  = reinterpret_cast<const QAbstractXmlNodeModel *>(~0);
      m_atomicValue = std::get<const QPatternist::AtomicValue *>(i.m_data);

   } else {
      m_node.reset();
   }

   if (isAtomicValue()) {
      m_atomicValue->ref.ref();
   }
}

#endif
