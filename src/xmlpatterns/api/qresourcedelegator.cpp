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

#include "qresourcedelegator_p.h"

using namespace QPatternist;

bool ResourceDelegator::isUnparsedTextAvailable(const QUrl &uri,
      const QString &encoding)
{
   return m_parentLoader->isUnparsedTextAvailable(uri, encoding);
}

ItemType::Ptr ResourceDelegator::announceUnparsedText(const QUrl &uri)
{
   return m_parentLoader->announceUnparsedText(uri);
}

Item ResourceDelegator::openUnparsedText(const QUrl &uri,
      const QString &encoding,
      const ReportContext::Ptr &context,
      const SourceLocationReflection *const where)
{
   return m_parentLoader->openUnparsedText(uri, encoding, context, where);
}

Item ResourceDelegator::openDocument(const QUrl &uri,
                                     const ReportContext::Ptr &context)
{
   if (m_needsOverride.contains(uri)) {
      return m_forDeviceLoader->openDocument(uri, context);
   } else {
      return m_parentLoader->openDocument(uri, context);
   }
}

SequenceType::Ptr ResourceDelegator::announceDocument(const QUrl &uri, const Usage usageHint)
{
   return m_parentLoader->announceDocument(uri, usageHint);
}

bool ResourceDelegator::isDocumentAvailable(const QUrl &uri)
{
   return m_parentLoader->isDocumentAvailable(uri);
}

Item::Iterator::Ptr ResourceDelegator::openCollection(const QUrl &uri)
{
   return m_parentLoader->openCollection(uri);
}

SequenceType::Ptr ResourceDelegator::announceCollection(const QUrl &uri)
{
   return m_parentLoader->announceCollection(uri);
}

QSet<QUrl> ResourceDelegator::deviceURIs() const
{
   QSet<QUrl> uris(m_needsOverride);
   uris |= m_forDeviceLoader->deviceURIs();
   return uris;
}
