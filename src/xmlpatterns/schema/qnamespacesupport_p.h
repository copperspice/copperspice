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
   enum NameType {
      AttributeName,      // An attribute name to process.
      ElementName         // An element name to process.
   };

   NamespaceSupport();

   NamespaceSupport(NamePool &namePool);

   void setPrefix(const QXmlName::PrefixCode prefixCode, const QXmlName::NamespaceCode namespaceCode);
   void setPrefixes(const QXmlStreamNamespaceDeclarations &declarations);
   void setTargetNamespace(const QXmlName::NamespaceCode code);

   QXmlName::PrefixCode prefix(const QXmlName::NamespaceCode code) const;
   QXmlName::NamespaceCode uri(const QXmlName::PrefixCode code) const;

   bool processName(const QString &qualifiedName, NameType type, QXmlName &name) const;
   void pushContext();
   void popContext();


   QList<QXmlName> namespaceBindings() const;

 private:
   typedef QHash<QXmlName::PrefixCode, QXmlName::NamespaceCode> NamespaceHash;

   NamePool *m_namePool;
   QStack<NamespaceHash> m_nsStack;
   NamespaceHash m_ns;
};

}

#endif
