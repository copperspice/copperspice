/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qimage.h>
#include <qimage_p.h>
#include <qsimd_p.h>

#ifdef QT_HAVE_NEON

QT_BEGIN_NAMESPACE

Q_GUI_EXPORT void QT_FASTCALL qt_convert_rgb888_to_rgb32_neon(quint32 *dst, const uchar *src, int len)
{
   if (!len) {
      return;
   }

   const quint32 *const end = dst + len;

   // align dst on 64 bits
   const int offsetToAlignOn8Bytes = (reinterpret_cast<quintptr>(dst) >> 2) & 0x1;
   for (int i = 0; i < offsetToAlignOn8Bytes; ++i) {
      *dst++ = qRgb(src[0], src[1], src[2]);
      src += 3;
   }

   if ((len - offsetToAlignOn8Bytes) >= 8) {
      const quint32 *const simdEnd = end - 7;
      register uint8x8_t fullVector asm ("d3") = vdup_n_u8(0xff);
      do {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
         asm volatile (
            "vld3.8     { d4, d5, d6 }, [%[SRC]] !\n\t"
            "vst4.8     { d3, d4, d5, d6 }, [%[DST],:64] !\n\t"
            : [DST]"+r" (dst), [SRC]"+r" (src)
            : "w"(fullVector)
            : "memory", "d4", "d5", "d6"
         );
#else
         asm volatile (
            "vld3.8     { d0, d1, d2 }, [%[SRC]] !\n\t"
            "vswp d0, d2\n\t"
            "vst4.8     { d0, d1, d2, d3 }, [%[DST],:64] !\n\t"
            : [DST]"+r" (dst), [SRC]"+r" (src)
            : "w"(fullVector)
            : "memory", "d0", "d1", "d2"
         );
#endif
      } while (dst < simdEnd);
   }

   while (dst != end) {
      *dst++ = qRgb(src[0], src[1], src[2]);
      src += 3;
   }
}

void convert_RGB888_to_RGB32_neon(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
   Q_ASSERT(src->format == QImage::Format_RGB888);
   Q_ASSERT(dest->format == QImage::Format_RGB32 || dest->format == QImage::Format_ARGB32 ||
            dest->format == QImage::Format_ARGB32_Premultiplied);
   Q_ASSERT(src->width == dest->width);
   Q_ASSERT(src->height == dest->height);

   const uchar *src_data = (uchar *) src->data;
   quint32 *dest_data = (quint32 *) dest->data;

   for (int i = 0; i < src->height; ++i) {
      qt_convert_rgb888_to_rgb32_neon(dest_data, src_data, src->width);
      src_data += src->bytes_per_line;
      dest_data = (quint32 *)((uchar *)dest_data + dest->bytes_per_line);
   }
}

QT_END_NAMESPACE

#endif // QT_HAVE_NEON
