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

#include <qimage.h>
#include <qimage_p.h>
#include <qsimd_p.h>
#include <qdrawhelper_p.h>
#include <qdrawingprimitive_sse2_p.h>

#ifdef __SSE2__

bool convert_ARGB_to_ARGB_PM_inplace_sse2(QImageData *data, Qt::ImageConversionFlags)
{
   Q_ASSERT(data->format == QImage::Format_ARGB32 || data->format == QImage::Format_RGBA8888);

   // extra pixels on each line
   const int spare = data->width & 3;
   // width in pixels of the pad at the end of each line
   const int pad = (data->bytes_per_line >> 2) - data->width;
   const int iter = data->width >> 2;
   int height = data->height;

   const __m128i alphaMask = _mm_set1_epi32(0xff000000);
   const __m128i nullVector = _mm_setzero_si128();
   const __m128i half = _mm_set1_epi16(0x80);
   const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);

   __m128i *d = reinterpret_cast<__m128i *>(data->data);

   while (height--) {
      const __m128i *end = d + iter;

      for (; d != end; ++d) {
         const __m128i srcVector = _mm_loadu_si128(d);
         const __m128i srcVectorAlpha = _mm_and_si128(srcVector, alphaMask);
         if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, alphaMask)) == 0xffff) {
            // opaque, data is unchanged
         } else if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, nullVector)) == 0xffff) {
            // fully transparent
            _mm_storeu_si128(d, nullVector);
         } else {
            __m128i alphaChannel = _mm_srli_epi32(srcVector, 24);
            alphaChannel = _mm_or_si128(alphaChannel, _mm_slli_epi32(alphaChannel, 16));

            __m128i result;
            BYTE_MUL_SSE2(result, srcVector, alphaChannel, colorMask, half);
            result = _mm_or_si128(_mm_andnot_si128(alphaMask, result), srcVectorAlpha);
            _mm_storeu_si128(d, result);
         }
      }

      QRgb *p = reinterpret_cast<QRgb *>(d);
      QRgb *pe = p + spare;

      for (; p != pe; ++p) {

         if (*p < 0x00ffffff) {
            *p = 0;

         } else if (*p < 0xff000000) {
            *p = qPremultiply(*p);
         }
      }

      d = reinterpret_cast<__m128i *>(p + pad);
   }

   if (data->format == QImage::Format_ARGB32) {
      data->format = QImage::Format_ARGB32_Premultiplied;
   } else {
      data->format = QImage::Format_RGBA8888_Premultiplied;
   }

   return true;
}

#endif
