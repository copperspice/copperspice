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

#ifndef QWINDOWSURFACE_VG_P_H
#define QWINDOWSURFACE_VG_P_H

#include <qwindowsurface_p.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_EGL)

class QVGEGLWindowSurfacePrivate;

class Q_OPENVG_EXPORT QVGWindowSurface : public QWindowSurface, public QPaintDevice
{
public:
    QVGWindowSurface(QWidget *window);
    QVGWindowSurface(QWidget *window, QVGEGLWindowSurfacePrivate *d);
    ~QVGWindowSurface();

    QPaintDevice *paintDevice();
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

#if !defined(Q_WS_QPA)
    void setGeometry(const QRect &rect);
#else
	void resize(const QSize &size);
#endif

    bool scroll(const QRegion &area, int dx, int dy);

    void beginPaint(const QRegion &region);
    void endPaint(const QRegion &region);

    QPaintEngine *paintEngine() const;

    WindowSurfaceFeatures features() const;

protected:
    int metric(PaintDeviceMetric metric) const;

private:
    QVGEGLWindowSurfacePrivate *d_ptr;
};

#endif

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_VG_P_H
