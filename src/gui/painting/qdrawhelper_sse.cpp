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

#include <qdrawhelper_p.h>

#ifdef QT_HAVE_SSE

#include <qdrawhelper_sse_p.h>

QT_BEGIN_NAMESPACE

CompositionFunctionSolid qt_functionForModeSolid_SSE[numCompositionFunctions] = {
   comp_func_solid_SourceOver<QSSEIntrinsics>,
   comp_func_solid_DestinationOver<QSSEIntrinsics>,
   comp_func_solid_Clear<QSSEIntrinsics>,
   comp_func_solid_Source<QSSEIntrinsics>,
   0,
   comp_func_solid_SourceIn<QSSEIntrinsics>,
   comp_func_solid_DestinationIn<QSSEIntrinsics>,
   comp_func_solid_SourceOut<QSSEIntrinsics>,
   comp_func_solid_DestinationOut<QSSEIntrinsics>,
   comp_func_solid_SourceAtop<QSSEIntrinsics>,
   comp_func_solid_DestinationAtop<QSSEIntrinsics>,
   comp_func_solid_XOR<QSSEIntrinsics>,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // svg 1.2 modes
   rasterop_solid_SourceOrDestination<QMMXIntrinsics>,
   rasterop_solid_SourceAndDestination<QMMXIntrinsics>,
   rasterop_solid_SourceXorDestination<QMMXIntrinsics>,
   rasterop_solid_NotSourceAndNotDestination<QMMXIntrinsics>,
   rasterop_solid_NotSourceOrNotDestination<QMMXIntrinsics>,
   rasterop_solid_NotSourceXorDestination<QMMXIntrinsics>,
   rasterop_solid_NotSource<QMMXIntrinsics>,
   rasterop_solid_NotSourceAndDestination<QMMXIntrinsics>,
   rasterop_solid_SourceAndNotDestination<QMMXIntrinsics>
};

CompositionFunction qt_functionForMode_SSE[numCompositionFunctions] = {
   comp_func_SourceOver<QSSEIntrinsics>,
   comp_func_DestinationOver<QSSEIntrinsics>,
   comp_func_Clear<QSSEIntrinsics>,
   comp_func_Source<QSSEIntrinsics>,
   comp_func_Destination,
   comp_func_SourceIn<QSSEIntrinsics>,
   comp_func_DestinationIn<QSSEIntrinsics>,
   comp_func_SourceOut<QSSEIntrinsics>,
   comp_func_DestinationOut<QSSEIntrinsics>,
   comp_func_SourceAtop<QSSEIntrinsics>,
   comp_func_DestinationAtop<QSSEIntrinsics>,
   comp_func_XOR<QSSEIntrinsics>,
   comp_func_Plus,
   comp_func_Multiply,
   comp_func_Screen,
   comp_func_Overlay,
   comp_func_Darken,
   comp_func_Lighten,
   comp_func_ColorDodge,
   comp_func_ColorBurn,
   comp_func_HardLight,
   comp_func_SoftLight,
   comp_func_Difference,
   comp_func_Exclusion,
   rasterop_SourceOrDestination,
   rasterop_SourceAndDestination,
   rasterop_SourceXorDestination,
   rasterop_NotSourceAndNotDestination,
   rasterop_NotSourceOrNotDestination,
   rasterop_NotSourceXorDestination,
   rasterop_NotSource,
   rasterop_NotSourceAndDestination,
   rasterop_SourceAndNotDestination
};

void qt_blend_color_argb_sse(int count, const QSpan *spans, void *userData)
{
   qt_blend_color_argb_x86<QSSEIntrinsics>(count, spans, userData,
                                           (CompositionFunctionSolid *)qt_functionForModeSolid_SSE);
}

void qt_memfill32_sse(quint32 *dest, quint32 value, int count)
{
   return qt_memfill32_sse_template<QSSEIntrinsics>(dest, value, count);
}

void qt_bitmapblit16_sse(QRasterBuffer *rasterBuffer, int x, int y,
                         quint32 color,
                         const uchar *src,
                         int width, int height, int stride)
{
   return qt_bitmapblit16_sse_template<QSSEIntrinsics>(rasterBuffer, x, y,
          color, src, width,
          height, stride);
}

void qt_blend_argb32_on_argb32_sse(uchar *destPixels, int dbpl,
                                   const uchar *srcPixels, int sbpl,
                                   int w, int h,
                                   int const_alpha)
{
   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;

   uint ca = const_alpha - 1;

   for (int y = 0; y < h; ++y) {
      comp_func_SourceOver<QSSEIntrinsics>(dst, src, w, ca);
      dst = (quint32 *)(((uchar *) dst) + dbpl);
      src = (const quint32 *)(((const uchar *) src) + sbpl);
   }
}

void qt_blend_rgb32_on_rgb32_sse(uchar *destPixels, int dbpl,
                                 const uchar *srcPixels, int sbpl,
                                 int w, int h,
                                 int const_alpha)
{
   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;

   uint ca = const_alpha - 1;

   for (int y = 0; y < h; ++y) {
      comp_func_Source<QSSEIntrinsics>(dst, src, w, ca);
      dst = (quint32 *)(((uchar *) dst) + dbpl);
      src = (const quint32 *)(((const uchar *) src) + sbpl);
   }
}

QT_END_NAMESPACE

#endif // QT_HAVE_SSE
