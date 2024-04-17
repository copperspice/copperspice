/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
* Copyright (c) 2015 The Qt Company Ltd.
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

#include <qopengl_paintdevice.h>

#include <qpaintengine.h>
#include <qthreadstorage.h>
#include <qopenglfunctions.h>

#include <qopengl_paintdevice_p.h>
#include <qopengl_framebufferobject_p.h>
#include <qopengl_paintengine_p.h>
#include <qopenglcontext_p.h>

// for qt_defaultDpiX/Y
#include <qfont_p.h>

QOpenGLPaintDevice::QOpenGLPaintDevice()
    : d_ptr(new QOpenGLPaintDevicePrivate(QSize()))
{
}

QOpenGLPaintDevice::QOpenGLPaintDevice(const QSize &size)
    : d_ptr(new QOpenGLPaintDevicePrivate(size))
{
}

QOpenGLPaintDevice::QOpenGLPaintDevice(int width, int height)
    : d_ptr(new QOpenGLPaintDevicePrivate(QSize(width, height)))
{
}

QOpenGLPaintDevice::QOpenGLPaintDevice(QOpenGLPaintDevicePrivate &dd)
    : d_ptr(&dd)
{
}

QOpenGLPaintDevice::~QOpenGLPaintDevice()
{
    delete d_ptr->engine;
}

QOpenGLPaintDevicePrivate::QOpenGLPaintDevicePrivate(const QSize &sz)
    : size(sz), ctx(QOpenGLContext::currentContext()), dpmx(qt_defaultDpiX() * 100. / 2.54),
      dpmy(qt_defaultDpiY() * 100. / 2.54), devicePixelRatio(1.0), flipped(false), engine(nullptr)
{
}

QOpenGLPaintDevicePrivate::~QOpenGLPaintDevicePrivate()
{
}

class QOpenGLEngineThreadStorage
{
 public:
    QPaintEngine *engine() {
        QPaintEngine *&localEngine = storage.localData();
        if (! localEngine) {
            localEngine = new QOpenGL2PaintEngineEx;
        }

        return localEngine;
    }

 private:
    QThreadStorage<QPaintEngine *> storage;
};

static QOpenGLEngineThreadStorage *qt_opengl_engine()
{
   static QOpenGLEngineThreadStorage retval;
   return &retval;
}

QPaintEngine *QOpenGLPaintDevice::paintEngine() const
{
    if (d_ptr->engine) {
        return d_ptr->engine;
    }

    QPaintEngine *engine = qt_opengl_engine()->engine();
    if (engine->isActive() && engine->paintDevice() != this) {
        d_ptr->engine = new QOpenGL2PaintEngineEx;
        return d_ptr->engine;
    }

    return engine;
}

QOpenGLContext *QOpenGLPaintDevice::context() const
{
    return d_ptr->ctx;
}

QSize QOpenGLPaintDevice::size() const
{
    return d_ptr->size;
}

void QOpenGLPaintDevice::setSize(const QSize &size)
{
    d_ptr->size = size;
}

void QOpenGLPaintDevice::setDevicePixelRatio(qreal devicePixelRatio)
{
    d_ptr->devicePixelRatio = devicePixelRatio;
}

int QOpenGLPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch (metric) {
       case PdmWidth:
           return d_ptr->size.width();
       case PdmHeight:
           return d_ptr->size.height();
       case PdmDepth:
           return 32;
       case PdmWidthMM:
           return qRound(d_ptr->size.width() * 1000 / d_ptr->dpmx);
       case PdmHeightMM:
           return qRound(d_ptr->size.height() * 1000 / d_ptr->dpmy);
       case PdmNumColors:
           return 0;
       case PdmDpiX:
           return qRound(d_ptr->dpmx * 0.0254);
       case PdmDpiY:
           return qRound(d_ptr->dpmy * 0.0254);
       case PdmPhysicalDpiX:
           return qRound(d_ptr->dpmx * 0.0254);
       case PdmPhysicalDpiY:
           return qRound(d_ptr->dpmy * 0.0254);
       case PdmDevicePixelRatio:
           return d_ptr->devicePixelRatio;
       case PdmDevicePixelRatioScaled:
           return d_ptr->devicePixelRatio * QPaintDevice::devicePixelRatioFScale();

       default:
           qWarning("QOpenGLPaintDevice::metric() - metric %d not known", metric);
           return 0;
    }
}

qreal QOpenGLPaintDevice::dotsPerMeterX() const
{
    return d_ptr->dpmx;
}

qreal QOpenGLPaintDevice::dotsPerMeterY() const
{
    return d_ptr->dpmy;
}

void QOpenGLPaintDevice::setDotsPerMeterX(qreal dpmx)
{
    d_ptr->dpmx = dpmx;
}

void QOpenGLPaintDevice::setDotsPerMeterY(qreal dpmy)
{
    d_ptr->dpmx = dpmy;
}

void QOpenGLPaintDevice::setPaintFlipped(bool flipped)
{
    d_ptr->flipped = flipped;
}

bool QOpenGLPaintDevice::paintFlipped() const
{
    return d_ptr->flipped;
}

void QOpenGLPaintDevice::ensureActiveTarget()
{
}
