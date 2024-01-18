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

#include "qabstractxmlnodemodel.h"

#include <qvector.h>
#include "qabstractxmlreceiver.h"
#include "qitem_p.h"

#include "qabstractxmlnodemodel_p.h"
#include "qcommonvalues_p.h"
#include "qemptyiterator_p.h"
#include "qitemmappingiterator_p.h"
#include "qnamespaceresolver_p.h"
#include "qsequencemappingiterator_p.h"
#include "qsingletoniterator_p.h"

using namespace QPatternist;

typedef QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> >
QXmlNodeModelIndexIteratorPointer;

bool QAbstractXmlNodeModel::isIgnorableInDeepEqual(const QXmlNodeModelIndex &n)
{
   Q_ASSERT(! n.isNull());

   const QXmlNodeModelIndex::NodeKind nk = n.kind();
   return nk == QXmlNodeModelIndex::ProcessingInstruction || nk == QXmlNodeModelIndex::Comment;
}

using namespace QPatternist;

QAbstractXmlNodeModel::QAbstractXmlNodeModel()
   : d_ptr(nullptr)
{
}

QAbstractXmlNodeModel::QAbstractXmlNodeModel(QAbstractXmlNodeModelPrivate *d)
   : d_ptr(d)
{
}

QAbstractXmlNodeModel::~QAbstractXmlNodeModel()
{
}

/*
TODO: Add : virtual QSourceLocation sourceLocation(const QXmlNodeModelIndex &nodeIndex) const = 0;
Such that the data model can communicate back source locations.
*/

namespace QPatternist {
class MergeIterator
{
 public:
   inline MergeIterator() {
   }

   inline
   QXmlNodeModelIndexIteratorPointer
   mapToSequence(const QXmlNodeModelIndexIteratorPointer &it,
                 const DynamicContext::Ptr &) const {
      return it;
   }

 private:
   MergeIterator(const MergeIterator &) = delete;
   MergeIterator &operator=(const MergeIterator &) = delete;
};

static const MergeIterator mergeIterator;

class IteratorVector : public
   ListIterator<QXmlNodeModelIndexIteratorPointer, QVector<QXmlNodeModelIndexIteratorPointer> >
{
   typedef QVector<QXmlNodeModelIndexIteratorPointer> ItVector;
 public:
   typedef QAbstractXmlForwardIterator<QXmlNodeModelIndexIteratorPointer>::Ptr Ptr;

   IteratorVector(const ItVector &in) :
      ListIterator<QXmlNodeModelIndexIteratorPointer, QVector<QXmlNodeModelIndexIteratorPointer> >(in) {
   }

   QAbstractXmlForwardIterator<QXmlNodeModelIndexIteratorPointer>::Ptr copy() const override {
      ItVector result;

      for (int i = 0; i < m_list.count(); ++i) {
         result.append(m_list.at(i)->copy());
      }

      return Ptr(new IteratorVector(result));
   }
};
}

/*!
 \internal
 This function is not a private member of QAbstractXmlNodeModel
 because it would be messy to forward declare the required types.
*/
static inline QXmlNodeModelIndexIteratorPointer mergeIterators(const QXmlNodeModelIndex &node,
      const QXmlNodeModelIndexIteratorPointer &it2)
{
   QVector<QXmlNodeModelIndexIteratorPointer> iterators;
   iterators.append(makeSingletonIterator(node));
   iterators.append(it2);

   return makeSequenceMappingIterator<QXmlNodeModelIndex>(&mergeIterator,
          IteratorVector::Ptr(new IteratorVector(iterators)),
          DynamicContext::Ptr());
}

inline QAbstractXmlForwardIterator<QXmlNodeModelIndex>::Ptr
QAbstractXmlNodeModel::mapToSequence(const QXmlNodeModelIndex &ni, const DynamicContext::Ptr &) const
{
   Q_ASSERT(! ni.isNull());

   /* Since we pass in this here, mapToSequence is used recursively. */
   return mergeIterators(ni, makeSequenceMappingIterator<QXmlNodeModelIndex>(this,
                         ni.iterate(QXmlNodeModelIndex::AxisChild),
                         DynamicContext::Ptr()));
}

