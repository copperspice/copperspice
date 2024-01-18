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
   /**
    * Creates an empty field node.
    */
   FieldNode();

   /**
    * Creates a field node that is bound to a xml node.
    *
    * @param item The xml node the field is bound to.
    * @param data The string content of that field.
    * @param type The type that is bound to that field.
    */
   FieldNode(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type);

   /**
    * Returns whether this field is empty.
    *
    * A field can be empty, if the xpath expression selects an absent attribute
    * or element.
    */
   bool isEmpty() const;

   /**
    * Returns whether this field is equal to the @p other field.
    *
    * Equal means that both have the same type and there content is equal in the
    * types value space.
    */
   bool isEqualTo(const FieldNode &other, const NamePool::Ptr &namePool, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const reflection) const;

   /**
    * Returns the xml node item the field is bound to.
    */
   QXmlItem item() const;

 private:
   QXmlItem m_item;
   QString m_data;
   SchemaType::Ptr m_type;
};

/**
 * @short A helper class for validating identity constraints.
 *
 * This class represents a target or qualified node from the target or qualified
 * node set as defined in the validation rules at http://www.w3.org/TR/xmlschema11-1/#d0e32243.
 *
 * A target node is part of the qualified node set, if all of its fields are not empty.
 */
class TargetNode
{
 public:
   /**
    * Defines a set of target nodes.
    */
   typedef QSet<TargetNode> Set;

   /**
    * Creates a new target node that is bound to the xml node @p item.
    */
   explicit TargetNode(const QXmlItem &item);

   /**
    * Returns the xml node item the target node is bound to.
    */
   QXmlItem item() const;

   /**
    * Returns all xml node items, the fields of that target node are bound to.
    */
   QVector<QXmlItem> fieldItems() const;

   /**
    * Returns the number of fields that are empty.
    */
   int emptyFieldsCount() const;

   /**
    * Returns whether the target node has the same fields as the @p other target node.
    */
   bool fieldsAreEqual(const TargetNode &other, const NamePool::Ptr &namePool, const ReportContext::Ptr &context,
                       const SourceLocationReflection *const reflection) const;

   /**
    * Adds a new field to the target node with the given values.
    */
   void addField(const QXmlItem &item, const QString &data, const SchemaType::Ptr &type);

   /**
    * Returns whether the target node is equal to the @p other target node.
    */
   bool operator==(const TargetNode &other) const;

 private:
   QXmlItem m_item;
   QVector<FieldNode> m_fields;
};

/**
 * Creates a hash value for the given target @p node.
 */
inline uint qHash(const QPatternist::TargetNode &node)
{
   return qHash(node.item().toNodeModelIndex());
}

}

#endif
