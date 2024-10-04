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

#ifndef QXsdIdcHelper_P_H
#define QXsdIdcHelper_P_H

#include <qreportcontext_p.h>
#include <qschematype_p.h>
#include <QXmlItem>

namespace QPatternist {
class FieldNode
{
 public:
   FieldNode();

   FieldNode(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type);

   bool isEmpty() const;

   bool isEqualTo(const FieldNode &other, const NamePool::Ptr &namePool, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const reflection) const;

   QXmlItem item() const;

 private:
   QXmlItem m_item;
   QString m_data;
   SchemaType::Ptr m_type;
};

class TargetNode
{
 public:
   typedef QSet<TargetNode> Set;

   explicit TargetNode(const QXmlItem &item);

   QXmlItem item() const;

   QVector<QXmlItem> fieldItems() const;

   int emptyFieldsCount() const;

   bool fieldsAreEqual(const TargetNode &other, const NamePool::Ptr &namePool, const ReportContext::Ptr &context,
                       const SourceLocationReflection *const reflection) const;

   void addField(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type);

   bool operator==(const TargetNode &other) const;

 private:
   QXmlItem m_item;
   QVector<FieldNode> m_fields;
};

inline uint qHash(const QPatternist::TargetNode &node)
{
   return qHash(node.item().toNodeModelIndex());
}

}

#endif
