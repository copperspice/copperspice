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

#ifndef QCompressedWhitespace_P_H
#define QCompressedWhitespace_P_H

#include <qglobal.h>
#include <qstringfwd.h>

namespace QPatternist {

class CompressedWhitespace
{
 public:
   /**
    * @short Compresses @p input into a compressed format, returned
    * as a QString.
    *
    * The caller guarantees that input is not empty
    * and consists only of whitespace.
    *
    * The returned format is opaque. There is no way to find out
    * whether a QString contains compressed data or not.
    *
    * @see decompress()
    */
   static QString compress(QStringView input);

   /**
    * @short Decompresses @p input into a usual QString.
    *
    * @p input must be a QString as per returned from compress().
    *
    * @see compress()
    */
   static QString decompress(const QString &input);

 private:
   /**
    * We use the two upper bits for communicating what space it is.
    */
   enum CharIdentifier {
      Space   = 0x0,

      /**
       * 0xA, \\r
       *
       * Binary: 10000000
       */
      CR      = 0x80,

      /**
       * 0xD, \\n
       *
       * Binary: 01000000
       */
      LF      = 0x40,

      /**
       * Binary: 11000000
       */
      Tab     = 0xC0
   };

   enum Constants {
      /* We can at maximum store this many consecutive characters
       * of one type. We use 6 bits for the count. */
      MaxCharCount = (1 << 6) - 1,

      /**
       * Binary: 11111111
       */
      Lower8Bits = (1 << 8) - 1,

      /**
       * Binary: 111111
       */
      Lower6Bits = (1 << 6) - 1,

      /*
       * Binary: 11000000
       */
      UpperTwoBits = 3 << 6
   };

   static inline CharIdentifier toIdentifier(const QChar ch);

   static inline quint8 toCompressedChar(const QChar ch, const int len);
   static inline QChar toChar(const CharIdentifier id);

   /**
    * @short Returns @c true if @p number is an even number, otherwise
    * @c false.
    */
   static inline bool isEven(const int number);

   /**
    * @short This class can only be used via its static members.
    */
   inline CompressedWhitespace();

   CompressedWhitespace(const CompressedWhitespace &) = delete;
   CompressedWhitespace &operator=(const CompressedWhitespace &) = delete;
};

}

#endif
