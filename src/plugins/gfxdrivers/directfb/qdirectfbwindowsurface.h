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

#ifndef QDIRECFBWINDOWSURFACE_H
#define QDIRECFBWINDOWSURFACE_H

#include <qdirectfbpaintengine.h>
#include <qdirectfbpaintdevice.h>
#include <qdirectfbscreen.h>

#ifndef QT_NO_QWS_DIRECTFB

#include <qpaintengine_raster_p.h>
#include <qwindowsurface_qws_p.h>

#ifdef QT_DIRECTFB_TIMING
#include <qdatetime.h>
#endif

QT_BEGIN_NAMESPACE

class QDirectFBWindowSurface : public QWSWindowSurface, public QDirectFBPaintDevice
{

public:
    QDirectFBWindowSurface(DFBSurfaceFlipFlags flipFlags, QDirectFBScreen *scr);
    QDirectFBWindowSurface(DFBSurfaceFlipFlags flipFlags, QDirectFBScreen *scr, QWidget *widget);
    ~QDirectFBWindowSurface();

#ifdef QT_DIRECTFB_WM
    void raise();
#endif
    bool isValid() const;

    void setGeometry(const QRect &rect);

    QString key() const { return QLatin1String("directfb"); }
    QByteArray permanentState() const;
    void setPermanentState(const QByteArray &state);

    bool scroll(const QRegion &area, int dx, int dy);

    bool move(const QPoint &offset);

    QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return this; }

    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    void beginPaint(const QRegion &);
    void endPaint(const QRegion &);

    IDirectFBSurface *surfaceForWidget(const QWidget *widget, QRect *rect) const;
    IDirectFBSurface *directFBSurface() const;

#ifdef QT_DIRECTFB_WM
    IDirectFBWindow *directFBWindow() const;
#endif

private:
    void updateIsOpaque();
    void setOpaque(bool opaque);
    void releaseSurface();

#ifdef QT_DIRECTFB_WM
    void createWindow(const QRect &rect);
    IDirectFBWindow *dfbWindow;
#else
    enum Mode {
        Primary,
        Offscreen
    } mode;
#endif

    DFBSurfaceFlipFlags flipFlags;
    bool boundingRectFlip;
    bool flushPending;
#ifdef QT_DIRECTFB_TIMING
    int frames;
    QTime timer;
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB

#endif // QDIRECFBWINDOWSURFACE_H
