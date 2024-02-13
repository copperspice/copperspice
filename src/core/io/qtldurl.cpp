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

#include <qtldurl_p.h>

#include <qplatformdefs.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qstringparser.h>

#include <qurltlds_p.h>

static bool containsTLDEntry(const QString &entry)
{
   int index = cs_stable_hash(entry) % tldCount;

   // select the right chunk from the big table
   short chunk     = 0;
   uint chunkIndex = tldIndices[index];
   uint offset     = 0;

   while (chunk < tldChunkCount && tldIndices[index] >= tldChunks[chunk]) {
      chunkIndex -= tldChunks[chunk];
      offset     += tldChunks[chunk];
      chunk++;
   }

   // check all the entries from the given index
   while (chunkIndex < tldIndices[index + 1] - offset) {
      QString currentEntry = QString::fromUtf8(tldData[chunk] + chunkIndex);

      if (currentEntry == entry) {
         return true;
      }

      chunkIndex += qstrlen(tldData[chunk] + chunkIndex) + 1; // +1 for the ending \0
   }

   return false;
}

Q_CORE_EXPORT QString qTopLevelDomain(const QString &domain)
{
   const QString domainLower = domain.toLower();
   QStringList sections      = domainLower.split('.', QStringParser::SkipEmptyParts);

   if (sections.isEmpty()) {
      return QString();
   }

   QString level, tld;

   for (int j = sections.count() - 1; j >= 0; --j) {
      level.prepend('.' + sections.at(j));

      if (qIsEffectiveTLD(level.right(level.size() - 1))) {
         tld = level;
      }
   }

   return tld;
}

Q_CORE_EXPORT bool qIsEffectiveTLD(const QString &domain)
{
   // for domain 'foo.bar.com':
   // 1. return if TLD table contains 'foo.bar.com'
   if (containsTLDEntry(domain)) {
      return true;
   }

   const int dot = domain.indexOf('.');

   if (dot >= 0) {
      int count = domain.size() - dot;

      QString wildCardDomain;

      wildCardDomain.append('*');
      wildCardDomain.append(domain.right(count));

      // 2. if table contains '*.bar.com',
      // test if table contains '!foo.bar.com'
      if (containsTLDEntry(wildCardDomain)) {
         QString exceptionDomain;

         exceptionDomain.append('!');
         exceptionDomain.append(domain);

         return (! containsTLDEntry(exceptionDomain));
      }
   }

   return false;
}
