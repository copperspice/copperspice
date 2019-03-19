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

#include "qxlibwindowsurface.h"
#include "qxlibintegration.h"

#include <QtCore/qdebug.h>
#include <QWindowSystemInterface>

#include "qxlibwindow.h"
#include "qxlibscreen.h"
#include "qxlibdisplay.h"

#include "qpainter.h"

# include <sys/ipc.h>
# include <sys/shm.h>
# include <X11/extensions/XShm.h>

QT_BEGIN_NAMESPACE


struct QXlibShmImageInfo {
    QXlibShmImageInfo(Display *xdisplay) :  image(0), display(xdisplay) {}
    ~QXlibShmImageInfo() { destroy(); }

    void destroy();

    XShmSegmentInfo shminfo;
    XImage *image;
    Display *display;
};


#ifndef DONT_USE_MIT_SHM
void QXlibShmImageInfo::destroy()
{
    XShmDetach (display, &shminfo);
    XDestroyImage (image);
    shmdt (shminfo.shmaddr);
    shmctl (shminfo.shmid, IPC_RMID, 0);
}
#endif

void QXlibWindowSurface::resizeShmImage(int width, int height)
{
    QXlibScreen *screen = QXlibScreen::testLiteScreenForWidget(window());
    QXlibWindow *win = static_cast<QXlibWindow*>(window()->platformWindow());

#ifdef DONT_USE_MIT_SHM
    shm_img = QImage(width, height, win->format());
#else

    if (image_info)
        image_info->destroy();
    else
        image_info = new QXlibShmImageInfo(screen->display()->nativeDisplay());

    XImage *image = XShmCreateImage (screen->display()->nativeDisplay(), win->visual(), win->depth(), ZPixmap, 0,
                                     &image_info->shminfo, width, height);


    image_info->shminfo.shmid = shmget (IPC_PRIVATE,
          image->bytes_per_line * image->height, IPC_CREAT|0700);

    image_info->shminfo.shmaddr = image->data = (char*)shmat (image_info->shminfo.shmid, 0, 0);
    image_info->shminfo.readOnly = False;

    image_info->image = image;

    Status shm_attach_status = XShmAttach(screen->display()->nativeDisplay(), &image_info->shminfo);

    Q_ASSERT(shm_attach_status == True);

    shm_img = QImage( (uchar*) image->data, image->width, image->height, image->bytes_per_line, win->format() );
#endif
    painted = false;
}


void QXlibWindowSurface::resizeBuffer(QSize s)
{
    if (shm_img.size() != s)
        resizeShmImage(s.width(), s.height());
}

QSize QXlibWindowSurface::bufferSize() const
{
    return shm_img.size();
}

QXlibWindowSurface::QXlibWindowSurface (QWidget *window)
    : QWindowSurface(window),
      painted(false), image_info(0)
{
    xw = static_cast<QXlibWindow*>(window->platformWindow());
//    qDebug() << "QTestLiteWindowSurface::QTestLiteWindowSurface:" << xw->window;
}

QXlibWindowSurface::~QXlibWindowSurface()
{
    delete image_info;
}

QPaintDevice *QXlibWindowSurface::paintDevice()
{
    return &shm_img;
}


void QXlibWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(region);
    Q_UNUSED(offset);

    if (!painted)
        return;

    QXlibScreen *screen = QXlibScreen::testLiteScreenForWidget(widget);
    GC gc = xw->graphicsContext();
    Window window = xw->xWindow();
#ifdef DONT_USE_MIT_SHM
    // just convert the image every time...
    if (!shm_img.isNull()) {
        QXlibWindow *win = static_cast<QXlibWindow*>(window()->platformWindow());

        QImage image = shm_img;
        //img.convertToFormat(
        XImage *xi = XCreateImage(screen->display(), win->visual(), win->depth(), ZPixmap,
                                  0, (char *) image.scanLine(0), image.width(), image.height(),
                                  32, image.bytesPerLine());

        int x = 0;
        int y = 0;

        /*int r =*/  XPutImage(screen->display(), window, gc, xi, 0, 0, x, y, image.width(), image.height());

        xi->data = 0; // QImage owns these bits
        XDestroyImage(xi);
    }
#else
    // Use MIT_SHM
    if (image_info && image_info->image) {
        //qDebug() << "Here we go" << image_info->image->width << image_info->image->height;
        int x = 0;
        int y = 0;

        // We could set send_event to true, and then use the ShmCompletion to synchronize,
        // but let's do like Qt/11 and just use XSync
        XShmPutImage (screen->display()->nativeDisplay(), window, gc, image_info->image, 0, 0,
                      x, y, image_info->image->width, image_info->image->height,
                      /*send_event*/ False);

        screen->display()->sync();
    }
#endif
}

// from qwindowsurface.cpp
extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

bool QXlibWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    if (shm_img.isNull())
        return false;

    const QVector<QRect> rects = area.rects();
    for (int i = 0; i < rects.size(); ++i)
        qt_scrollRectInImage(shm_img, rects.at(i), QPoint(dx, dy));

    return true;
}


void QXlibWindowSurface::beginPaint(const QRegion &region)
{
    Q_UNUSED(region);
    resizeBuffer(size());

    if (shm_img.hasAlphaChannel()) {
        QPainter p(&shm_img);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        const QVector<QRect> rects = region.rects();
        const QColor blank = Qt::transparent;
        for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it) {
            p.fillRect(*it, blank);
        }
    }
}

void QXlibWindowSurface::endPaint(const QRegion &region)
{
    Q_UNUSED(region);
    painted = true; //there is content in the buffer
}
QT_END_NAMESPACE
