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

#ifndef QXsdInstanceReader_P_H
#define QXsdInstanceReader_P_H

#include <qabstractxmlnodemodel.h>
#include <qpullbridge_p.h>
#include <qxsdschemacontext_p.h>

namespace QPatternist {

class XsdInstanceReader
{
 public:
   typedef QExplicitlySharedDataPointer<XsdInstanceReader> Ptr;

   XsdInstanceReader(const QAbstractXmlNodeModel *model, const XsdSchemaContext::Ptr &context);

 protected:
   bool atEnd() const;
   void readNext();
   bool isStartElement() const;
   bool isEndElement() const;

   bool hasChildText() const;
   bool hasChildElement() const;

   QXmlName name() const;
   bool hasAttribute(const QXmlName &name) const;

   QString attribute(const QXmlName &name) const;
   QSet<QXmlName> attributeNames() const;

   QString text() const;
   QXmlName convertToQName(const QString &name) const;

   QSourceLocation sourceLocation() const;
   QXmlItem item() const;
   QXmlItem attributeItem(const QXmlName &name) const;
   QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &index) const;

   XsdSchemaContext::Ptr m_context;

 private:
   PullBridge                m_model;
   QHash<QXmlName, QString>  m_cachedAttributes;
   QHash<QXmlName, QXmlItem> m_cachedAttributeItems;
   QSourceLocation           m_cachedSourceLocation;
   QXmlItem                  m_cachedItem;
};

}

#endif
