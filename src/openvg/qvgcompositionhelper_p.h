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

#ifndef QVGCOMPOSITIONHELPER_H
#define QVGCOMPOSITIONHELPER_H

#include "qwindowsurface_vgegl_p.h"

QT_BEGIN_NAMESPACE

#if !defined(QVG_NO_SINGLE_CONTEXT) && !defined(QT_NO_EGL)

class QVGPaintEnginePrivate;
class QVGEGLWindowSurfacePrivate;

class Q_OPENVG_EXPORT QVGCompositionHelper
{
public:
    QVGCompositionHelper();
    virtual ~QVGCompositionHelper();

    void startCompositing(const QSize& screenSize);
    void endCompositing();

    void blitWindow(VGImage image, const QSize& imageSize,
                    const QRect& rect, const QPoint& topLeft, int opacity);
    void fillBackground(const QRegion& region, const QBrush& brush);
    void drawCursorPixmap(const QPixmap& pixmap, const QPoint& offset);
    void setScissor(const QRegion& region);
    void clearScissor();

private:
    QVGPaintEnginePrivate *d;
    QSize screenSize;
};

#endif

QT_END_NAMESPACE

#endif
