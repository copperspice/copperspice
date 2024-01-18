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

#ifndef QSIMPLEXMLNODEMODEL_H
#define QSIMPLEXMLNODEMODEL_H

#include <qabstractxmlnodemodel.h>
#include <qxmlquery.h>

template<typename T> class QExplicitlySharedDataPointer;
class QSimpleXmlNodeModelPrivate;
class Q_XMLPATTERNS_EXPORT QSimpleXmlNodeModel : public QAbstractXmlNodeModel
{
 public:
   QSimpleXmlNodeModel(const QXmlNamePool &namePool);
   virtual ~QSimpleXmlNodeModel();

   QUrl baseUri(const QXmlNodeModelIndex &node) const override;
   QXmlNamePool &namePool() const;
   QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &node) const override;
   QString stringValue(const QXmlNodeModelIndex &node) const override;
   QXmlNodeModelIndex elementById(const QXmlName &id) const override;
   QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &idref) const override;

 private:
   Q_DECLARE_PRIVATE(QSimpleXmlNodeModel)
};

#endif
