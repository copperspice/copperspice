/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QVector>

#include "qabstractxmlnodemodel_p.h"
#include "qabstractxmlreceiver.h"
#include "qcommonvalues_p.h"
#include "qemptyiterator_p.h"
#include "qitemmappingiterator_p.h"
#include "qitem_p.h"
#include "qnamespaceresolver_p.h"
#include "qsequencemappingiterator_p.h"
#include "qsingletoniterator_p.h"

#include "qabstractxmlnodemodel.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

typedef QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> >
QXmlNodeModelIndexIteratorPointer;

bool QAbstractXmlNodeModel::isIgnorableInDeepEqual(const QXmlNodeModelIndex &n)
{
   Q_ASSERT(!n.isNull());
   const QXmlNodeModelIndex::NodeKind nk = n.kind();
   return nk == QXmlNodeModelIndex::ProcessingInstruction ||
          nk == QXmlNodeModelIndex::Comment;
}

using namespace QPatternist;

/*!
  Default constructor.
 */
QAbstractXmlNodeModel::QAbstractXmlNodeModel() : d_ptr(0)
{
}

/*!
 \internal

 Takes the d-pointer.

 */
QAbstractXmlNodeModel::QAbstractXmlNodeModel(QAbstractXmlNodeModelPrivate *d) : d_ptr(d)
{
}

/*!
  Destructor.
 */
QAbstractXmlNodeModel::~QAbstractXmlNodeModel()
{
}

/*
### Qt5:

Add the function:

    virtual QSourceLocation sourceLocation(const QXmlNodeModelIndex &nodeIndex) const = 0;

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
   Q_DISABLE_COPY(MergeIterator)
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
QAbstractXmlNodeModel::mapToSequence(const QXmlNodeModelIndex &ni,
                                     const DynamicContext::Ptr &) const
{
   Q_ASSERT(!ni.isNull());
   /* Since we pass in this here, mapToSequence is used recursively. */
   return mergeIterators(ni, makeSequenceMappingIterator<QXmlNodeModelIndex>(this,
                         ni.iterate(QXmlNodeModelIndex::AxisChild),
                         DynamicContext::Ptr()));
}

/*!
  \fn QVector<QXmlNodeModelIndex> QAbstractXmlNodeModel::attributes(const QXmlNodeModelIndex &element) const

  Returns the attributes of \a element. The caller guarantees
  that \a element is an element in this node model.
 */

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
QAbstractXmlNodeModel::iterate(const QXmlNodeModelIndex &ni,
                               QXmlNodeModelIndex::Axis axis) const
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

         while (!ancestor.isNull()) {
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
               /* Fallthrough. */
               case QXmlNodeModelIndex::ProcessingInstruction:
               /* Fallthrough. */
               case QXmlNodeModelIndex::Element:
               /* Fallthrough. */
               case QXmlNodeModelIndex::Text:
                  return makeSingletonIterator(ni);
               case QXmlNodeModelIndex::Attribute:
               /* Fallthrough. */
               case QXmlNodeModelIndex::Document:
               /* Fallthrough. */
               case QXmlNodeModelIndex::Namespace:
                  /* Do nothing. */
                  ;
            }
         }

         /* Else, fallthrough to AxisChild. */
      }
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

         /* Else, fallthrough to AxisAttribute. */
      }
      case QXmlNodeModelIndex::AxisAttribute:
         return makeVectorIterator(attributes(ni));
      case QXmlNodeModelIndex::AxisDescendantOrSelf:
         return mergeIterators(ni, iterate(ni, QXmlNodeModelIndex::AxisDescendant));
      case QXmlNodeModelIndex::AxisFollowing:
      /* Fallthrough. */
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
   if (candidate.isNull()) {
      return QPatternist::CommonValues::emptyIterator;
   } else {
      return makeSingletonIterator(AtomicValue::toXDM(candidate));
   }
}

/*!
 \internal
 */
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

         /* Fallthrough, so we check the children. */
      }
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

      /* Fallthrough */
      case QXmlNodeModelIndex::ProcessingInstruction:

      /* Fallthrough. */
      case QXmlNodeModelIndex::Text:

      /* Fallthrough. */
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

