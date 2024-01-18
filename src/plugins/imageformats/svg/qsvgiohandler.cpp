/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
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

#include <qsvgiohandler.h>

#ifndef QT_NO_SVGRENDERER

#include <qbuffer.h>
#include <qdebug.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsvgrenderer.h>
#include <qvariant.h>

class QSvgIOHandlerPrivate
{
public:
    QSvgIOHandlerPrivate(QSvgIOHandler *qq)
        : q(qq), loaded(false), readDone(false), backColor(Qt::transparent)
    {
    }

    bool load(QIODevice *device);

    QSvgIOHandler   *q;
    QSvgRenderer     r;
    QXmlStreamReader xmlReader;
    QSize            defaultSize;
    QRect            clipRect;
    QSize            scaledSize;
    QRect            scaledClipRect;
    bool             loaded;
    bool             readDone;
    QColor           backColor;
};

bool QSvgIOHandlerPrivate::load(QIODevice *device)
{
    if (loaded)
        return true;

    if (q->format().isEmpty())
        q->canRead();

    // # The SVG renderer doesn't handle trailing, unrelated data, so we must
    // assume that all available data in the device is to be read.
    bool res = false;
    QBuffer *buf = dynamic_cast<QBuffer *>(device);

    if (buf) {
        const QByteArray &ba = buf->data();
        res = r.load(QByteArray::fromRawData(ba.constData() + buf->pos(), ba.size() - buf->pos()));
        buf->seek(ba.size());
    } else if (q->format() == "svgz") {
        res = r.load(device->readAll());
    } else {
        xmlReader.setDevice(device);
        res = r.load(&xmlReader);
    }

    if (res) {
        defaultSize = QSize(r.viewBox().width(), r.viewBox().height());
        loaded = true;
    }

    return loaded;
}

QSvgIOHandler::QSvgIOHandler()
    : d(new QSvgIOHandlerPrivate(this))
{

}

QSvgIOHandler::~QSvgIOHandler()
{
    delete d;
}

bool QSvgIOHandler::canRead()
{
   if (! device()) {
     return false;
   }

   if (d->loaded && !d->readDone) {
     return true;        // happens if we have been asked for the size
   }

   QByteArray buf = device()->peek(8);
   if (buf.startsWith("\x1f\x8b")) {
     setFormat("svgz");
     return true;

   } else if (buf.contains("<?xml") || buf.contains("<svg") || buf.contains("<!--")) {
     setFormat("svg");
     return true;
   }

   return false;
}

QString QSvgIOHandler::name() const
{
   return "svg";
}

bool QSvgIOHandler::read(QImage *image)
{
    if (! d->readDone && d->load(device())) {
        bool xform = (d->clipRect.isValid() || d->scaledSize.isValid() || d->scaledClipRect.isValid());
        QSize finalSize = d->defaultSize;
        QRectF bounds;

        if (xform && !d->defaultSize.isEmpty()) {
            bounds = QRectF(QPointF(0,0), QSizeF(d->defaultSize));
            QPoint tr1, tr2;
            QSizeF sc(1, 1);

            if (d->clipRect.isValid()) {
                tr1 = -d->clipRect.topLeft();
                finalSize = d->clipRect.size();
            }

            if (d->scaledSize.isValid()) {
                sc = QSizeF(qreal(d->scaledSize.width()) / finalSize.width(),
                            qreal(d->scaledSize.height()) / finalSize.height());
                finalSize = d->scaledSize;
            }

            if (d->scaledClipRect.isValid()) {
                tr2 = -d->scaledClipRect.topLeft();
                finalSize = d->scaledClipRect.size();
            }

            QTransform t;
            t.translate(tr2.x(), tr2.y());
            t.scale(sc.width(), sc.height());
            t.translate(tr1.x(), tr1.y());
            bounds = t.mapRect(bounds);
        }

        *image = QImage(finalSize, QImage::Format_ARGB32_Premultiplied);

        if (!finalSize.isEmpty()) {
            image->fill(d->backColor.rgba());
            QPainter p(image);
            d->r.render(&p, bounds);
            p.end();
        }

        d->readDone = true;
        return true;
    }

    return false;
}

QVariant QSvgIOHandler::option(ImageOption option)
{
    switch(option) {
       case ImageFormat:
           return QImage::Format_ARGB32_Premultiplied;
           break;

       case Size:
           d->load(device());
           return d->defaultSize;
           break;

       case ClipRect:
           return d->clipRect;
           break;

       case ScaledSize:
           return d->scaledSize;
           break;

       case ScaledClipRect:
           return d->scaledClipRect;
           break;

       case BackgroundColor:
           return d->backColor;
           break;

       default:
           break;
    }

    return QVariant();
}

void QSvgIOHandler::setOption(ImageOption option, const QVariant & value)
{
    switch(option) {
       case ClipRect:
           d->clipRect = value.toRect();
           break;

       case ScaledSize:
           d->scaledSize = value.toSize();
           break;

       case ScaledClipRect:
           d->scaledClipRect = value.toRect();
           break;

       case BackgroundColor:
           d->backColor = value.value<QColor>();
           break;

       default:
           break;
    }
}

bool QSvgIOHandler::supportsOption(ImageOption option) const
{
    switch(option)     {
       case ImageFormat:
       case Size:
       case ClipRect:
       case ScaledSize:
       case ScaledClipRect:
       case BackgroundColor:
           return true;

       default:
           break;
    }

    return false;
}


bool QSvgIOHandler::canRead(QIODevice *device)
{
    QByteArray buf = device->peek(8);
    return buf.startsWith("\x1f\x8b") || buf.contains("<?xml") || buf.contains("<svg") || buf.contains("<!--");
}

#endif
