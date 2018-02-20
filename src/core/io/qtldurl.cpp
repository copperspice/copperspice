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

#include <qplatformdefs.h>
#include <qurl.h>
#include <qurltlds_p.h>
#include <qtldurl_p.h>
#include <qstringlist.h>

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
   while (chunkIndex < tldIndices[index+1] - offset) {
        QString currentEntry = QString::fromUtf8(tldData[chunk] + chunkIndex);

        if (currentEntry == entry) {
           return true;
        }

        chunkIndex += qstrlen(tldData[chunk] + chunkIndex) + 1; // +1 for the ending \0
   }

   return false;
}

/*!
    \internal

    Return the top-level-domain per Qt's copy of the Mozilla public suffix list of
    \a domain.
*/

Q_CORE_EXPORT QString qTopLevelDomain(const QString &domain)
{
   const QString domainLower = domain.toLower();
   QStringList sections = domainLower.split(QLatin1Char('.'), QString::SkipEmptyParts);

   if (sections.isEmpty()) {
      return QString();
   }

   QString level, tld;
   for (int j = sections.count() - 1; j >= 0; --j) {
      level.prepend(QLatin1Char('.') + sections.at(j));
      if (qIsEffectiveTLD(level.right(level.size() - 1))) {
         tld = level;
      }
   }

   return tld;
}

/*!
    \internal

    Return true if \a domain is a top-level-domain per Qt's copy of the Mozilla public suffix list.
*/

Q_CORE_EXPORT bool qIsEffectiveTLD(const QString &domain)
{
   // for domain 'foo.bar.com':
   // 1. return if TLD table contains 'foo.bar.com'
   if (containsTLDEntry(domain)) {
      return true;
   }

   const int dot = domain.indexOf(QLatin1Char('.'));

   if (dot >= 0) {
      int count = domain.size() - dot;
      QString wildCardDomain;

      wildCardDomain.reserve(count + 1);
      wildCardDomain.append(QLatin1Char('*'));
      wildCardDomain.append(domain.right(count));

      // 2. if table contains '*.bar.com',
      // test if table contains '!foo.bar.com'
      if (containsTLDEntry(wildCardDomain)) {
         QString exceptionDomain;
         exceptionDomain.reserve(domain.size() + 1);
         exceptionDomain.append(QLatin1Char('!'));
         exceptionDomain.append(domain);
         return (! containsTLDEntry(exceptionDomain));
      }
   }
   return false;
}

