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

#ifndef QXMLNAME_H
#define QXMLNAME_H

#include <qstring.h>

class QXmlName;
class QXmlNamePool;

Q_XMLPATTERNS_EXPORT uint qHash(const QXmlName &name);

class Q_XMLPATTERNS_EXPORT QXmlName
{
 public:
   typedef qint16 NamespaceCode;
   typedef NamespaceCode PrefixCode;
   typedef NamespaceCode LocalNameCode;

   QXmlName();

   QXmlName(QXmlNamePool &namePool, const QString &localName, const QString &namespaceURI = QString(),
            const QString &prefix = QString());

   QXmlName(const QXmlName &other) = default;
   QXmlName &operator=(const QXmlName &other) = default;

   QString namespaceUri(const QXmlNamePool &namePool) const;
   QString prefix(const QXmlNamePool &namePool) const;
   QString localName(const QXmlNamePool &namePool) const;
   QString toClarkName(const QXmlNamePool &namePool) const;

   bool operator==(const QXmlName &other) const;
   bool operator!=(const QXmlName &other) const;
   bool isNull() const;

   static bool isNCName(const QString &candidate);
   static QXmlName fromClarkName(const QString &clarkName, const QXmlNamePool &namePool);

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them is undefined behavior. */
   typedef qint64 Code;

   inline QXmlName(const NamespaceCode uri, const LocalNameCode ln, const PrefixCode p = 0);

   /* implementations are in utils/qnamepool_p.h. */
   inline LocalNameCode localName() const;
   inline PrefixCode prefix() const;
   inline bool hasPrefix() const;
   inline bool hasNamespace() const;
   inline NamespaceCode namespaceURI() const;
   inline bool isLexicallyEqual(const QXmlName &other) const;
   inline void setPrefix(const PrefixCode c);
   inline void setNamespaceURI(const NamespaceCode c);
   inline void setLocalName(const LocalNameCode c);
   inline Code code() const;

   friend Q_XMLPATTERNS_EXPORT uint qHash(const QXmlName &name);

 private:

  enum Constant {
      LocalNameOffset     = 0,
      LocalNameLength     = 12,
      NamespaceOffset     = LocalNameLength,
      NamespaceLength     = 9,
      PrefixLength        = 9,
      InvalidCode         = 1 << 31,
      NamespaceMask       = ((1 << ((NamespaceOffset + NamespaceLength) - NamespaceOffset)) - 1) << NamespaceOffset,
      LocalNameMask       = ((1 << ((LocalNameOffset + LocalNameLength) - LocalNameOffset)) - 1) << LocalNameOffset,
      PrefixOffset        = LocalNameLength + NamespaceLength,
      PrefixMask          = ((1 << ((PrefixOffset + PrefixLength) - PrefixOffset)) - 1) << PrefixOffset,
      MaximumPrefixes     = (PrefixMask >> PrefixOffset) - 1,
      MaximumLocalNames   = (LocalNameMask >> LocalNameOffset) - 1,
      MaximumNamespaces   = (NamespaceMask >> NamespaceOffset) - 1,
      ExpandedNameMask    = LocalNameMask | NamespaceMask,
      LexicalQNameMask    = LocalNameMask | PrefixMask
   };

   inline QXmlName(const int c)
      : m_qNameCode(c)
   {
   }

   Code m_qNameCode;
};

CS_DECLARE_METATYPE(QXmlName)

#endif
