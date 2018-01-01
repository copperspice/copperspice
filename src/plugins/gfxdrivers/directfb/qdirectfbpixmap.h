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

#ifndef QDIRECTFBPIXMAP_H
#define QDIRECTFBPIXMAP_H

#include <qglobal.h>

#ifndef QT_NO_QWS_DIRECTFB

#include <qpixmapdata_p.h>
#include <qpaintengine_raster_p.h>
#include <qdirectfbpaintdevice.h>
#include <directfb.h>

QT_BEGIN_NAMESPACE

class QDirectFBPaintEngine;

class QDirectFBPixmapData : public QPixmapData, public QDirectFBPaintDevice
{

public:
    QDirectFBPixmapData(QDirectFBScreen *screen, PixelType pixelType);
    ~QDirectFBPixmapData();

    // Re-implemented from QPixmapData:
    virtual void resize(int width, int height);
    virtual void fromImage(const QImage &image, Qt::ImageConversionFlags flags);

#ifdef QT_DIRECTFB_IMAGEPROVIDER
    virtual bool fromFile(const QString &filename, const char *format,Qt::ImageConversionFlags flags);
    virtual bool fromData(const uchar *buffer, uint len, const char *format,Qt::ImageConversionFlags flags);
#endif

    virtual void copy(const QPixmapData *data, const QRect &rect);
    virtual void fill(const QColor &color);
    virtual QPixmap transformed(const QTransform &matrix,
                                Qt::TransformationMode mode) const;
    virtual QImage toImage() const;
    virtual QPaintEngine *paintEngine() const;
    virtual QImage *buffer();
    virtual bool scroll(int dx, int dy, const QRect &rect);
    // Pure virtual in QPixmapData, so re-implement here and delegate to QDirectFBPaintDevice
    virtual int metric(QPaintDevice::PaintDeviceMetric m) const { return QDirectFBPaintDevice::metric(m); }

    inline QImage::Format pixelFormat() const { return imageFormat; }
    inline bool hasAlphaChannel() const { return alpha; }
    static bool hasAlphaChannel(const QImage &img, Qt::ImageConversionFlags flags = Qt::AutoColor);

private:

#ifdef QT_DIRECTFB_IMAGEPROVIDER
    bool fromDataBufferDescription(const DFBDataBufferDescription &dataBuffer);
#endif
    void invalidate();
    bool alpha;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB

#endif // QDIRECTFBPIXMAP_H
