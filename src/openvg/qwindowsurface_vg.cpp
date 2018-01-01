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

#include <qwindowsurface_vg_p.h>
#include <qwindowsurface_vgegl_p.h>
#include <qpaintengine_vg_p.h>
#include <qpixmapdata_vg_p.h>
#include <qvg_p.h>

#if !defined(QT_NO_EGL)

#include <qeglcontext_p.h>
#include <qwidget_p.h>

QT_BEGIN_NAMESPACE

QVGWindowSurface::QVGWindowSurface(QWidget *window)
    : QWindowSurface(window)
{
    // Create the default type of EGL window surface for windows.
    d_ptr = new QVGEGLWindowSurfaceDirect(this);
}

QVGWindowSurface::QVGWindowSurface
        (QWidget *window, QVGEGLWindowSurfacePrivate *d)
    : QWindowSurface(window), d_ptr(d)
{
}

QVGWindowSurface::~QVGWindowSurface()
{
    delete d_ptr;
}

QPaintDevice *QVGWindowSurface::paintDevice()
{
    return this;
}

void QVGWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset);

    QWidget *parent = widget->internalWinId() ? widget : widget->nativeParentWidget();
    d_ptr->endPaint(parent, region);
}

#if !defined(Q_WS_QPA)
void QVGWindowSurface::setGeometry(const QRect &rect)
{
    QWindowSurface::setGeometry(rect);
}
#else
void QVGWindowSurface::resize(const QSize &size)
{
            QWindowSurface::resize(size);
}
#endif //!Q_WS_QPA

bool QVGWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    if (!d_ptr->scroll(window(), area, dx, dy))
        return QWindowSurface::scroll(area, dx, dy);
    return true;
}

void QVGWindowSurface::beginPaint(const QRegion &region)
{
    d_ptr->beginPaint(window());

    // If the window is not opaque, then fill the region we are about
    // to paint with the transparent color.
    if (!qt_widget_private(window())->isOpaque &&
            window()->testAttribute(Qt::WA_TranslucentBackground)) {
        QVGPaintEngine *engine = static_cast<QVGPaintEngine *>
            (d_ptr->paintEngine());
        engine->fillRegion(region, Qt::transparent, d_ptr->surfaceSize());
    }
}

void QVGWindowSurface::endPaint(const QRegion &region)
{
    // Nothing to do here.
    Q_UNUSED(region);
}

QPaintEngine *QVGWindowSurface::paintEngine() const
{
    return d_ptr->paintEngine();
}

QWindowSurface::WindowSurfaceFeatures QVGWindowSurface::features() const
{
    WindowSurfaceFeatures features = PartialUpdates | PreservedContents;
    if (d_ptr->supportsStaticContents())
        features |= StaticContents;
    return features;
}

int QVGWindowSurface::metric(PaintDeviceMetric met) const
{
    return qt_paint_device_metric(window(), met);
}

QT_END_NAMESPACE

#endif
