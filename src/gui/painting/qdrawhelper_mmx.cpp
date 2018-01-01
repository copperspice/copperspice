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

#include <qdrawhelper_p.h>

#if defined(QT_HAVE_MMX)

#include <qdrawhelper_mmx_p.h>

QT_BEGIN_NAMESPACE

CompositionFunctionSolid qt_functionForModeSolid_MMX[numCompositionFunctions] = {
   comp_func_solid_SourceOver<QMMXIntrinsics>,
   comp_func_solid_DestinationOver<QMMXIntrinsics>,
   comp_func_solid_Clear<QMMXIntrinsics>,
   comp_func_solid_Source<QMMXIntrinsics>,
   0,
   comp_func_solid_SourceIn<QMMXIntrinsics>,
   comp_func_solid_DestinationIn<QMMXIntrinsics>,
   comp_func_solid_SourceOut<QMMXIntrinsics>,
   comp_func_solid_DestinationOut<QMMXIntrinsics>,
   comp_func_solid_SourceAtop<QMMXIntrinsics>,
   comp_func_solid_DestinationAtop<QMMXIntrinsics>,
   comp_func_solid_XOR<QMMXIntrinsics>,
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

CompositionFunction qt_functionForMode_MMX[numCompositionFunctions] = {
   comp_func_SourceOver<QMMXIntrinsics>,
   comp_func_DestinationOver<QMMXIntrinsics>,
   comp_func_Clear<QMMXIntrinsics>,
   comp_func_Source<QMMXIntrinsics>,
   comp_func_Destination,
   comp_func_SourceIn<QMMXIntrinsics>,
   comp_func_DestinationIn<QMMXIntrinsics>,
   comp_func_SourceOut<QMMXIntrinsics>,
   comp_func_DestinationOut<QMMXIntrinsics>,
   comp_func_SourceAtop<QMMXIntrinsics>,
   comp_func_DestinationAtop<QMMXIntrinsics>,
   comp_func_XOR<QMMXIntrinsics>,
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

void qt_blend_color_argb_mmx(int count, const QSpan *spans, void *userData)
{
   qt_blend_color_argb_x86<QMMXIntrinsics>(count, spans, userData,
                                           (CompositionFunctionSolid *)qt_functionForModeSolid_MMX);
}


void qt_blend_argb32_on_argb32_mmx(uchar *destPixels, int dbpl,
                                   const uchar *srcPixels, int sbpl,
                                   int w, int h,
                                   int const_alpha)
{
   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;

   uint ca = const_alpha - 1;

   for (int y = 0; y < h; ++y) {
      comp_func_SourceOver<QMMXIntrinsics>(dst, src, w, ca);
      dst = (quint32 *)(((uchar *) dst) + dbpl);
      src = (const quint32 *)(((const uchar *) src) + sbpl);
   }
}

void qt_blend_rgb32_on_rgb32_mmx(uchar *destPixels, int dbpl,
                                 const uchar *srcPixels, int sbpl,
                                 int w, int h,
                                 int const_alpha)
{
   const uint *src = (const uint *) srcPixels;
   uint *dst = (uint *) destPixels;

   uint ca = const_alpha - 1;

   for (int y = 0; y < h; ++y) {
      comp_func_Source<QMMXIntrinsics>(dst, src, w, ca);
      dst = (quint32 *)(((uchar *) dst) + dbpl);
      src = (const quint32 *)(((const uchar *) src) + sbpl);
   }
}

QT_END_NAMESPACE

#endif // QT_HAVE_MMX

