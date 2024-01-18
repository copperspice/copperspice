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

#ifndef QABSTRACTXMLNODEMODEL_H
#define QABSTRACTXMLNODEMODEL_H

#include <qxmlname.h>
#include <qshareddata.h>
#include <qscopedpointer.h>

class QAbstractXmlNodeModel;
class QAbstractXmlNodeModelPrivate;
class QAbstractXmlReceiver;
class QSourceLocation;
class QUrl;
class QXmlName;
class QXmlNodeModelIndex;

template<typename T>
class QAbstractXmlForwardIterator;

template<typename T>
class QVector;

/* The members in the namespace QPatternist are internal, not part of the public API, and
 * unsupported. Using them leads to undefined behavior. */
namespace QPatternist {

class DynamicContext;
class Item;
class ItemType;
class XsdValidatedXmlNodeModel;

template<typename TResult, typename TSource, typename TMapper, typename Context>
class ItemMappingIterator;

template<typename TResult, typename TSource, typename TMapper>
class SequenceMappingIterator;

typedef QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<Item> > ItemIteratorPtr;
typedef QVector<QXmlName> QXmlNameVector;

class NodeIndexStorage
{
 public:
   void *pointer() const {
      // Constructing to qptrdiff means we avoid the warning "cast to pointer  from integer of different size.
      return (void *)qptrdiff(data);
   }

   // Implementation is in qabstractxmlnodemodel.cpp
   inline bool operator!=(const NodeIndexStorage &other) const;

   void reset() {
      data = 0;
      additionalData = 0;
      model = nullptr;
   }

   qint64 additionalData = 0;
   const QAbstractXmlNodeModel *model = nullptr;

   // ** Changing the order of these two members, ptr and data is a binary incompatible on Mac Power PC
   // Do not use ptr directly, use pointer() instead
   union {
      void *ptr;
      qint64 data;
   };
};

}   // end namespace

class Q_XMLPATTERNS_EXPORT QXmlNodeModelIndex
{
   enum Constants {
      ForwardAxis = 8192,
      ReverseAxis = 16384
   };

 public:
   typedef QAbstractXmlForwardIterator<QXmlNodeModelIndex> Iterator;
   typedef QList<QXmlNodeModelIndex> List;

   enum NodeKind {
      Attribute               = 1,
      Comment                 = 2,
      Document                = 4,
      Element                 = 8,
      Namespace               = 16,
      ProcessingInstruction   = 32,
      Text                    = 64
   };

   enum DocumentOrder {
      Precedes = -1,
      Is       = 0,
      Follows  = 1
   };

   enum Axis {
      AxisChild               = 1 | ForwardAxis,
      AxisDescendant          = 2 | ForwardAxis,
      AxisAttribute           = 4 | ForwardAxis,
      AxisSelf                = 8 | ForwardAxis,
      AxisDescendantOrSelf    = 16 | ForwardAxis,
      AxisFollowingSibling    = 32 | ForwardAxis,
      AxisNamespace           = 64 | ForwardAxis,
      AxisFollowing           = 128 | ReverseAxis,
      AxisParent              = 256 | ReverseAxis,
      AxisAncestor            = 512 | ReverseAxis,
      AxisPrecedingSibling    = 1024 | ReverseAxis,
      AxisPreceding           = 2048 | ReverseAxis,
      AxisAncestorOrSelf      = 4096 | ReverseAxis,
      /* Note, can not clash with the values of ForwardAxis and ReverseAxis. */
      AxisChildOrTop          = 32768 | ForwardAxis,
      AxisAttributeOrTop      = 65536 | ForwardAxis
   };

   inline QXmlNodeModelIndex() {
      reset();
   }

   QXmlNodeModelIndex(const QXmlNodeModelIndex &other) = default;
   QXmlNodeModelIndex &operator=(const QXmlNodeModelIndex &other) = default;

   bool operator==(const QXmlNodeModelIndex &other) const;
   bool operator!=(const QXmlNodeModelIndex &other) const;

   inline qint64 data() const {
      return m_storage.data;
   }

   inline void *internalPointer() const {
      return m_storage.pointer();
   }

   inline const QAbstractXmlNodeModel *model() const {
      return m_storage.model;
   }

   inline qint64 additionalData() const {
      return m_storage.additionalData;
   }

