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

#include <QUrl>
#include <QVector>
#include <QXmlNamePool>

#include "qabstractxmlnodemodel_p.h"
#include "qemptyiterator_p.h"
#include "qitemmappingiterator_p.h"
#include "qsequencemappingiterator_p.h"
#include "qsimplexmlnodemodel.h"
#include "qsingletoniterator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

class QSimpleXmlNodeModelPrivate : public QAbstractXmlNodeModelPrivate
{
 public:
   QSimpleXmlNodeModelPrivate(const QXmlNamePool &np) : namePool(np) {
   }

   mutable QXmlNamePool namePool;
};

/*!
  \class QSimpleXmlNodeModel
  \brief The QSimpleXmlNodeModel class is an implementation of QAbstractXmlNodeModel sufficient for many common cases.
  \reentrant
  \since 4.4
  \ingroup xml-tools

  Subclassing QAbstractXmlNodeModel can be a significant task, because it
  requires implementing several, complex member functions. QSimpleXmlNodeModel
  provides default implementations of these member functions that are suitable
  for a wide range of node models.

  Subclasses of QSimpleXmlNodeModel must be thread-safe.
 */

/*!
  Constructs a QSimpleXmlNodeModel for use with with the specified
  \a namePool.
 */
QSimpleXmlNodeModel::QSimpleXmlNodeModel(const QXmlNamePool &namePool)
   : QAbstractXmlNodeModel(new QSimpleXmlNodeModelPrivate(namePool))
{
}

/*!
  Destructor.
 */
QSimpleXmlNodeModel::~QSimpleXmlNodeModel()
{
}

/*!
 If \a node is an element or attribute, typedValue() is called, and
 the return value converted to a string, as per XQuery's rules.

 If \a node is another type of node, the empty string is returned.

 If this function is overridden for comments or processing
 instructions, it is important to remember to call it (for elements
 and attributes having values not of type \c xs:string) to ensure that
 the values are formatted according to XQuery.

 */
QString QSimpleXmlNodeModel::stringValue(const QXmlNodeModelIndex &node) const
{
   const QXmlNodeModelIndex::NodeKind k = kind(node);
   if (k == QXmlNodeModelIndex::Element || k == QXmlNodeModelIndex::Attribute) {
      const QVariant &candidate = typedValue(node);
      if (candidate.isNull()) {
         return QString();
      } else {
         return AtomicValue::toXDM(candidate).stringValue();
      }
   } else {
      return QString();
   }
}

/*!
  Returns the base URI for \a node. This is always the document
  URI.

  \sa documentUri()
 */
QUrl QSimpleXmlNodeModel::baseUri(const QXmlNodeModelIndex &node) const
{
   return documentUri(node);
}

/*!
  Returns the name pool associated with this model. The
  implementation of name() will use this name pool to create
  names.
 */
QXmlNamePool &QSimpleXmlNodeModel::namePool() const
{
   Q_D(const QSimpleXmlNodeModel);

   return d->namePool;
}

/*!
  Always returns an empty QVector. This signals that no namespace
  bindings are in scope for \a node.
 */
QVector<QXmlName> QSimpleXmlNodeModel::namespaceBindings(const QXmlNodeModelIndex &node) const
{
   Q_UNUSED(node);
   return QVector<QXmlName>();
}

/*!
  Always returns a default constructed QXmlNodeModelIndex instance,
  regardless of \a id.

  This effectively means the model has no elements that have an id.
 */
QXmlNodeModelIndex QSimpleXmlNodeModel::elementById(const QXmlName &id) const
{
   Q_UNUSED(id);
   return QXmlNodeModelIndex();
}

/*!
  Always returns an empty vector, regardless of \a idref.

  This effectively means the model has no elements or attributes of
  type \c IDREF.
 */
QVector<QXmlNodeModelIndex> QSimpleXmlNodeModel::nodesByIdref(const QXmlName &idref) const
{
   Q_UNUSED(idref);
   return QVector<QXmlNodeModelIndex>();
}

QT_END_NAMESPACE


