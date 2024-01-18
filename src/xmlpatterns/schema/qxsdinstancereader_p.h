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

   /**
    * Creates a new instance reader that will read the data from
    * the given @p model.
    *
    * @param model The model the data are read from.
    * @param context The context that is used for error reporting etc.
    */
   XsdInstanceReader(const QAbstractXmlNodeModel *model, const XsdSchemaContext::Ptr &context);

 protected:
   /**
    * Returns @c true if the end of the document is reached, @c false otherwise.
    */
   bool atEnd() const;

   /**
    * Reads the next node from the document.
    */
   void readNext();

   /**
    * Returns whether the current node is a start element.
    */
   bool isStartElement() const;

   /**
    * Returns whether the current node is an end element.
    */
   bool isEndElement() const;

   /**
    * Returns whether the current node has a text node among its children.
    */
   bool hasChildText() const;

   /**
    * Returns whether the current node has an element node among its children.
    */
   bool hasChildElement() const;

   /**
    * Returns the name of the current node.
    */
   QXmlName name() const;

   /**
    * Returns whether the current node has an attribute with the given @p name.
    */
   bool hasAttribute(const QXmlName &name) const;

   /**
    * Returns the attribute with the given @p name of the current node.
    */
   QString attribute(const QXmlName &name) const;

   /**
    * Returns the list of attribute names of the current node.
    */
   QSet<QXmlName> attributeNames() const;

   /**
    * Returns the concatenated text of all direct child text nodes.
    */
   QString text() const;

   /**
    * Converts a qualified name into a QXmlName according to the namespace
    * mappings of the current node.
    */
   QXmlName convertToQName(const QString &name) const;

   /**
    * Returns a source location object for the current position.
    */
   QSourceLocation sourceLocation() const;

   /**
    * Returns the QXmlItem for the current position.
    */
   QXmlItem item() const;

   /**
    * Returns the QXmlItem for the attribute with the given @p name at the current position.
    */
   QXmlItem attributeItem(const QXmlName &name) const;

   /**
    * Returns the namespace bindings for the given node model @p index.
    */
   QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &index) const;

   /**
    * The shared schema context.
    */
   XsdSchemaContext::Ptr     m_context;

 private:
   PullBridge                m_model;
   QHash<QXmlName, QString>  m_cachedAttributes;
   QHash<QXmlName, QXmlItem> m_cachedAttributeItems;
   QSourceLocation           m_cachedSourceLocation;
   QXmlItem                  m_cachedItem;
};

}

#endif
