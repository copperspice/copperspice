/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QNamespaceBinding_P_H
#define QNamespaceBinding_P_H

#include <qxmlname.h>

template<typename T>
class QVector;

namespace QPatternist {

class NamespaceBinding
{
 public:
   enum {
      InvalidCode = -1
   };

   typedef QVector<NamespaceBinding> Vector;

   NamespaceBinding()
      : m_prefix(InvalidCode), m_namespace(InvalidCode)
   { }

   NamespaceBinding(const QXmlName::PrefixCode p, const QXmlName::NamespaceCode n)
      : m_prefix(p), m_namespace(n)
   { }

   bool operator==(const NamespaceBinding &other) const {
      return m_prefix == other.m_prefix && m_namespace == other.m_namespace;
   }

   QXmlName::PrefixCode prefix() const {
      return m_prefix;
   }

   QXmlName::NamespaceCode namespaceURI() const {
      return m_namespace;
   }

   bool isNull() const {
      return m_prefix == InvalidCode;
   }

   static NamespaceBinding fromQXmlName(const QXmlName qName) {
      Q_ASSERT(!qName.isNull());
      return NamespaceBinding(qName.prefix(), qName.namespaceURI());
   }

 private:
   QXmlName::PrefixCode      m_prefix;
   QXmlName::NamespaceCode   m_namespace;
};

static inline uint qHash(const NamespaceBinding nb)
{
   return (nb.prefix() << 16) + nb.namespaceURI();
}

}

#endif
