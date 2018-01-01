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

#ifndef QPAINTENGINE_DIRECTFB_P_H
#define QPAINTENGINE_DIRECTFB_P_H

#include <QtGui/qpaintengine.h>
#include <qpaintengine_raster_p.h>

#ifndef QT_NO_QWS_DIRECTFB

QT_BEGIN_NAMESPACE

class QDirectFBPaintEnginePrivate;

class QDirectFBPaintEngine : public QRasterPaintEngine
{
    Q_DECLARE_PRIVATE(QDirectFBPaintEngine)

public:
    QDirectFBPaintEngine(QPaintDevice *device);
    virtual ~QDirectFBPaintEngine();

    virtual bool begin(QPaintDevice *device);
    virtual bool end();

    virtual void drawRects(const QRect  *rects, int rectCount);
    virtual void drawRects(const QRectF *rects, int rectCount);

    virtual void fillRect(const QRectF &r, const QBrush &brush);
    virtual void fillRect(const QRectF &r, const QColor &color);

    virtual void drawLines(const QLine *line, int lineCount);
    virtual void drawLines(const QLineF *line, int lineCount);

    virtual void drawImage(const QPointF &p, const QImage &img);
    virtual void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                           Qt::ImageConversionFlags falgs = Qt::AutoColor);

    virtual void drawPixmap(const QPointF &p, const QPixmap &pm);
    virtual void drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &sr);

    virtual void drawBufferSpan(const uint *buffer, int bufsize,
                                int x, int y, int length, uint const_alpha);

    virtual void stroke(const QVectorPath &path, const QPen &pen);
    virtual void drawPath(const QPainterPath &path);
    virtual void drawPoints(const QPointF *points, int pointCount);
    virtual void drawPoints(const QPoint *points, int pointCount);
    virtual void drawEllipse(const QRectF &rect);
    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
    virtual void drawTextItem(const QPointF &p, const QTextItem &textItem);
    virtual void fill(const QVectorPath &path, const QBrush &brush);
    virtual void drawRoundedRect(const QRectF &rect, qreal xrad, qreal yrad, Qt::SizeMode mode);

    virtual void clipEnabledChanged();
    virtual void brushChanged();
    virtual void penChanged();
    virtual void opacityChanged();
    virtual void compositionModeChanged();
    virtual void renderHintsChanged();
    virtual void transformChanged();

    virtual void setState(QPainterState *state);

    virtual void clip(const QVectorPath &path, Qt::ClipOperation op);
    virtual void clip(const QRegion &region, Qt::ClipOperation op);
    virtual void clip(const QRect &rect, Qt::ClipOperation op);

    virtual void drawStaticTextItem(QStaticTextItem *item);

    static void initImageCache(int size);
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_DIRECTFB

#endif // QPAINTENGINE_DIRECTFB_P_H
