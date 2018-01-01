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

#include "qdirectfbscreen.h"
#include "qdirectfbpaintdevice.h"
#include "qdirectfbpaintengine.h"

#ifndef QT_NO_QWS_DIRECTFB

QT_BEGIN_NAMESPACE

QDirectFBPaintDevice::QDirectFBPaintDevice(QDirectFBScreen *scr)
    : QCustomRasterPaintDevice(0), dfbSurface(0), screen(scr),
      bpl(-1), lockFlgs(DFBSurfaceLockFlags(0)), mem(0), engine(0), imageFormat(QImage::Format_Invalid)
{
#ifdef QT_DIRECTFB_SUBSURFACE
    subSurface = 0;
    syncPending = false;
#endif
}

QDirectFBPaintDevice::~QDirectFBPaintDevice()
{
    if (QDirectFBScreen::instance()) {
        unlockSurface();
#ifdef QT_DIRECTFB_SUBSURFACE
        releaseSubSurface();
#endif
        if (dfbSurface) {
            screen->releaseDFBSurface(dfbSurface);
        }
    }
    delete engine;
}

IDirectFBSurface *QDirectFBPaintDevice::directFBSurface() const
{
    return dfbSurface;
}

bool QDirectFBPaintDevice::lockSurface(DFBSurfaceLockFlags lockFlags)
{
    if (lockFlgs && (lockFlags & ~lockFlgs))
        unlockSurface();
    if (!mem) {
        Q_ASSERT(dfbSurface);
#ifdef QT_DIRECTFB_SUBSURFACE
        if (!subSurface) {
            DFBResult result;
            subSurface = screen->getSubSurface(dfbSurface, QRect(), QDirectFBScreen::TrackSurface, &result);
            if (result != DFB_OK || !subSurface) {
                DirectFBError("Couldn't create sub surface", result);
                return false;
            }
        }
        IDirectFBSurface *surface = subSurface;
#else
        IDirectFBSurface *surface = dfbSurface;
#endif
        Q_ASSERT(surface);
        mem = QDirectFBScreen::lockSurface(surface, lockFlags, &bpl);
        lockFlgs = lockFlags;
        Q_ASSERT(mem);
        Q_ASSERT(bpl > 0);
        const QSize s = size();
        lockedImage = QImage(mem, s.width(), s.height(), bpl,
                             QDirectFBScreen::getImageFormat(dfbSurface));
        return true;
    }
#ifdef QT_DIRECTFB_SUBSURFACE
    if (syncPending) {
        syncPending = false;
        screen->waitIdle();
    }
#endif
    return false;
}

void QDirectFBPaintDevice::unlockSurface()
{
    if (QDirectFBScreen::instance() && lockFlgs) {
#ifdef QT_DIRECTFB_SUBSURFACE
        IDirectFBSurface *surface = subSurface;
#else
        IDirectFBSurface *surface = dfbSurface;
#endif
        if (surface) {
            surface->Unlock(surface);
            lockFlgs = static_cast<DFBSurfaceLockFlags>(0);
            mem = 0;
        }
    }
}

void *QDirectFBPaintDevice::memory() const
{
    return mem;
}

QImage::Format QDirectFBPaintDevice::format() const
{
    return imageFormat;
}

int QDirectFBPaintDevice::bytesPerLine() const
{
    Q_ASSERT(!mem || bpl != -1);
    return bpl;
}

QSize QDirectFBPaintDevice::size() const
{
    int w, h;
    dfbSurface->GetSize(dfbSurface, &w, &h);
    return QSize(w, h);
}

int QDirectFBPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    if (!dfbSurface)
        return 0;

    switch (metric) {
    case QPaintDevice::PdmWidth:
    case QPaintDevice::PdmHeight:
        return (metric == PdmWidth ? size().width() : size().height());
    case QPaintDevice::PdmWidthMM:
        return (size().width() * 1000) / dotsPerMeterX();
    case QPaintDevice::PdmHeightMM:
        return (size().height() * 1000) / dotsPerMeterY();
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmDpiX:
        return (dotsPerMeterX() * 254) / 10000; // 0.0254 meters-per-inch
    case QPaintDevice::PdmPhysicalDpiY:
    case QPaintDevice::PdmDpiY:
        return (dotsPerMeterY() * 254) / 10000; // 0.0254 meters-per-inch
    case QPaintDevice::PdmDepth:
        return QDirectFBScreen::depth(imageFormat);
    case QPaintDevice::PdmNumColors: {
        if (!lockedImage.isNull())
            return lockedImage.colorCount();

        DFBResult result;
        IDirectFBPalette *palette = 0;
        unsigned int numColors = 0;

        result = dfbSurface->GetPalette(dfbSurface, &palette);
        if ((result != DFB_OK) || !palette)
            return 0;

        result = palette->GetSize(palette, &numColors);
        palette->Release(palette);
        if (result != DFB_OK)
            return 0;

        return numColors;
    }
    default:
        qCritical("QDirectFBPaintDevice::metric(): Unhandled metric!");
        return 0;
    }
}

QPaintEngine *QDirectFBPaintDevice::paintEngine() const
{
    return engine;
}

#ifdef QT_DIRECTFB_SUBSURFACE
void QDirectFBPaintDevice::releaseSubSurface()
{
    Q_ASSERT(QDirectFBScreen::instance());
    if (subSurface) {
        unlockSurface();
        screen->releaseDFBSurface(subSurface);
        subSurface = 0;
    }
}
#endif

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB
