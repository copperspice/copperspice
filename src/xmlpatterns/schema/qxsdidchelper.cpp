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

#include "qxsdidchelper_p.h"

#include "qderivedstring_p.h"
#include "qxsdschemahelper_p.h"

using namespace QPatternist;

FieldNode::FieldNode()
{
}

FieldNode::FieldNode(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type)
   : m_item(item)
   , m_data(data)
   , m_type(type)
{
}

bool FieldNode::isEmpty() const
{
   return m_item.isNull();
}

bool FieldNode::isEqualTo(const FieldNode &other, const NamePool::Ptr &namePool, const ReportContext::Ptr &context,
                          const SourceLocationReflection *const reflection) const
{
   if (m_type != other.m_type) {
      return false;
   }

   const DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(namePool, m_data);
   const DerivedString<TypeString>::Ptr otherString = DerivedString<TypeString>::fromLexical(namePool, other.m_data);

   return XsdSchemaHelper::constructAndCompare(string, AtomicComparator::OperatorEqual, otherString, m_type, context,
          reflection);
}

QXmlItem FieldNode::item() const
{
   return m_item;
}

TargetNode::TargetNode(const QXmlItem &item)
   : m_item(item)
{
}

QXmlItem TargetNode::item() const
{
   return m_item;
}

QVector<QXmlItem> TargetNode::fieldItems() const
{
   QVector<QXmlItem> items;

   for (int i = 0; i < m_fields.count(); ++i) {
      items.append(m_fields.at(i).item());
   }

   return items;
}

int TargetNode::emptyFieldsCount() const
{
   int counter = 0;
   for (int i = 0; i < m_fields.count(); ++i) {
      if (m_fields.at(i).isEmpty()) {
         ++counter;
      }
   }

   return counter;
}

bool TargetNode::fieldsAreEqual(const TargetNode &other, const NamePool::Ptr &namePool,
                                const ReportContext::Ptr &context, const SourceLocationReflection *const reflection) const
{
   if (m_fields.count() != other.m_fields.count()) {
      return false;
   }

   for (int i = 0; i < m_fields.count(); ++i) {
      if (!m_fields.at(i).isEqualTo(other.m_fields.at(i), namePool, context, reflection)) {
         return false;
      }
   }

   return true;
}

void TargetNode::addField(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type)
{
   m_fields.append(FieldNode(item, data, type));
}

bool TargetNode::operator==(const TargetNode &other) const
{
   return (m_item.toNodeModelIndex() == other.m_item.toNodeModelIndex());
}

