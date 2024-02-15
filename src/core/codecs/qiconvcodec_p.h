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

#ifndef QICONVCODEC_P_H
#define QICONVCODEC_P_H

#include <qtextcodec.h>

#if defined(Q_OS_UNIX) && ! defined(QT_NO_ICONV)

#ifdef Q_OS_DARWIN
using iconv_t = void *;

#else
#include <iconv.h>

#endif

class QIconvCodec: public QTextCodec
{
 private:
   mutable QTextCodec *utf16Codec;

 public:
   QIconvCodec();
   ~QIconvCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(QStringView str, ConverterState *) const override;

   QString name() const override;
   int mibEnum() const override;

   static iconv_t createIconv_t(const char *to, const char *from);

   class IconvState
   {
    public:
      IconvState(iconv_t x);
      ~IconvState();

      ConverterState internalState;

      char *buffer;
      int bufferLen;
      iconv_t cd;

      char array[8];

      void saveChars(const char *c, int count);
   };
};

#endif // Q_OS_UNIX && !QT_NO_ICONV

#endif // QICONVCODEC_P_H
