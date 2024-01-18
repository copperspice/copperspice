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

#ifndef QNamespaceSupport_P_H
#define QNamespaceSupport_P_H

#include <qnamepool_p.h>
#include <QExplicitlySharedDataPointer>
#include <QHash>
#include <QSet>
#include <QStack>
#include <QXmlStreamNamespaceDeclarations>

namespace QPatternist {

class NamespaceSupport
{
 public:
   /**
    * Describes whether the name to process is an attribute or element.
    */
   enum NameType {
      AttributeName,  ///< An attribute name to process.
      ElementName     ///< An element name to process.
   };

   /**
    * Creates an empty namespace support object.
    */
   NamespaceSupport();

   /**
    * Creates a new namespace support object.
    *
    * @param namePool The name pool where all processed names are stored in.
    */
   NamespaceSupport(NamePool &namePool);

   /**
    * Adds a new prefix-to-namespace binding.
    *
    * @param prefixCode The name pool code for the prefix.
    * @param namespaceCode The name pool code for the namespace.
    */
   void setPrefix(const QXmlName::PrefixCode prefixCode, const QXmlName::NamespaceCode namespaceCode);

   /**
    * Adds the prefix-to-namespace bindings from @p declarations to
    * the namespace support.
    */
   void setPrefixes(const QXmlStreamNamespaceDeclarations &declarations);

   /**
    * Sets the name pool code of the target namespace of the schema the
    * namespace support works on.
    */
   void setTargetNamespace(const QXmlName::NamespaceCode code);

   /**
    * Returns the prefix code for the given namespace @p code.
    */
   QXmlName::PrefixCode prefix(const QXmlName::NamespaceCode code) const;

   /**
    * Returns the namespace code for the given prefix @p code.
    */
   QXmlName::NamespaceCode uri(const QXmlName::PrefixCode code) const;

   /**
    * Converts the given @p qualifiedName to a resolved QXmlName @p name according
    * to the current namespace mapping.
    *
    * @param qualifiedName The full qualified name.
    * @param type The type of name processing.
    * @param name The resolved QXmlName.
    *
    * @returns @c true if the name could be processed correctly or @c false if the
    *          namespace prefix is unknown.
    */
   bool processName(const QString &qualifiedName, NameType type, QXmlName &name) const;

   /**
    * Pushes the current namespace mapping onto the stack.
    */
   void pushContext();

   /**
    * Pops the current namespace mapping from the stack.
    */
   void popContext();

   /**
    * Returns the list of namespace bindings.
    */
   QList<QXmlName> namespaceBindings() const;

 private:
   typedef QHash<QXmlName::PrefixCode, QXmlName::NamespaceCode> NamespaceHash;

   NamePool              *m_namePool;
   QStack<NamespaceHash> m_nsStack;
   NamespaceHash         m_ns;
};

}

#endif
