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

#include <qdrawhelper_x86_p.h>

#ifdef __SSSE3__

#include <qdrawingprimitive_sse2_p.h>

static inline void blend_pixel(quint32 &dst, const quint32 src)
{
   if (src >= 0xff000000) {
      dst = src;
   } else if (src != 0) {
      dst = src + BYTE_MUL(dst, qAlpha(~src));
   }
}


/* The instruction palignr uses direct arguments, so we have to generate the code fo the different
   shift (4, 8, 12). Checking the alignment inside the loop is unfortunatelly way too slow.
 */
#define BLENDING_LOOP(palignrOffset, length)\
    for (; x-minusOffsetToAlignSrcOn16Bytes < length-7; x += 4) { \
        const __m128i srcVectorLastLoaded = _mm_load_si128((const __m128i *)&src[x - minusOffsetToAlignSrcOn16Bytes + 4]);\
        const __m128i srcVector = _mm_alignr_epi8(srcVectorLastLoaded, srcVectorPrevLoaded, palignrOffset); \
        const __m128i srcVectorAlpha = _mm_and_si128(srcVector, alphaMask); \
        if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, alphaMask)) == 0xffff) { \
            _mm_store_si128((__m128i *)&dst[x], srcVector); \
        } else if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, nullVector)) != 0xffff) { \
            __m128i alphaChannel = _mm_shuffle_epi8(srcVector, alphaShuffleMask); \
            alphaChannel = _mm_sub_epi16(one, alphaChannel); \
            const __m128i dstVector = _mm_load_si128((__m128i *)&dst[x]); \
            __m128i destMultipliedByOneMinusAlpha; \
            BYTE_MUL_SSE2(destMultipliedByOneMinusAlpha, dstVector, alphaChannel, colorMask, half); \
            const __m128i result = _mm_add_epi8(srcVector, destMultipliedByOneMinusAlpha); \
            _mm_store_si128((__m128i *)&dst[x], result); \
        } \
        srcVectorPrevLoaded = srcVectorLastLoaded;\
    }


// Basically blend src over dst with the const alpha defined as constAlphaVector.
// nullVector, half, one, colorMask are constant across the whole image/texture, and should be defined as:
//const __m128i nullVector = _mm_set1_epi32(0);
//const __m128i half = _mm_set1_epi16(0x80);
//const __m128i one = _mm_set1_epi16(0xff);
//const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
//const __m128i alphaMask = _mm_set1_epi32(0xff000000);
//
// The computation being done is:
// result = s + d * (1-alpha)
// with shortcuts if fully opaque or fully transparent.
#define BLEND_SOURCE_OVER_ARGB32_SSSE3(dst, src, length, nullVector, half, one, colorMask, alphaMask) { \
    int x = 0; \
\
    /* First, get dst aligned. */ \
    ALIGNMENT_PROLOGUE_16BYTES(dst, x, length) { \
        blend_pixel(dst[x], src[x]); \
    } \
\
    const int minusOffsetToAlignSrcOn16Bytes = (reinterpret_cast<quintptr>(&(src[x])) >> 2) & 0x3;\
\
    if (!minusOffsetToAlignSrcOn16Bytes) {\
        /* src is aligned, usual algorithm but with aligned operations.\
           See the SSE2 version for more documentation on the algorithm itself. */\
        const __m128i alphaShuffleMask = _mm_set_epi8(char(0xff),15,char(0xff),15,char(0xff),11,char(0xff),11,char(0xff),7,char(0xff),7,char(0xff),3,char(0xff),3);\
        for (; x < length-3; x += 4) { \
            const __m128i srcVector = _mm_load_si128((const __m128i *)&src[x]); \
            const __m128i srcVectorAlpha = _mm_and_si128(srcVector, alphaMask); \
            if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, alphaMask)) == 0xffff) { \
                _mm_store_si128((__m128i *)&dst[x], srcVector); \
            } else if (_mm_movemask_epi8(_mm_cmpeq_epi32(srcVectorAlpha, nullVector)) != 0xffff) { \
                __m128i alphaChannel = _mm_shuffle_epi8(srcVector, alphaShuffleMask); \
                alphaChannel = _mm_sub_epi16(one, alphaChannel); \
                const __m128i dstVector = _mm_load_si128((__m128i *)&dst[x]); \
                __m128i destMultipliedByOneMinusAlpha; \
                BYTE_MUL_SSE2(destMultipliedByOneMinusAlpha, dstVector, alphaChannel, colorMask, half); \
                const __m128i result = _mm_add_epi8(srcVector, destMultipliedByOneMinusAlpha); \
                _mm_store_si128((__m128i *)&dst[x], result); \
            } \
        } /* end for() */\
    } else if ((length - x) >= 8) {\
        /* We use two vectors to extract the src: prevLoaded for the first pixels, lastLoaded for the current pixels. */\
        __m128i srcVectorPrevLoaded = _mm_load_si128((const __m128i *)&src[x - minusOffsetToAlignSrcOn16Bytes]);\
        const int palignrOffset = minusOffsetToAlignSrcOn16Bytes << 2;\
\
        const __m128i alphaShuffleMask = _mm_set_epi8(char(0xff),15,char(0xff),15,char(0xff),11,char(0xff),11,char(0xff),7,char(0xff),7,char(0xff),3,char(0xff),3);\
        switch (palignrOffset) {\
        case 4:\
            BLENDING_LOOP(4, length)\
            break;\
        case 8:\
            BLENDING_LOOP(8, length)\
            break;\
        case 12:\
            BLENDING_LOOP(12, length)\
            break;\
        }\
    }\
    for (; x < length; ++x) \
        blend_pixel(dst[x], src[x]); \
}

void qt_blend_argb32_on_argb32_ssse3(uchar *destPixels, int dbpl,
   const uchar *srcPixels, int sbpl,
   int w, int h,
   int const_alpha)
{
   const quint32 *src = (const quint32 *) srcPixels;
   quint32 *dst = (quint32 *) destPixels;
   if (const_alpha == 256) {
      const __m128i alphaMask = _mm_set1_epi32(0xff000000);
      const __m128i nullVector = _mm_setzero_si128();
      const __m128i half = _mm_set1_epi16(0x80);
      const __m128i one = _mm_set1_epi16(0xff);
      const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);

      for (int y = 0; y < h; ++y) {
         BLEND_SOURCE_OVER_ARGB32_SSSE3(dst, src, w, nullVector, half, one, colorMask, alphaMask);
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   } else if (const_alpha != 0) {
      // dest = (s + d * sia) * ca + d * cia
      //      = s * ca + d * (sia * ca + cia)
      //      = s * ca + d * (1 - sa*ca)
      const_alpha = (const_alpha * 255) >> 8;
      const __m128i nullVector = _mm_setzero_si128();
      const __m128i half = _mm_set1_epi16(0x80);
      const __m128i one = _mm_set1_epi16(0xff);
      const __m128i colorMask = _mm_set1_epi32(0x00ff00ff);
      const __m128i constAlphaVector = _mm_set1_epi16(const_alpha);
      for (int y = 0; y < h; ++y) {
         BLEND_SOURCE_OVER_ARGB32_WITH_CONST_ALPHA_SSE2(dst, src, w, nullVector, half, one, colorMask, constAlphaVector)
         dst = (quint32 *)(((uchar *) dst) + dbpl);
         src = (const quint32 *)(((const uchar *) src) + sbpl);
      }
   }
}

#endif
