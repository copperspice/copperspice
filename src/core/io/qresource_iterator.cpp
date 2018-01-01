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

#include <qresource.h>
#include <qresource_iterator_p.h>
#include <qvariant.h>

QT_BEGIN_NAMESPACE

QResourceFileEngineIterator::QResourceFileEngineIterator(QDir::Filters filters,
      const QStringList &filterNames)
   : QAbstractFileEngineIterator(filters, filterNames), index(-1)
{
}

QResourceFileEngineIterator::~QResourceFileEngineIterator()
{
}

QString QResourceFileEngineIterator::next()
{
   if (!hasNext()) {
      return QString();
   }
   ++index;
   return currentFilePath();
}

bool QResourceFileEngineIterator::hasNext() const
{
   if (index == -1) {
      // Lazy initialization of the iterator
      QResource resource(path());
      if (!resource.isValid()) {
         return false;
      }

      // Initialize and move to the next entry.
      entries = resource.children();
      index = 0;
   }

   return index < entries.size();
}

QString QResourceFileEngineIterator::currentFileName() const
{
   if (index <= 0 || index > entries.size()) {
      return QString();
   }
   return entries.at(index - 1);
}

QT_END_NAMESPACE
