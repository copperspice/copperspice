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

#ifndef QWINDOWSURFACE_VGEGL_P_H
#define QWINDOWSURFACE_VGEGL_P_H

#include <qwindowsurface_p.h>
#include <qvg_p.h>

#if !defined(QT_NO_EGL)

#include <qeglcontext_p.h>

QT_BEGIN_NAMESPACE

class QWindowSurface;

class Q_OPENVG_EXPORT QVGEGLWindowSurfacePrivate
{
public:
    QVGEGLWindowSurfacePrivate(QWindowSurface *win);
    virtual ~QVGEGLWindowSurfacePrivate();

    QVGPaintEngine *paintEngine();
    virtual QEglContext *ensureContext(QWidget *widget) = 0;
    virtual void beginPaint(QWidget *widget) = 0;
    virtual void endPaint
        (QWidget *widget, const QRegion& region, QImage *image = 0) = 0;
    virtual VGImage surfaceImage() const;
    virtual QSize surfaceSize() const = 0;
    virtual bool supportsStaticContents() const { return false; }
    virtual bool scroll(QWidget *, const QRegion&, int, int) { return false; }

protected:
    QVGPaintEngine *engine;
    QWindowSurface *winSurface;

    void destroyPaintEngine();
    QSize windowSurfaceSize(QWidget *widget) const;
};

#if defined(EGL_OPENVG_IMAGE) && !defined(QVG_NO_SINGLE_CONTEXT)

#define QVG_VGIMAGE_BACKBUFFERS 1

class Q_OPENVG_EXPORT QVGEGLWindowSurfaceVGImage : public QVGEGLWindowSurfacePrivate
{
public:
    QVGEGLWindowSurfaceVGImage(QWindowSurface *win);
    virtual ~QVGEGLWindowSurfaceVGImage();

    QEglContext *ensureContext(QWidget *widget);
    void beginPaint(QWidget *widget);
    void endPaint(QWidget *widget, const QRegion& region, QImage *image);
    VGImage surfaceImage() const;
    QSize surfaceSize() const { return size; }

protected:
    QEglContext *context;
    VGImage backBuffer;
    EGLSurface backBufferSurface;
    bool recreateBackBuffer;
    bool isPaintingActive;
    QSize size;
    EGLSurface windowSurface;

    EGLSurface mainSurface() const;
};

#endif // EGL_OPENVG_IMAGE

class Q_OPENVG_EXPORT QVGEGLWindowSurfaceDirect : public QVGEGLWindowSurfacePrivate
{
public:
    QVGEGLWindowSurfaceDirect(QWindowSurface *win);
    virtual ~QVGEGLWindowSurfaceDirect();

    QEglContext *ensureContext(QWidget *widget);
    void beginPaint(QWidget *widget);
    void endPaint(QWidget *widget, const QRegion& region, QImage *image);
    QSize surfaceSize() const { return size; }
    bool supportsStaticContents() const;
    bool scroll(QWidget *widget, const QRegion& area, int dx, int dy);

protected:
    QEglContext *context;
    QSize size;
    bool isPaintingActive;
    bool needToSwap;
    EGLSurface windowSurface;
};

QT_END_NAMESPACE

#endif // !QT_NO_EGL

#endif // QWINDOWSURFACE_VGEGL_P_H