   inline bool isNull() const {
      return ! m_storage.model;
   }

   // The members below are internal and not part of the public API. They are unsupported.
   // using them in your application can lead to undefined behavior.

   inline QXmlName name() const;
   inline QXmlNodeModelIndex root() const;

   inline QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex>> iterate(const Axis axis) const;
   inline QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QPatternist::Item> > sequencedTypedValue() const;

   inline QUrl documentUri() const;
   inline QUrl baseUri() const;
   inline NodeKind kind() const;
   inline bool isDeepEqual(const QXmlNodeModelIndex &other) const;
   inline DocumentOrder compareOrder(const QXmlNodeModelIndex &other) const;
   inline void sendNamespaces(QAbstractXmlReceiver *const receiver) const;
   inline QVector<QXmlName> namespaceBindings() const;
   inline QXmlName::NamespaceCode namespaceForPrefix(const QXmlName::PrefixCode prefix) const;
   inline QString stringValue() const;

   inline QExplicitlySharedDataPointer<QPatternist::ItemType> type() const;
   inline bool is(const QXmlNodeModelIndex &other) const;

   inline void reset() {
      m_storage.reset();
   }

 private:
   static inline QXmlNodeModelIndex create(const qint64 d, const QAbstractXmlNodeModel *const nm) {
      QXmlNodeModelIndex n;
      n.m_storage.data = d;
      n.m_storage.model = nm;
      n.m_storage.additionalData = 0;
      return n;
   }

   static inline QXmlNodeModelIndex create(const qint64 data,
                                           const QAbstractXmlNodeModel *const nm,
                                           const qint64 addData) {
      QXmlNodeModelIndex n;
      n.m_storage.data = data;
      n.m_storage.model = nm;
      n.m_storage.additionalData = addData;
      return n;
   }

   inline QXmlNodeModelIndex(const QPatternist::NodeIndexStorage &storage) : m_storage(storage) {
   }

   friend class QAbstractXmlNodeModel;
   friend class QPatternist::Item;
   friend class QXmlItem;
   inline operator int() const; // Disable

   QPatternist::NodeIndexStorage m_storage;
};

Q_XMLPATTERNS_EXPORT uint qHash(const QXmlNodeModelIndex &index);

inline bool qIsForwardIteratorEnd(const QXmlNodeModelIndex &item)
{
   return item.isNull();
}

class Q_XMLPATTERNS_EXPORT QAbstractXmlNodeModel : public QSharedData
{
 public:
   enum SimpleAxis {
      Parent,
      FirstChild,
      PreviousSibling,
      NextSibling
   };

   typedef QExplicitlySharedDataPointer<QAbstractXmlNodeModel> Ptr;
   typedef QList<Ptr> List;

   QAbstractXmlNodeModel();
   virtual ~QAbstractXmlNodeModel();

   virtual QUrl baseUri(const QXmlNodeModelIndex &index) const = 0;
   virtual QUrl documentUri(const QXmlNodeModelIndex &index) const = 0;
   virtual QXmlNodeModelIndex::NodeKind kind(const QXmlNodeModelIndex &index) const = 0;
   virtual QXmlNodeModelIndex::DocumentOrder compareOrder(const QXmlNodeModelIndex &index1,
            const QXmlNodeModelIndex &index2) const = 0;

   virtual QXmlNodeModelIndex root(const QXmlNodeModelIndex &index) const = 0;
   virtual QXmlName name(const QXmlNodeModelIndex &index) const = 0;
   virtual QString stringValue(const QXmlNodeModelIndex &index) const = 0;
   virtual QVariant typedValue(const QXmlNodeModelIndex &index) const = 0;

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   virtual QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> > iterate(
            const QXmlNodeModelIndex &ni, QXmlNodeModelIndex::Axis axis) const;

   virtual QPatternist::ItemIteratorPtr sequencedTypedValue(const QXmlNodeModelIndex &index) const;
   virtual QExplicitlySharedDataPointer<QPatternist::ItemType> type(const QXmlNodeModelIndex &index) const;

   virtual QXmlName::NamespaceCode namespaceForPrefix(const QXmlNodeModelIndex &index,
            const QXmlName::PrefixCode prefix) const;