/*!
  \internal

  Performs navigation, starting from \a ni, by returning an
  QAbstractXmlForwardIterator that returns nodes the \a axis emanating
  from \a ni.

  The implementation returns the nodes on the \a axis, without
  duplicates and in \a axis order. This means that if \a axis is a
  reverse axis, which is the case for the \c parent, \c ancestor, \c
  ancestor-or-self, \c preceding, and \c preceding-sibling, the nodes
  are delivered in reverse document order. Otherwise the nodes are
  delivered in document order.

  The implementor guarantees that the nodes delivered for the axes are
  consistent with the XPath Data Model. This just implies common
  sense, e.g., The child axis for a comment node can't contain any
  children; a document node can't be a child of an element, etc.
  Attributes aren't considered children of an element, but are only
  available on AxisAttribute.

  The value past in \a axis is not guaranteed based on what is used in
  a query. QtXmlPatterns may call this function arbitrarily with any
  value for \a axis. This is because QtXmlPatterns may rewrite queries
  to be more efficient, using axes in different ways from the original
  query.

  QAbstractXmlNodeModel::Axis has a good overview of the axes and what
  they select.

  The caller guarantees that \a ni is not \c null and that it belongs
  to this QAbstractXmlNodeModel instance.

  Implementing iterate() can involve significant work, since it
  requires different iterators for all the axes used. In the worst
  case, it could require writing as many QAbstractXmlForwardIterator
  subclasses as there are axes, but the number can often be reduced
  with clever use of lists and template classes. It is better to use
  or subclass QSimpleXmlNodeModel, which makes it easier to write the
  node navigation code without loss of efficiency or flexibility.

  \sa QSimpleXmlNodeModel
  \sa QXmlNodeModelIndex::Axis
  \sa {http://www.w3.org/TR/xquery/#axes}{XQuery 1.0: An XML Query Language, 3.2.1.1 Axes}
  \sa {http://www.w3.org/TR/xpath-datamodel/}{W3CXQuery 1.0 and XPath 2.0 Data Model (XDM)}
 */

QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> >

