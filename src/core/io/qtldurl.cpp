/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qplatformdefs.h>
#include <qurl.h>
#include <qurltlds_p.h>
#include <qtldurl_p.h>
#include <qstringlist.h>

QT_BEGIN_NAMESPACE

static bool containsTLDEntry(const QString &entry)
{
   int index = qHash(entry) % tldCount;
   int currentDomainIndex = tldIndices[index];
   while (currentDomainIndex < tldIndices[index + 1]) {
      QString currentEntry = QString::fromUtf8(tldData + currentDomainIndex);
      if (currentEntry == entry) {
         return true;
      }
      currentDomainIndex += qstrlen(tldData + currentDomainIndex) + 1; // +1 for the ending \0
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
   QStringList sections = domain.toLower().split(QLatin1Char('.'), QString::SkipEmptyParts);
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

   if (domain.contains(QLatin1Char('.'))) {
      int count = domain.size() - domain.indexOf(QLatin1Char('.'));
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

QT_END_NAMESPACE
