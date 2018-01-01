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

#ifndef QDIRECTFBPAINTDEVICE_H
#define QDIRECTFBPAINTDEVICE_H

#include <qpaintengine_raster_p.h>
#include <qdirectfbscreen.h>

#ifndef QT_NO_QWS_DIRECTFB

QT_BEGIN_NAMESPACE

// Inherited by both window surface and pixmap
class QDirectFBPaintEngine;

class QDirectFBPaintDevice : public QCustomRasterPaintDevice
{
public:
    ~QDirectFBPaintDevice();

    virtual IDirectFBSurface *directFBSurface() const;

    bool lockSurface(DFBSurfaceLockFlags lockFlags);
    void unlockSurface();

    // Reimplemented from QCustomRasterPaintDevice:
    void *memory() const;
    QImage::Format format() const;
    int bytesPerLine() const;
    QSize size() const;
    int metric(QPaintDevice::PaintDeviceMetric metric) const;
    DFBSurfaceLockFlags lockFlags() const { return lockFlgs; }
    QPaintEngine *paintEngine() const;

protected:
    QDirectFBPaintDevice(QDirectFBScreen *scr);

    inline int dotsPerMeterX() const
    {
        return (screen->deviceWidth() * 1000) / screen->physicalWidth();
    }
    inline int dotsPerMeterY() const
    {
        return (screen->deviceHeight() * 1000) / screen->physicalHeight();
    }

    IDirectFBSurface *dfbSurface;

#ifdef QT_DIRECTFB_SUBSURFACE
    void releaseSubSurface();
    IDirectFBSurface *subSurface;
    friend class QDirectFBPaintEnginePrivate;
    bool syncPending;
#endif

    QImage lockedImage;
    QDirectFBScreen *screen;
    int bpl;
    DFBSurfaceLockFlags lockFlgs;
    uchar *mem;
    QDirectFBPaintEngine *engine;
    QImage::Format imageFormat;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB
#endif //QDIRECTFBPAINTDEVICE_H
