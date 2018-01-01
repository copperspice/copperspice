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

#include <QtCore/qglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qpolygon.h>
#include <QtCore/qstringbuilder.h>

#ifndef QHEXSTRING_P_H
#define QHEXSTRING_P_H

QT_BEGIN_NAMESPACE

// internal helper. Converts an integer value to an unique string token
template <typename T>
struct HexString {
   inline HexString(const T t)
      : val(t) {
   }

   inline void write(QChar *&dest) const {
      const ushort hexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
      const char *c = reinterpret_cast<const char *>(&val);
      for (uint i = 0; i < sizeof(T); ++i) {
         *dest++ = hexChars[*c & 0xf];
         *dest++ = hexChars[(*c & 0xf0) >> 4];
         ++c;
      }
   }
   const T val;
};

// specialization to enable fast concatenating of our string tokens to a string
template <typename T>
struct QConcatenable<HexString<T> > {
   typedef HexString<T> type;
   enum { ExactSize = true };
   static int size(const HexString<T> &) {
      return sizeof(T) * 2;
   }
   static inline void appendTo(const HexString<T> &str, QChar *&out) {
      str.write(out);
   }
   typedef QString ConvertTo;
};

QT_END_NAMESPACE

#endif // QHEXSTRING_P_H