QAbstractXmlNodeModel::iterate(const QXmlNodeModelIndex &ni, QXmlNodeModelIndex::Axis axis) const
{
   /* Returns iterators that track state and calls nextFromSimpleAxis()
    * iteratively. Typically, when sub-classing QSimpleXmlNodeModel,
    * you don't reimplement this function, but instead implement
    * nextFromSimpleAxis(). */

   switch (axis) {
      case QXmlNodeModelIndex::AxisSelf:
         return makeSingletonIterator(ni);

      case QXmlNodeModelIndex::AxisParent: {
         if (kind(ni) == QXmlNodeModelIndex::Document) {
            return makeEmptyIterator<QXmlNodeModelIndex>();
         } else {
            return makeSingletonIterator(nextFromSimpleAxis(Parent, ni));
         }
      }

      case QXmlNodeModelIndex::AxisNamespace:
         return makeEmptyIterator<QXmlNodeModelIndex>();
      case QXmlNodeModelIndex::AxisAncestor: {
         QList<QXmlNodeModelIndex> ancestors;
         QXmlNodeModelIndex ancestor = nextFromSimpleAxis(Parent, ni);

         while (! ancestor.isNull()) {
            ancestors.append(ancestor);
            ancestor = nextFromSimpleAxis(Parent, ancestor);
         }

         return makeListIterator(ancestors);
      }

      case QXmlNodeModelIndex::AxisAncestorOrSelf: {
         QList<QXmlNodeModelIndex> ancestors;
         ancestors.append(ni);
         QXmlNodeModelIndex ancestor = nextFromSimpleAxis(Parent, ni);

         while (!ancestor.isNull()) {
            ancestors.append(ancestor);
            ancestor = nextFromSimpleAxis(Parent, ancestor);
         }

         return makeListIterator(ancestors);
      }
      case QXmlNodeModelIndex::AxisPrecedingSibling: {
         QList<QXmlNodeModelIndex> preceding;
         QXmlNodeModelIndex sibling = nextFromSimpleAxis(PreviousSibling, ni);

         while (!sibling.isNull()) {
            preceding.append(sibling);
            sibling = nextFromSimpleAxis(PreviousSibling, sibling);
         }

         return makeListIterator(preceding);
      }

      case QXmlNodeModelIndex::AxisFollowingSibling: {
         QList<QXmlNodeModelIndex> preceding;
         QXmlNodeModelIndex sibling = nextFromSimpleAxis(NextSibling, ni);

         while (!sibling.isNull()) {
            preceding.append(sibling);
            sibling = nextFromSimpleAxis(NextSibling, sibling);
         }

         return makeListIterator(preceding);
      }

      case QXmlNodeModelIndex::AxisChildOrTop: {
         if (nextFromSimpleAxis(Parent, ni).isNull()) {
            switch (kind(ni)) {
               case QXmlNodeModelIndex::Comment:
               case QXmlNodeModelIndex::ProcessingInstruction:
               case QXmlNodeModelIndex::Element:
               case QXmlNodeModelIndex::Text:
                  return makeSingletonIterator(ni);

               case QXmlNodeModelIndex::Attribute:
               case QXmlNodeModelIndex::Document:
               case QXmlNodeModelIndex::Namespace:
                  // do nothing
                  ;
            }
         }
      }
      [[fallthrough]];

      case QXmlNodeModelIndex::AxisChild: {
         QList<QXmlNodeModelIndex> children;
         QXmlNodeModelIndex child = nextFromSimpleAxis(FirstChild, ni);

         while (!child.isNull()) {
            children.append(child);
            child = nextFromSimpleAxis(NextSibling, child);
         }

         return makeListIterator(children);
      }

      case QXmlNodeModelIndex::AxisDescendant: {
         return makeSequenceMappingIterator<QXmlNodeModelIndex>(this,
                ni.iterate(QXmlNodeModelIndex::AxisChild),
                DynamicContext::Ptr());
      }

      case QXmlNodeModelIndex::AxisAttributeOrTop: {
         if (kind(ni) == QXmlNodeModelIndex::Attribute && nextFromSimpleAxis(Parent, ni).isNull()) {
            return makeSingletonIterator(ni);
         }
      }
      [[fallthrough]];

      case QXmlNodeModelIndex::AxisAttribute:
         return makeVectorIterator(attributes(ni));

      case QXmlNodeModelIndex::AxisDescendantOrSelf:
         return mergeIterators(ni, iterate(ni, QXmlNodeModelIndex::AxisDescendant));

      case QXmlNodeModelIndex::AxisFollowing:
      case QXmlNodeModelIndex::AxisPreceding: {
         /* We walk up along the ancestors, and for each parent, we grab its preceding/following
          * siblings, and evaluate the descendant axis. The descendant axes gets added
          * to a list and we then merge those iterators. */
         QVector<QXmlNodeModelIndexIteratorPointer> descendantIterators;

         QXmlNodeModelIndex current(ni);

         while (!current.isNull()) {
            QXmlNodeModelIndex candidate(nextFromSimpleAxis(axis == QXmlNodeModelIndex::AxisPreceding ? PreviousSibling :
                                         NextSibling, current));
            if (candidate.isNull()) {
               /* current is an ancestor. We don't want it, so next iteration we
                * will grab its preceding sibling. */
               current = nextFromSimpleAxis(Parent, current);
            } else {
               current = candidate;
               descendantIterators.append(iterate(current, QXmlNodeModelIndex::AxisDescendantOrSelf)->toReversed());
            }
         }

         return makeSequenceMappingIterator<QXmlNodeModelIndex>(&mergeIterator,
                IteratorVector::Ptr(new IteratorVector(descendantIterators)),
                DynamicContext::Ptr());
      }
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown axis, internal error.");

   return makeEmptyIterator<QXmlNodeModelIndex>();
}

void QAbstractXmlNodeModel::sendNamespaces(const QXmlNodeModelIndex &n, QAbstractXmlReceiver *const receiver) const
{
   Q_ASSERT(receiver);
   const QVector<QXmlName> nss(namespaceBindings(n));

   /* This is by far the most common case. */
   if (nss.isEmpty()) {
      return;
   }

   const int len = nss.size();
   for (int i = 0; i < len; ++i) {
      receiver->namespaceBinding(nss.at(i));
   }
}

QPatternist::ItemIteratorPtr QAbstractXmlNodeModel::sequencedTypedValue(const QXmlNodeModelIndex &ni) const
{
   const QVariant &candidate = typedValue(ni);

   if (! candidate.isValid()) {
      return QPatternist::CommonValues::emptyIterator;
   } else {
      return makeSingletonIterator(AtomicValue::toXDM(candidate));
   }
}

QExplicitlySharedDataPointer<QPatternist::ItemType> QAbstractXmlNodeModel::type(const QXmlNodeModelIndex &) const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function is internal, unsupported, and should never be called.");
   return QPatternist::ItemType::Ptr();
}

