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

#include <qfontlaocodec_p.h>
#include <qlist.h>

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS

static unsigned char const unicode_to_mulelao[256] = {
   // U+0E80
   0x00, 0xa1, 0xa2, 0x00, 0xa4, 0x00, 0x00, 0xa7,
   0xa8, 0x00, 0xaa, 0x00, 0x00, 0xad, 0x00, 0x00,
   // U+0E90
   0x00, 0x00, 0x00, 0x00, 0xb4, 0xb5, 0xb6, 0xb7,
   0x00, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
   // U+0EA0
   0x00, 0xc1, 0xc2, 0xc3, 0x00, 0xc5, 0x00, 0xc7,
   0x00, 0x00, 0xca, 0xcb, 0x00, 0xcd, 0xce, 0xcf,
   // U+0EB0
   0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
   0xd8, 0xd9, 0x00, 0xdb, 0xdc, 0xdd, 0x00, 0x00,
   // U+0EC0
   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0x00, 0xe6, 0x00,
   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0x00, 0x00,
   // U+0ED0
   0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
   0xf8, 0xf9, 0x00, 0x00, 0xfc, 0xfd, 0x00, 0x00,
   // U+0EE0
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // U+0EF0
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

QFontLaoCodec::~QFontLaoCodec()
{
}

QString QFontLaoCodec::name() const
{
   return QString("mulelao-1");
}

int QFontLaoCodec::mibEnum() const
{
   return -4242;
}

QString QFontLaoCodec::convertToUnicode(const char *, int, ConverterState *) const
{
   return QString();
}

QByteArray QFontLaoCodec::convertFromUnicode(QStringView str, ConverterState *) const
{
   QByteArray retval;

   for (auto c : str) {
      char32_t uc = c.unicode();

      if (uc < 0x80) {
         retval.append(uc & 0xFF);

      } else if (uc >= 0x0e80 && uc <= 0x0eff) {
         uchar lao = unicode_to_mulelao[uc - 0x0e80];

         if (lao) {
            retval.append(lao);

         } else {
            retval.append('\0');
         }

      } else {
         retval.append('\0');
      }
   }

   return retval;
}

#endif // QT_NO_BIG_CODECS

#endif // QT_NO_CODECS
