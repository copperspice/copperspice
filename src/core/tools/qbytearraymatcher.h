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

#ifndef QBYTEARRAYMATCHER_H
#define QBYTEARRAYMATCHER_H

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QByteArrayMatcherPrivate;

class Q_CORE_EXPORT QByteArrayMatcher
{
 public:
   QByteArrayMatcher();
   explicit QByteArrayMatcher(const QByteArray &pattern);
   explicit QByteArrayMatcher(const char *pattern, int length);
   QByteArrayMatcher(const QByteArrayMatcher &other);
   ~QByteArrayMatcher();

   QByteArrayMatcher &operator=(const QByteArrayMatcher &other);

   void setPattern(const QByteArray &pattern);

   int indexIn(const QByteArray &ba, int from = 0) const;
   int indexIn(const char *str, int len, int from = 0) const;

   inline QByteArray pattern() const {
      if (q_pattern.isNull()) {
         return QByteArray(reinterpret_cast<const char *>(p.p), p.l);
      }
      return q_pattern;
   }

 private:
   QByteArrayMatcherPrivate *d;
   QByteArray q_pattern;

   struct Data {
      uchar q_skiptable[256];
      const uchar *p;
      int l;
   };
   union {
      uint dummy[256];
      Data p;
   };
};

QT_END_NAMESPACE

#endif // QBYTEARRAYMATCHER_H
