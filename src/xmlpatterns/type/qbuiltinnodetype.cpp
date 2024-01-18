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

template <const QXmlNodeModelIndex::NodeKind kind>
BuiltinNodeType<kind>::BuiltinNodeType()
{
}

template <const QXmlNodeModelIndex::NodeKind kind>
bool BuiltinNodeType<kind>::xdtTypeMatches(const ItemType::Ptr &other) const
{
   if (!other->isNodeType()) {
      return false;
   }

   return *static_cast<const BuiltinNodeType *>(other.data()) == *this
          ? true
          : xdtTypeMatches(other->xdtSuperType());
}

template <const QXmlNodeModelIndex::NodeKind kind>
bool BuiltinNodeType<kind>::itemMatches(const Item &item) const
{
   Q_ASSERT(item);

   return item.isNode() &&
          item.asNode().kind() == kind;
}

template <const QXmlNodeModelIndex::NodeKind kind>
ItemType::Ptr BuiltinNodeType<kind>::atomizedType() const
{
   switch (kind) {
      case QXmlNodeModelIndex::Attribute:
      case QXmlNodeModelIndex::Document:
      case QXmlNodeModelIndex::Element:
      case QXmlNodeModelIndex::Text:
         return BuiltinTypes::xsUntypedAtomic;

      case QXmlNodeModelIndex::ProcessingInstruction:
      case QXmlNodeModelIndex::Comment:
         return BuiltinTypes::xsString;

      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Encountered invalid XPath Data Model node type.");
         return BuiltinTypes::xsUntypedAtomic;
      }
   }
}

template <const QXmlNodeModelIndex::NodeKind kind>
QString BuiltinNodeType<kind>::displayName(const NamePool::Ptr &) const
{
   switch (kind) {
      case QXmlNodeModelIndex::Element:
         return QLatin1String("element()");
      case QXmlNodeModelIndex::Document:
         return QLatin1String("document()");
      case QXmlNodeModelIndex::Attribute:
         return QLatin1String("attribute()");
      case QXmlNodeModelIndex::Text:
         return QLatin1String("text()");
      case QXmlNodeModelIndex::ProcessingInstruction:
         return QLatin1String("processing-instruction()");
      case QXmlNodeModelIndex::Comment:
         return QLatin1String("comment()");
      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Encountered invalid XPath Data Model node type.");
         return QString();
      }
   }
}

template <const QXmlNodeModelIndex::NodeKind kind>
ItemType::Ptr BuiltinNodeType<kind>::xdtSuperType() const
{
   return BuiltinTypes::node;
}

template <const QXmlNodeModelIndex::NodeKind kind>
QXmlNodeModelIndex::NodeKind BuiltinNodeType<kind>::nodeKind() const
{
   return kind;
}

template <const QXmlNodeModelIndex::NodeKind kind>
PatternPriority BuiltinNodeType<kind>::patternPriority() const
{
   /* See XSL Transformations (XSLT) Version 2.0, 6.4 Conflict Resolution for
    * Template Rules */

   switch (kind) {
      case QXmlNodeModelIndex::Text:
      case QXmlNodeModelIndex::ProcessingInstruction:
      case QXmlNodeModelIndex::Comment:
      case QXmlNodeModelIndex::Attribute:
      case QXmlNodeModelIndex::Element:
      case QXmlNodeModelIndex::Document:
         return -0.5;

      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown node type");
         return 0;
      }
   }

}

