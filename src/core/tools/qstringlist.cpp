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

#include <algorithm>
#include <qstringlist.h>
#include <qset.h>

QT_BEGIN_NAMESPACE

bool QStringList::contains(const QString &str, Qt::CaseSensitivity cs) const
{
   for (int i = 0; i < this->size(); ++i) {
      const QString &string = this->at(i);

      if (string.length() == str.length() && str.compare(string, cs) == 0) {
         return true;
      }
   }

   return false;
}

QStringList QStringList::filter(const QString &str, Qt::CaseSensitivity cs) const
{
   QStringList res;

   for (int i = 0; i < this->size(); ++i) {

      if (this->at(i).indexOf(str, cs) != -1) {
         res << this->at(i);
      }
   }

   return res;
}

QStringList & QStringList::replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
   for (int i = 0; i < this->size(); ++i) {
      (*this)[i].replace(before, after, cs);
   }

   return *this;
}

QString QStringList::join(const QString &sep) const
{
   int totalLength = 0;
   const int size = this->size();

   for (int i = 0; i < size; ++i) {
      totalLength += this->at(i).size();
   }

   if (size > 0) {
      totalLength += sep.size() * (size - 1);
   }

   QString res;
   if (totalLength == 0) {
      return res;
   }

   res.reserve(totalLength);
   for (int i = 0; i < this->size(); ++i) {
      if (i) {
         res += sep;
      }

      res += this->at(i);
   }
   return res;
}

void QStringList::sort()
{
   std::sort(this->begin(), this->end());
}

#ifndef QT_NO_REGEXP

QStringList QStringList::filter(const QRegExp &rx) const
{
   QStringList res;

   for (int i = 0; i < this->size(); ++i) {
      if (this->at(i).contains(rx)) {
         res << this->at(i);
      }
   }

   return res;
}

QStringList & QStringList::replaceInStrings(const QRegExp &rx, const QString &after)
{
   for (int i = 0; i < this->size(); ++i) {
      (*this)[i].replace(rx, after);
   }

   return *this;
}

static int indexOfMutating(const QStringList *that, QRegExp &rx, int from)
{
   if (from < 0) {
      from = qMax(from + that->size(), 0);
   }

   for (int i = from; i < that->size(); ++i) {
      if (rx.exactMatch(that->at(i))) {
         return i;
      }
   }

   return -1;
}

static int lastIndexOfMutating(const QStringList *that, QRegExp &rx, int from)
{
   if (from < 0) {
      from += that->size();

   } else if (from >= that->size()) {
      from = that->size() - 1;
   }

   for (int i = from; i >= 0; --i) {
      if (rx.exactMatch(that->at(i))) {
         return i;
      }
   }

   return -1;
}

int QStringList::indexOf(const QRegExp &rx, int from) const
{
   QRegExp rx2(rx);
   return indexOfMutating(this, rx2, from);
}

int QStringList::indexOf(QRegExp &rx, int from) const
{
   return indexOfMutating(this, rx, from);
}

int QStringList::lastIndexOf( const QRegExp &rx, int from) const
{
   QRegExp rx2(rx);
   return lastIndexOfMutating(this, rx2, from);
}

int QStringList::lastIndexOf(QRegExp &rx, int from) const
{
   return lastIndexOfMutating(this, rx, from);
}
#endif

int QStringList::removeDuplicates()
{
   int n = this->size();
   int j = 0;

   QSet<QString> seen;
   seen.reserve(n);

   for (int i = 0; i < n; ++i) {
      const QString &s = this->at(i);

      if (seen.contains(s)) {
         continue;
      }

      seen.insert(s);

      if (j != i) {
         (*this)[j] = s;
      }
      ++j;
   }

   if (n != j) {
      this->erase(this->begin() + j, this->end());
   }

   return n - j;
}

QT_END_NAMESPACE
