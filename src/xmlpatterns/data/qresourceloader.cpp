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

#include <QUrl>

#include <qresourceloader_p.h>

using namespace QPatternist;

ResourceLoader::~ResourceLoader()
{
}

bool ResourceLoader::isUnparsedTextAvailable(const QUrl &uri,
      const QString &encoding)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;
   (void) encoding;

   return false;
}

ItemType::Ptr ResourceLoader::announceUnparsedText(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;

   return ItemType::Ptr();
}

Item ResourceLoader::openUnparsedText(const QUrl &uri,
                                      const QString &encoding,
                                      const ReportContext::Ptr &context,
                                      const SourceLocationReflection *const where)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;
   (void) encoding;
   (void) context;
   (void) where;

   return Item();
}

Item ResourceLoader::openDocument(const QUrl &uri, const ReportContext::Ptr &context)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;
   (void) context;

   return Item();
}

SequenceType::Ptr ResourceLoader::announceDocument(const QUrl &uri, const Usage)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;

   return SequenceType::Ptr();
}

bool ResourceLoader::isDocumentAvailable(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;

   return false;
}

Item::Iterator::Ptr ResourceLoader::openCollection(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;

   return Item::Iterator::Ptr();
}

SequenceType::Ptr ResourceLoader::announceCollection(const QUrl &uri)
{
   Q_ASSERT(uri.isValid());
   Q_ASSERT(!uri.isRelative());

   (void) uri;

   return SequenceType::Ptr();
}

void ResourceLoader::clear(const QUrl &uri)
{
   (void) uri;
}