/*!
  The copy constructor constructs a copy of \a other.
 */
QXmlItem::QXmlItem(const QXmlItem &other) : m_node(other.m_node)
{
   if (internalIsAtomicValue()) {
      m_atomicValue->ref.ref();
   }
}

/*!
  Constructs an atomic value QXmlItem with \a atomicValue.

  \sa isAtomicValue()
 */
QXmlItem::QXmlItem(const QVariant &atomicValue)
{
   m_node.reset();
   if (atomicValue.isNull()) {
      /* Then we behave just like the default constructor. */
      return;
   }

   /*
     We can't assign directly to m_atomicValue, because the
     temporary will self-destruct before we've ref'd it.
   */
   const QPatternist::Item temp(QPatternist::AtomicValue::toXDM(atomicValue));

   if (temp) {
      temp.asAtomicValue()->ref.ref();
      m_node.model = reinterpret_cast<const QAbstractXmlNodeModel *>(~0);
      m_atomicValue = temp.asAtomicValue();
   } else {
      m_atomicValue = 0;
   }
}

/*!
  Constructs a node QXmlItem that is a copy of \a node.

  \sa isNode()
 */
QXmlItem::QXmlItem(const QXmlNodeModelIndex &node) : m_node(node.m_storage)
{
}


/*!
  Destructor.
 */
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

/*!
  Assigns \a other to \c this.
 */
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

/*!
  Returns true if this item is a Node. Returns false if it
  is an atomic value or null.

  \sa isNull(), isAtomicValue()
 */
bool QXmlItem::isNode() const
{
   return QPatternist::Item::fromPublic(*this).isNode();
}

/*!
  Returns true if this item is an atomic value. Returns false
  if it is a node or null.

  \sa isNull(), isNode()
 */
bool QXmlItem::isAtomicValue() const
{
   return internalIsAtomicValue();
}

/*!
  If this QXmlItem represents an atomic value, it is converted
  to an appropriate QVariant and returned. If this QXmlItem is
  not an atomic value, the return value is a default constructed
  QVariant. You can call isAtomicValue() to test whether the
  item is an atomic value.

 \sa isAtomicValue()
 */
QVariant QXmlItem::toAtomicValue() const
{
   if (isAtomicValue()) {
      return QPatternist::AtomicValue::toQt(m_atomicValue);
   } else {
      return QVariant();
   }
}

/*!
  If this QXmlItem represents a node, it returns the item as a
  QXmlNodeModelIndex. If this QXmlItem is not a node, the return
  value is undefined. You can call isNode() to test whether the
  item is a node.

 \sa isNode()
 */
QXmlNodeModelIndex QXmlItem::toNodeModelIndex() const
{
   if (isNode()) {
      return reinterpret_cast<const QXmlNodeModelIndex &>(m_node);
   } else {
      return QXmlNodeModelIndex();
   }
}

/*!
  Returns true if this QXmlItem is neither a node nor an
  atomic value. Default constructed instances of QXmlItem
  are null.
 */
bool QXmlItem::isNull() const
{
   return !m_node.model;
}