   virtual bool isDeepEqual(const QXmlNodeModelIndex &index1, const QXmlNodeModelIndex &index2) const;
   virtual void sendNamespaces(const QXmlNodeModelIndex &index, QAbstractXmlReceiver *const receiver) const;
   virtual QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &index) const = 0;

   virtual QXmlNodeModelIndex elementById(const QXmlName &id) const = 0;
   virtual QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &id) const = 0;

   enum NodeCopySetting {
      InheritNamespaces   = 0x1,
      PreserveNamespaces  = 0x2
   };

   typedef QFlags<NodeCopySetting> NodeCopySettings;
   virtual void copyNodeTo(const QXmlNodeModelIndex &node, QAbstractXmlReceiver *const receiver,
            const NodeCopySettings &) const;

   QSourceLocation sourceLocation(const QXmlNodeModelIndex &index) const;

 protected:
   virtual QXmlNodeModelIndex nextFromSimpleAxis(SimpleAxis axis, const QXmlNodeModelIndex &origin) const = 0;
   virtual QVector<QXmlNodeModelIndex> attributes(const QXmlNodeModelIndex &element) const = 0;

   QAbstractXmlNodeModel(QAbstractXmlNodeModelPrivate *d);

   inline QXmlNodeModelIndex createIndex(qint64 data) const {
      return QXmlNodeModelIndex::create(data, this);
   }

   inline QXmlNodeModelIndex createIndex(void *pointer, qint64 additionalData = 0) const {
      return QXmlNodeModelIndex::create(qptrdiff(pointer), this, additionalData);
   }

   inline QXmlNodeModelIndex createIndex(qint64 data, qint64 additionalData) const {
      return QXmlNodeModelIndex::create(data, this, additionalData);
   }

   QScopedPointer<QAbstractXmlNodeModelPrivate> d_ptr;

 private:
   friend class QPatternist::ItemMappingIterator<QXmlNodeModelIndex, QXmlNodeModelIndex,
            const QAbstractXmlNodeModel *, QExplicitlySharedDataPointer<QPatternist::DynamicContext> >;

   friend class QPatternist::SequenceMappingIterator<QXmlNodeModelIndex, QXmlNodeModelIndex,
            const QAbstractXmlNodeModel *>;

   friend class QPatternist::XsdValidatedXmlNodeModel;

   inline QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> > mapToSequence(
      const QXmlNodeModelIndex &ni,
      const QExplicitlySharedDataPointer<QPatternist::DynamicContext> &) const;

   static inline bool isIgnorableInDeepEqual(const QXmlNodeModelIndex &n);
   QAbstractXmlNodeModel(const QAbstractXmlNodeModel &) = delete;
   QAbstractXmlNodeModel &operator=(const QAbstractXmlNodeModel &) = delete;
};

template<typename T>
class QAbstractXmlForwardIterator;

class QVariant;
class QXmlItemPrivate;

namespace QPatternist {
   class AtomicValue;
   class VariableLoader;
   class IteratorBridge;
   class ToQXmlItemMapper;
   class ToItemMapper;
}

class Q_XMLPATTERNS_EXPORT QXmlItem
{
 public:
   typedef QAbstractXmlForwardIterator<QXmlItem> Iterator;

   QXmlItem();
   QXmlItem(const QXmlItem &other);
   QXmlItem(const QXmlNodeModelIndex &node);
   QXmlItem(const QVariant &atomicValue);
   ~QXmlItem();

   QXmlItem &operator=(const QXmlItem &other);

   bool isNull() const;
   bool isNode() const;
   bool isAtomicValue() const;

   QVariant toAtomicValue() const;
   QXmlNodeModelIndex toNodeModelIndex() const;

 private:
   friend class QPatternist::IteratorBridge;
   friend class QPatternist::VariableLoader;
   friend class QPatternist::ToQXmlItemMapper;
   friend class QPatternist::ToItemMapper;
   friend class QPatternist::Item;

   inline bool internalIsAtomicValue() const;

   inline QXmlItem(const QPatternist::Item &i);

   union {
      QPatternist::NodeIndexStorage   m_node;
      const QPatternist::AtomicValue *m_atomicValue;

      QXmlItemPrivate *m_ptr;    // not currently used.
   };
};

inline bool qIsForwardIteratorEnd(const QXmlItem &item)
{
   return item.isNull();
}

CS_DECLARE_METATYPE(QXmlItem)

#endif
