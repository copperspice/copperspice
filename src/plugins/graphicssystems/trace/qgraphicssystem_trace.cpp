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

#include <qgraphicssystem_trace_p.h>
#include <qpixmap_raster_p.h>
#include <qpaintbuffer_p.h>
#include <qwindowsurface_raster_p.h>
#include <QFile>
#include <QPainter>
#include <QDebug>

QT_BEGIN_NAMESPACE

class QTraceWindowSurface : public QRasterWindowSurface
{
public:
    QTraceWindowSurface(QWidget *widget);
    ~QTraceWindowSurface();

    QPaintDevice *paintDevice();
    void beginPaint(const QRegion &rgn);
    void endPaint(const QRegion &rgn);

    bool scroll(const QRegion &area, int dx, int dy);

private:
    QPaintBuffer *buffer;
    QList<QRegion> updates;

    quint64 winId;
};

QTraceWindowSurface::QTraceWindowSurface(QWidget *widget)
    : QRasterWindowSurface(widget), buffer(0), winId(0)
{
}

QTraceWindowSurface::~QTraceWindowSurface()
{
    if (buffer) {
        QFile outputFile(QString(QLatin1String("qtgraphics-%0.trace")).arg(winId));
        if (outputFile.open(QIODevice::WriteOnly)) {
            QDataStream out(&outputFile);
            out.setFloatingPointPrecision(QDataStream::SinglePrecision);

            out.writeBytes("qttraceV2", 9);

            uint version = 1;

            out << version << *buffer << updates;
        }
        delete buffer;
    }
}

QPaintDevice *QTraceWindowSurface::paintDevice()
{
    if (!buffer) {
        buffer = new QPaintBuffer;
#ifdef Q_WS_QPA
        buffer->setBoundingRect(QRect(QPoint(), size()));
#else
        buffer->setBoundingRect(geometry());
#endif
    }
    return buffer;
}

void QTraceWindowSurface::beginPaint(const QRegion &rgn)
{
    // ensure paint buffer is created
    paintDevice();
    buffer->beginNewFrame();

    QRasterWindowSurface::beginPaint(rgn);
}

void QTraceWindowSurface::endPaint(const QRegion &rgn)
{
    QPainter p(QRasterWindowSurface::paintDevice());
    buffer->draw(&p, buffer->numFrames()-1);
    p.end();

    winId = (quint64)window()->winId();

    updates << rgn;

    QRasterWindowSurface::endPaint(rgn);
}

bool QTraceWindowSurface::scroll(const QRegion &, int, int)
{
    // TODO: scrolling should also be streamed and replayed
    // to test scrolling performance
    return false;
}

QTraceGraphicsSystem::QTraceGraphicsSystem()
{
}

QPixmapData *QTraceGraphicsSystem::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QWindowSurface *QTraceGraphicsSystem::createWindowSurface(QWidget *widget) const
{
    return new QTraceWindowSurface(widget);
}

QT_END_NAMESPACE