/*!
  \class QXmlNodeModelIndex
  \brief The QXmlNodeModelIndex class identifies a node in an XML node model subclassed from QAbstractXmlNodeModel.
  \reentrant
  \since 4.4
  \ingroup xml-tools

  QXmlNodeModelIndex is an index into an \l{QAbstractXmlNodeModel}
  {XML node model}. It contains:

  \list
    \o A pointer to an \l{QAbstractXmlNodeModel} {XML node model},
    which is returned by model(), and
    \o Some data, which is returned by data(), internalPointer(),
    and additionalData().
  \endlist

  Because QXmlNodeModelIndex is intentionally a simple class, it
  doesn't have member functions for accessing the properties of
  nodes. For example, it doesn't have functions for getting a
  node's name or its list of attributes or child nodes. If you find
  that you need to retrieve this kind of information from your
  query results, there are two ways to proceed.

  \list

  \o Send the output of your XQuery to an \l{QAbstractXmlReceiver}
  {XML receiver}, or

  \o Let your XQuery do all the work to produce the desired result.

  \endlist

  The second case is explained by example. Suppose you want to
  populate a list widget with the values of certain attributes from a
  set of result elements. You could write an XQuery to return the set
  of elements, and then you would write the code to iterate over the
  result elements, get their attributes, and extract the desired
  string values. But the simpler way is to just augment your XQuery to
  finding the desired attribute values. Then all you have to do is
  evaluate the XQuery using the version of QXmlQuery::evaluateTo()
  that populates a QStringList, which you can send directly to your
  widget.

  QXmlNodeModelIndex doesn't impose any restrictions on the \c data
  value an QXmlNodeModelIndex should contain. The meaning of the data
  left to the associated \l {QAbstractXmlNodeModel} {node model}.
  Because QXmlNodeModelIndex depends on a particular subclass of
  QAbstractXmlNodeModel for its existence, the only way you can create
  an instance of QXmlNodeModelIndex is by asking the node model to
  create one for you with QAbstractXmlNodeModel::createIndex(). Since
  that function is protected, it is usually a good idea to write a
  public function that creates a QXmlNodeModelIndex from arguments that
  are appropriate for your particular node model.

  A default constructed node index is said to be null, i.e., isNull()
  returns true.

  QXmlNodeModelIndex and QAbstractXmlNodeModel follow the same design
  pattern used for QModelIndex and QAbstractItemModel.
 */

/*!
  \since 4.4
  \relates QHash

  Computes a hash key from the QXmlNodeModelIndex \a index, and
  returns it. This function would be used by QHash if you wanted
  to build a hash table for instances of QXmlNodeModelIndex.

  The hash is computed on QXmlNodeModelIndex::data(),
  QXmlNodeModelIndex::additionalData(), and
  QXmlNodeModelIndex::model(). This means the hash key can be used for
  node indexes from different node models.
 */
uint qHash(const QXmlNodeModelIndex &index)
{
   return uint(index.data() + index.additionalData() + quintptr(index.model()));
}

/*!
  \enum QXmlNodeModelIndex::NodeKind

  Identifies a kind of node.

  \value Attribute Identifies an attribute node
  \value Text Identifies a text node
  \value Comment Identifies a comment node
  \value Document Identifies a document node
  \value Element Identifies an element node
  \value Namespace Identifies a namespace node
  \value ProcessingInstruction Identifies a processing instruction.

  Note that the optional XML declaration at very beginning of the XML
  document is not a processing instruction

  \sa QAbstractXmlNodeModel::kind()
*/

/*!
 \typedef QXmlNodeModelIndex::List

 Typedef for QList<QXmlNodeModelIndex>.
 */

/*!
  Returns true if this node is the same as \a other. This operator
  does not compare values, children, or names of nodes. It compares
  node identities, i.e., whether two nodes are from the same document
  and are found at the exact same place.
 */
bool QXmlNodeModelIndex::operator==(const QXmlNodeModelIndex &other) const
{
   return !(m_storage != other.m_storage);
}

/*!
  Returns true if \a other is the same node as this.
 */
bool QXmlNodeModelIndex::operator!=(const QXmlNodeModelIndex &other) const
{
   return !(operator==(other));
}

void QAbstractXmlNodeModel::copyNodeTo(const QXmlNodeModelIndex &node,
                  QAbstractXmlReceiver *const receiver, const NodeCopySettings &copySettings) const
{
   Q_UNUSED(node);
   Q_UNUSED(receiver);
   Q_UNUSED(copySettings);

   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
}

QSourceLocation QAbstractXmlNodeModel::sourceLocation(const QXmlNodeModelIndex &index) const
{
   // TODO: make this method virtual in Qt5 to allow source location support in custom models

   if (d_ptr) {
      return d_ptr->sourceLocation(index);
   } else {
      return QSourceLocation();
   }
}

QT_END_NAMESPACE
