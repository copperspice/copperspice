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
#include <qvector.h>
#include <qxmlnamepool.h>

#include "qabstractxmlnodemodel_p.h"
#include "qemptyiterator_p.h"
#include "qitemmappingiterator_p.h"
#include "qsequencemappingiterator_p.h"
#include "qsimplexmlnodemodel.h"
#include "qsingletoniterator_p.h"

using namespace QPatternist;

class QSimpleXmlNodeModelPrivate : public QAbstractXmlNodeModelPrivate
{
 public:
   QSimpleXmlNodeModelPrivate(const QXmlNamePool &np) : namePool(np) {
   }

   mutable QXmlNamePool namePool;
};

QSimpleXmlNodeModel::QSimpleXmlNodeModel(const QXmlNamePool &namePool)
   : QAbstractXmlNodeModel(new QSimpleXmlNodeModelPrivate(namePool))
{
}

QSimpleXmlNodeModel::~QSimpleXmlNodeModel()
{
}

QString QSimpleXmlNodeModel::stringValue(const QXmlNodeModelIndex &node) const
{
   const QXmlNodeModelIndex::NodeKind k = kind(node);

   if (k == QXmlNodeModelIndex::Element || k == QXmlNodeModelIndex::Attribute) {
      const QVariant &candidate = typedValue(node);

      if (! candidate.isValid()) {
         return QString();
      } else {
         return AtomicValue::toXDM(candidate).stringValue();
      }

   } else {
      return QString();
   }
}

QUrl QSimpleXmlNodeModel::baseUri(const QXmlNodeModelIndex &node) const
{
   return documentUri(node);
}

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
   (void) node;
   return QVector<QXmlName>();
}

/*!
  Always returns a default constructed QXmlNodeModelIndex instance,
  regardless of \a id.

  This effectively means the model has no elements that have an id.
 */
QXmlNodeModelIndex QSimpleXmlNodeModel::elementById(const QXmlName &id) const
{
   (void) id;
   return QXmlNodeModelIndex();
}

/*!
  Always returns an empty vector, regardless of \a idref.

  This effectively means the model has no elements or attributes of
  type \c IDREF.
 */
QVector<QXmlNodeModelIndex> QSimpleXmlNodeModel::nodesByIdref(const QXmlName &idref) const
{
   (void) idref;
   return QVector<QXmlNodeModelIndex>();
}

