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

#ifndef PVRQWSDRAWABLE_P_H
#define PVRQWSDRAWABLE_P_H

#include <pvr2d.h>
#include "pvrqwsdrawable.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PVRQWS_MAX_VISIBLE_RECTS    32
#define PVRQWS_MAX_SCREENS          1
#define PVRQWS_MAX_BACK_BUFFERS     2
#define PVRQWS_MAX_FLIP_BUFFERS     2

typedef struct {

    PvrQwsRect          screenRect;
    int                 screenStride;
    PVR2DFORMAT         pixelFormat;
    int                 bytesPerPixel;
    PVR2DMEMINFO       *frameBuffer;
    PvrQwsDrawable     *screenDrawable;
    void               *mapped;
    int                 mappedLength;
    unsigned long       screenStart;
    int                 needsUnmap;
    int                 initialized;

} PvrQwsScreenInfo;

typedef struct {

    int                 refCount;
    PvrQwsScreenInfo    screens[PVRQWS_MAX_SCREENS];
    PVR2DCONTEXTHANDLE  context;
    int                 numDrawables;
    unsigned long       numFlipBuffers;
    PVR2DFLIPCHAINHANDLE flipChain;
    PVR2DMEMINFO       *flipBuffers[PVRQWS_MAX_FLIP_BUFFERS];
    int                 usePresentBlit;
    PvrQwsDrawable     *firstWinId;

} PvrQwsDisplay;

extern PvrQwsDisplay pvrQwsDisplay;

struct _PvrQwsDrawable
{
    PvrQwsDrawableType  type;
    long                winId;
    int                 refCount;
    PvrQwsRect          rect;
    int                 screen;
    PVR2DFORMAT         pixelFormat;
    PvrQwsRect          visibleRects[PVRQWS_MAX_VISIBLE_RECTS];
    int                 numVisibleRects;
    PVR2DMEMINFO       *backBuffers[PVRQWS_MAX_BACK_BUFFERS];
    int                 currentBackBuffer;
    int                 backBuffersValid;
    int                 usingFlipBuffers;
    int                 isFullScreen;
    int                 strideBytes;
    int                 stridePixels;
    int                 rotationAngle;
    PvrQwsSwapFunction  swapFunction;
    void               *userData;
    PvrQwsDrawable     *nextWinId;

};

/* Get the current source and render buffers for a drawable */
int pvrQwsGetBuffers
    (PvrQwsDrawable *drawable, PVR2DMEMINFO **source, PVR2DMEMINFO **render);

#ifdef __cplusplus
};
#endif

#endif
