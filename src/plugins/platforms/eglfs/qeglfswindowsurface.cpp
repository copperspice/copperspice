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

#include "qeglfswindowsurface.h"

#include <QtGui/QPlatformGLContext>

#include <QtOpenGL/private/qgl_p.h>
#include <QtOpenGL/private/qglpaintdevice_p.h>

QT_BEGIN_NAMESPACE

class QEglFSPaintDevice : public QGLPaintDevice
{
public:
    QEglFSPaintDevice(QEglFSScreen *screen, QWidget *widget)
        :QGLPaintDevice(), m_screen(screen)
    {
    #ifdef QEGL_EXTRA_DEBUG
        qWarning("QEglPaintDevice %p, %p, %p",this, screen, widget);
    #endif
    }

    QSize size() const { return m_screen->geometry().size(); }
    QGLContext* context() const { return QGLContext::fromPlatformGLContext(m_screen->platformContext());}

    QPaintEngine *paintEngine() const { return qt_qgl_paint_engine(); }

    void  beginPaint(){
        QGLPaintDevice::beginPaint();
    }
private:
    QEglFSScreen *m_screen;
    QGLContext *m_context;
};


QEglFSWindowSurface::QEglFSWindowSurface( QEglFSScreen *screen, QWidget *window )
    :QWindowSurface(window)
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglWindowSurface %p, %p", window, screen);
#endif
    m_paintDevice = new QEglFSPaintDevice(screen,window);
}

void QEglFSWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(region);
    Q_UNUSED(offset);
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglWindowSurface::flush %p",widget);
#endif
    widget->platformWindow()->glContext()->swapBuffers();
}

void QEglFSWindowSurface::resize(const QSize &size)
{
    Q_UNUSED(size);
}

QT_END_NAMESPACE
