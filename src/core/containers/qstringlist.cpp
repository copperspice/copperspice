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

#include <qstringlist.h>

#include <qdatastream.h>
#include <qset.h>
#include <qregularexpression.h>

#include <algorithm>

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

QStringList &QStringList::replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
   for (int i = 0; i < this->size(); ++i) {
      (*this)[i].replace(before, after, cs);
   }

   return *this;
}

QString QStringList::join(const QString &sep) const
{
   QString retval;

   if (this->isEmpty()) {
      return retval;
   }

   for (int i = 0; i < this->size(); ++i) {
      if (i) {
         retval += sep;
      }

      retval += this->at(i);
   }

   return retval;
}

void QStringList::sort()
{
   std::sort(this->begin(), this->end());
}

QStringList QStringList::filter(const QRegularExpression8 &rx) const
{
   QStringList res;

   for (int i = 0; i < this->size(); ++i) {
      if (this->at(i).contains(rx)) {
         res << this->at(i);
      }
   }

   return res;
}

QStringList &QStringList::replaceInStrings(const QRegularExpression8 &rx, const QString &after)
{
   for (int i = 0; i < this->size(); ++i) {
      (*this)[i].replace(rx, after);
   }

   return *this;
}

int QStringList::indexOf(const QRegularExpression8 &regExp, int from) const
{
   if (from < 0) {
      from = qMax(from + this->size(), 0);
   }

   QPatternOptionFlags options = QPatternOption::ExactMatchOption | regExp.patternOptions();
   QRegularExpression8 re(regExp.pattern(), options);

   for (int i = from; i < this->size(); ++i) {

      if (re.match(this->at(i)).hasMatch()) {
         return i;
      }
   }

   return -1;
}

int QStringList::lastIndexOf( const QRegularExpression8 &regExp, int from) const
{
   if (from < 0) {
      from += this->size();

   } else if (from >= this->size()) {
      from = this->size() - 1;
   }

   QPatternOptionFlags options = QPatternOption::ExactMatchOption | regExp.patternOptions();
   QRegularExpression8 re(regExp.pattern(), options);

   for (int i = from; i >= 0; --i) {
      if (re.match(this->at(i)).hasMatch()) {
         return i;
      }
   }

   return -1;
}

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

QDataStream &operator>>(QDataStream &stream, QStringList &list)
{
   return operator>>(stream, static_cast<QList<QString> &>(list));
}

QDataStream &operator<<(QDataStream &stream, const QStringList &list)
{
   return operator<<(stream, static_cast<const QList<QString> &>(list));
}
