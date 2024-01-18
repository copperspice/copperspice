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

/*
 * QXmlName is conceptually identical to QPatternist::QName. The
 * difference is that the latter is elegant, powerful and fast.
 *
 * However, it is too powerful and too open and not at all designed
 * for being public. QXmlName, in contrast, is only a public marker,
 * that for instance uses a qint64 instead of qint32, such that we in
 * the future can use that, if needed.
 */

#include <qxmlname.h>
#include <qxmlnamepool.h>

#include <qnamepool_p.h>
#include <qxpathhelper_p.h>
#include <qxmlutils_p.h>

QXmlName::QXmlName(QXmlNamePool &namePool, const QString &localName, const QString &namespaceURI,
      const QString &prefix)
{
   Q_ASSERT_X(prefix.isEmpty() || QXmlUtils::isNCName(prefix), Q_FUNC_INFO,
              "The prefix is invalid, maybe the arguments were mixed up?");

   Q_ASSERT_X(QXmlUtils::isNCName(localName), Q_FUNC_INFO,
              "The local name is invalid, maybe the arguments were mixed up?");

   m_qNameCode = namePool.d->allocateQName(namespaceURI, localName, prefix).code();
}

bool QXmlName::isNull() const
{
   return m_qNameCode == InvalidCode;
}

QXmlName::QXmlName() : m_qNameCode(InvalidCode)
{
}

bool QXmlName::operator==(const QXmlName &other) const
{
   return (m_qNameCode & ExpandedNameMask) == (other.m_qNameCode & ExpandedNameMask);
}

bool QXmlName::operator!=(const QXmlName &other) const
{
   return !operator==(other);
}

uint qHash(const QXmlName &name)
{
   return name.m_qNameCode & QXmlName::ExpandedNameMask;
}

QString QXmlName::namespaceUri(const QXmlNamePool &namePool) const
{
   if (isNull()) {
      return QString();
   } else {
      return namePool.d->stringForNamespace(namespaceURI());
   }
}

QString QXmlName::prefix(const QXmlNamePool &namePool) const
{
   if (isNull()) {
      return QString();
   } else {
      return namePool.d->stringForPrefix(prefix());
   }
}

QString QXmlName::localName(const QXmlNamePool &namePool) const
{
   if (isNull()) {
      return QString();
   } else {
      return namePool.d->stringForLocalName(localName());
   }
}

QString QXmlName::toClarkName(const QXmlNamePool &namePool) const
{
   return namePool.d->toClarkName(*this);
}

bool QXmlName::isNCName(const QString &candidate)
{
   return QXmlUtils::isNCName(candidate);
}

QXmlName QXmlName::fromClarkName(const QString &clarkName, const QXmlNamePool &namePool)
{
   return namePool.d->fromClarkName(clarkName);
}