QXmlName::NamespaceCode QAbstractXmlNodeModel::namespaceForPrefix(const QXmlNodeModelIndex &ni,
      const QXmlName::PrefixCode prefix) const
{
   Q_ASSERT(kind(ni) == QXmlNodeModelIndex::Element);

   const QVector<QXmlName> nbs(namespaceBindings(ni));
   const int len = nbs.size();

   for (int i = 0; i < len; ++i) {
      if (nbs.at(i).prefix() == prefix) {
         return nbs.at(i).namespaceURI();
      }
   }

   return NamespaceResolver::NoBinding;
}


/*!
  \internal

  Determines whether \a ni1 is deep equal to \a ni2.

  isDeepEqual() is defined as evaluating the expression \c
  fn:deep-equal($n1, $n2) where \c $n1 is \a ni1 and \c $n1 is \a
  ni2. This function is associative, meaning the same value is
  returned regardless of if isDeepEqual() is invoked with \a ni1 as
  first argument or second. It is guaranteed that \a ni1 and \a ni2
  are nodes, as opposed to the definition of \c fn:deep-equal().

  Returns true if \a ni1 is deep-equal to \a ni2, otherwise false

  \sa {"http://www.w3.org/TR/xpath-functions/#func-deep-equal"}{XQuery 1.0 and XPath 2.0 Functions and Operators, 15.3.1 fn:deep-equal}
 */
bool QAbstractXmlNodeModel::isDeepEqual(const QXmlNodeModelIndex &n1, const QXmlNodeModelIndex &n2) const
{
   Q_ASSERT(!n1.isNull());
   Q_ASSERT(!n2.isNull());

   const QXmlNodeModelIndex::NodeKind nk = n1.kind();

   if (nk != n2.kind()) {
      return false;
   }

   if (n1.name() != n2.name()) {
      return false;
   }

   switch (nk) {
      case QXmlNodeModelIndex::Element: {
         QXmlNodeModelIndexIteratorPointer atts1(n1.iterate(QXmlNodeModelIndex::AxisAttribute));
         QXmlNodeModelIndex node(atts1->next());

         const QXmlNodeModelIndex::List atts2(n2.iterate(QXmlNodeModelIndex::AxisAttribute)->toList());
         const QXmlNodeModelIndex::List::const_iterator end(atts2.constEnd());

         while (!node.isNull()) {
            bool equal = false;
            for (QXmlNodeModelIndex::List::const_iterator it = atts2.constBegin(); it != end; ++it) {
               if (isDeepEqual(node, (*it))) {
                  equal = true;
               }
            }

            if (!equal) {
               return false;
            }

            node = atts1->next();
         }
      }
      [[fallthrough]];

      case QXmlNodeModelIndex::Document: {
         QXmlNodeModelIndexIteratorPointer itn1(n1.iterate(QXmlNodeModelIndex::AxisChild));
         QXmlNodeModelIndexIteratorPointer itn2(n2.iterate(QXmlNodeModelIndex::AxisChild));

         while (true) {
            QXmlNodeModelIndex no1(itn1->next());
            QXmlNodeModelIndex no2(itn2->next());

            while (!no1.isNull() && isIgnorableInDeepEqual(no1)) {
               no1 = itn1->next();
            }

            while (!no2.isNull() && isIgnorableInDeepEqual(no2)) {
               no2 = itn2->next();
            }

            if (!no1.isNull() && !no2.isNull()) {
               if (!isDeepEqual(no1, no2)) {
                  return false;
               }
            } else {
               return no1.isNull() && no2.isNull();
            }
         }

         return true;
      }

      case QXmlNodeModelIndex::Attribute:
      case QXmlNodeModelIndex::ProcessingInstruction:
      case QXmlNodeModelIndex::Text:
      case QXmlNodeModelIndex::Comment:
         return n1.stringValue() == n2.stringValue();

      case QXmlNodeModelIndex::Namespace: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "QXmlNodeModelIndex::Namespace was not implemented");
         return false;
      }
   }

   return false;
}

