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

#include <qdrawhelper_arm_simd_p.h>
#include <qpaintengine_raster_p.h>
#include <qblendfunctions_p.h>

#ifdef QT_HAVE_ARM_SIMD

// TODO: add GNU assembler instructions and support for other platforms.
//       Default to C code for now

void qt_blend_argb32_on_argb32_arm_simd(uchar *destPixels, int dbpl,
                                        const uchar *srcPixels, int sbpl,
                                        int w, int h,
                                        int const_alpha)
{
   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;
   if (const_alpha == 256) {
      for (int y = 0; y < h; ++y) {
         for (int x = 0; x < w; ++x) {
            uint s = src[x];
            if (s >= 0xff000000) {
               dst[x] = s;
            } else if (s != 0) {
               dst[x] = s + BYTE_MUL(dst[x], qAlpha(~s));
            }
         }
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   } else if (const_alpha != 0) {
      const_alpha = (const_alpha * 255) >> 8;
      for (int y = 0; y < h; ++y) {
         for (int x = 0; x < w; ++x) {
            uint s = BYTE_MUL(src[x], const_alpha);
            dst[x] = s + BYTE_MUL(dst[x], qAlpha(~s));
         }
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   }
}

void qt_blend_rgb32_on_rgb32_arm_simd(uchar *destPixels, int dbpl,
                                      const uchar *srcPixels, int sbpl,
                                      int w, int h,
                                      int const_alpha)
{
   if (const_alpha != 256) {
      qt_blend_argb32_on_argb32_arm_simd(destPixels, dbpl, srcPixels, sbpl, w, h, const_alpha);
      return;
   }

   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;
   if (w <= 64) {
      for (int y = 0; y < h; ++y) {
         qt_memconvert(dst, src, w);
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   } else {
      int len = w * 4;
      for (int y = 0; y < h; ++y) {
         memcpy(dst, src, len);
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   }
}

#endif // QT_HAVE_ARMV_SIMD
