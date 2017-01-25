/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSTRINGMATCHER_H
#define QSTRINGMATCHER_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QStringMatcherPrivate;

class Q_CORE_EXPORT QStringMatcher
{
 public:
   QStringMatcher();
   QStringMatcher(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive);
   QStringMatcher(const QChar *uc, int len, Qt::CaseSensitivity cs = Qt::CaseSensitive);
   QStringMatcher(const QStringMatcher &other);
   ~QStringMatcher();

   QStringMatcher &operator=(const QStringMatcher &other);

   void setPattern(const QString &pattern);
   void setCaseSensitivity(Qt::CaseSensitivity cs);

   int indexIn(const QString &str, int from = 0) const;
   int indexIn(const QChar *str, int length, int from = 0) const;
   QString pattern() const;

   inline Qt::CaseSensitivity caseSensitivity() const {
      return q_cs;
   }

 private:
   QStringMatcherPrivate *d_ptr;
   QString q_pattern;
   Qt::CaseSensitivity q_cs;

   struct Data {
      uchar q_skiptable[256];
      const QChar *uc;
      int len;
   };
   union {
      uint q_data[256];
      Data p;
   };
};

QT_END_NAMESPACE

#endif // QSTRINGMATCHER_H
