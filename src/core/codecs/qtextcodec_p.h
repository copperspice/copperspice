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

#ifndef QTEXTCODEC_P_H
#define QTEXTCODEC_P_H

#include <qtextcodec.h>
#include <string.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTCODEC

typedef void (*QTextCodecStateFreeFunction)(QTextCodec::ConverterState *);

struct QTextCodecUnalignedPointer {
   static inline QTextCodecStateFreeFunction decode(const uint *src) {
      quintptr data;
      memcpy(&data, src, sizeof(data));
      return reinterpret_cast<QTextCodecStateFreeFunction>(data);
   }

   static inline void encode(uint *dst, QTextCodecStateFreeFunction fn) {
      quintptr data = reinterpret_cast<quintptr>(fn);
      memcpy(dst, &data, sizeof(data));
   }
};

#else

class QTextCodec
{
 public:
   enum ConversionFlag {
      DefaultConversion,
      ConvertInvalidToNull = 0x80000000,
      IgnoreHeader = 0x1,
      FreeFunction = 0x2
   };
   using ConversionFlags = QFlags<ConversionFlag>;

   struct ConverterState {
      ConverterState(ConversionFlags f = DefaultConversion)
         : flags(f), remainingChars(0), invalidChars(0), d(0) {
         state_data[0] = state_data[1] = state_data[2] = 0;
      }
      ~ConverterState() { }
      ConversionFlags flags;
      int remainingChars;
      int invalidChars;
      uint state_data[3];
      void *d;

    private:
      Q_DISABLE_COPY(ConverterState)
   };
};

#endif //QT_NO_TEXTCODEC

QT_END_NAMESPACE

#endif
