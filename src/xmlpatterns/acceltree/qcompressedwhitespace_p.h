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
   static QString compress(QStringView input);
   static QString decompress(const QString &input);

 private:
   // use the two upper bits for communicating what space it is
   enum CharIdentifier {
      Space   = 0x0,
      CR      = 0x80,
      LF      = 0x40,
      Tab     = 0xC0
   };

   enum Constants {
      MaxCharCount = (1 << 6) - 1,
      Lower8Bits   = (1 << 8) - 1,
      Lower6Bits   = (1 << 6) - 1,
      UpperTwoBits = 3 << 6
   };

   static inline CharIdentifier toIdentifier(const QChar ch);

   static inline quint8 toCompressedChar(const QChar ch, const int len);
   static inline QChar toChar(const CharIdentifier id);
   static inline bool isEven(const int number);

   inline CompressedWhitespace();

   CompressedWhitespace(const CompressedWhitespace &) = delete;
   CompressedWhitespace &operator=(const CompressedWhitespace &) = delete;
};

}

#endif
