/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QUrl>


#include "qresourceloader_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ResourceLoader::~ResourceLoader()
{
}

bool ResourceLoader::isUnparsedTextAvailable(const QUrl &uri,
      const QString &encoding)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   Q_UNUSED(encoding);
   return false;
}

ItemType::Ptr ResourceLoader::announceUnparsedText(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   return ItemType::Ptr();
}

Item ResourceLoader::openUnparsedText(const QUrl &uri,
                                      const QString &encoding,
                                      const ReportContext::Ptr &context,
                                      const SourceLocationReflection *const where)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   Q_UNUSED(encoding);
   Q_UNUSED(context);
   Q_UNUSED(where);
   return Item();
}

Item ResourceLoader::openDocument(const QUrl &uri,
                                  const ReportContext::Ptr &context)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   Q_UNUSED(context); /* Needed when compiling in release mode. */
   return Item();
}

SequenceType::Ptr ResourceLoader::announceDocument(const QUrl &uri, const Usage)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   return SequenceType::Ptr();
}

bool ResourceLoader::isDocumentAvailable(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   return false;
}

Item::Iterator::Ptr ResourceLoader::openCollection(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   return Item::Iterator::Ptr();
}

SequenceType::Ptr ResourceLoader::announceCollection(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());
   Q_UNUSED(uri); /* Needed when compiling in release mode. */
   return SequenceType::Ptr();
}

void ResourceLoader::clear(const QUrl &uri)
{
   Q_UNUSED(uri);
}

QT_END_NAMESPACE