QXmlItem::QXmlItem()
{
   m_node.reset();
}

bool QXmlItem::internalIsAtomicValue() const
{
   return m_node.model == reinterpret_cast<QAbstractXmlNodeModel *>(~0);
}

QXmlItem::QXmlItem(const QXmlItem &other) : m_node(other.m_node)
{
   if (internalIsAtomicValue()) {
      m_atomicValue->ref.ref();
   }
}

QXmlItem::QXmlItem(const QVariant &atomicValue)
{
   m_node.reset();
   if (! atomicValue.isValid()) {
      /* Then we behave just like the default constructor. */
      return;
   }

   /*
     can not assign directly to m_atomicValue, because the
     temporary will self-destruct before we've ref'd it.
   */
   const QPatternist::Item temp(QPatternist::AtomicValue::toXDM(atomicValue));

   if (temp) {
      temp.asAtomicValue()->ref.ref();
      m_node.model = reinterpret_cast<const QAbstractXmlNodeModel *>(~0);
      m_atomicValue = temp.asAtomicValue();

   } else {
      m_atomicValue = nullptr;
   }
}

QXmlItem::QXmlItem(const QXmlNodeModelIndex &node) : m_node(node.m_storage)
{
}

QXmlItem::~QXmlItem()
{
   if (internalIsAtomicValue() && !m_atomicValue->ref.deref()) {
      delete m_atomicValue;
   }
}

bool QPatternist::NodeIndexStorage::operator!=(const NodeIndexStorage &other) const
{
   return data != other.data
          || additionalData != other.additionalData
          || model != other.model;
}

QXmlItem &QXmlItem::operator=(const QXmlItem &other)
{
   if (m_node != other.m_node) {
      if (internalIsAtomicValue() && !m_atomicValue->ref.deref()) {
         delete m_atomicValue;
      }

      m_node = other.m_node;

      if (internalIsAtomicValue()) {
         m_atomicValue->ref.ref();
      }
   }

   return *this;
}

bool QXmlItem::isNode() const
{
   return QPatternist::Item::fromPublic(*this).isNode();
}

bool QXmlItem::isAtomicValue() const
{
   return internalIsAtomicValue();
}

QVariant QXmlItem::toAtomicValue() const
{
   if (isAtomicValue()) {
      return QPatternist::AtomicValue::toQt(m_atomicValue);
   } else {
      return QVariant();
   }
}

QXmlNodeModelIndex QXmlItem::toNodeModelIndex() const
{
   if (isNode()) {
      return reinterpret_cast<const QXmlNodeModelIndex &>(m_node);
   } else {
      return QXmlNodeModelIndex();
   }
}

bool QXmlItem::isNull() const
{
   return !m_node.model;
}

uint qHash(const QXmlNodeModelIndex &index)
{
   return uint(index.data() + index.additionalData() + quintptr(index.model()));
}

bool QXmlNodeModelIndex::operator==(const QXmlNodeModelIndex &other) const
{
   return !(m_storage != other.m_storage);
}

bool QXmlNodeModelIndex::operator!=(const QXmlNodeModelIndex &other) const
{
   return !(operator==(other));
}

void QAbstractXmlNodeModel::copyNodeTo(const QXmlNodeModelIndex &node,
                  QAbstractXmlReceiver *const receiver, const NodeCopySettings &copySettings) const
{
   (void) node;
   (void) receiver;
   (void) copySettings;

   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
}

QSourceLocation QAbstractXmlNodeModel::sourceLocation(const QXmlNodeModelIndex &index) const
{
   // TODO: make this method virtual to allow source location support in custom models

   if (d_ptr) {
      return d_ptr->sourceLocation(index);
   } else {
      return QSourceLocation();
   }
}
