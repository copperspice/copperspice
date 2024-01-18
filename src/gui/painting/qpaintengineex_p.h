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

#ifndef QPAINTENGINEEX_P_H
#define QPAINTENGINEEX_P_H

#include <qpaintengine.h>


#include <qpaintengine_p.h>
#include <qstroker_p.h>
#include <qpainter_p.h>
#include <qvectorpath_p.h>

class QPainterState;
class QPaintEngineExPrivate;
class QStaticTextItem;

struct StrokeHandler;


QDebug Q_GUI_EXPORT &operator<<(QDebug &, const QVectorPath &path);


class Q_GUI_EXPORT QPaintEngineEx : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QPaintEngineEx)

 public:
   QPaintEngineEx();

   virtual QPainterState *createState(QPainterState *orig) const;

   virtual void draw(const QVectorPath &path);
   virtual void fill(const QVectorPath &path, const QBrush &brush) = 0;
   virtual void stroke(const QVectorPath &path, const QPen &pen);

   virtual void clip(const QVectorPath &path, Qt::ClipOperation op) = 0;
   virtual void clip(const QRect &rect, Qt::ClipOperation op);
   virtual void clip(const QRegion &region, Qt::ClipOperation op);
   virtual void clip(const QPainterPath &path, Qt::ClipOperation op);

   virtual void clipEnabledChanged() = 0;
   virtual void penChanged() = 0;
   virtual void brushChanged() = 0;
   virtual void brushOriginChanged() = 0;
   virtual void opacityChanged() = 0;
   virtual void compositionModeChanged() = 0;
   virtual void renderHintsChanged() = 0;
   virtual void transformChanged() = 0;

   virtual void fillRect(const QRectF &rect, const QBrush &brush);
   virtual void fillRect(const QRectF &rect, const QColor &color);

   virtual void drawRoundedRect(const QRectF &rect, qreal xrad, qreal yrad, Qt::SizeMode mode);

   void drawRects(const QRect *rects, int rectCount) override;
   void drawRects(const QRectF *rects, int rectCount) override;

   void drawLines(const QLine *lines, int lineCount) override;
   void drawLines(const QLineF *lines, int lineCount) override;

   void drawEllipse(const QRectF &rect) override;
   void drawEllipse(const QRect &rect) override;

   void drawPath(const QPainterPath &path) override;

   void drawPoints(const QPointF *points, int pointCount) override;
   void drawPoints(const QPoint *points, int pointCount) override;

   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;

   void drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &srcRect) override = 0;
   virtual void drawPixmap(const QPointF &pos, const QPixmap &pixmap);

   void drawImage(const QRectF &rect, const QImage &pm, const QRectF &sr,
      Qt::ImageConversionFlags flags = Qt::AutoColor) override = 0;

   virtual void drawImage(const QPointF &point, const QImage &image);

   void drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &point) override;

   virtual void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
      QFlags<QPainter::PixmapFragmentHint> hints);

   void updateState(const QPaintEngineState &state) override;

   virtual void drawStaticTextItem(QStaticTextItem *);

   virtual void setState(QPainterState *s);
   inline QPainterState *state() {
      return static_cast<QPainterState *>(QPaintEngine::state);
   }

   inline const QPainterState *state() const {
      return static_cast<const QPainterState *>(QPaintEngine::state);
   }

   virtual void sync() {}
   virtual void beginNativePainting() {}
   virtual void endNativePainting() {}


   // These flags are needed in the implementation of paint buffers.
   enum Flags {
      DoNotEmulate      = 0x01,   // If set, QPainter will not wrap this engine in an emulation engine.
      IsEmulationEngine = 0x02    // If set, this object is a QEmulationEngine.
   };

   virtual uint flags() const {
      return 0;
   }

   virtual bool requiresPretransformedGlyphPositions(QFontEngine *fontEngine, const QTransform &m) const;
   virtual bool shouldDrawCachedGlyphs(QFontEngine *fontEngine, const QTransform &m) const;

 protected:
   QPaintEngineEx(QPaintEngineExPrivate &data);
};

class Q_GUI_EXPORT QPaintEngineExPrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QPaintEngineEx)

 public:
   QPaintEngineExPrivate();
   ~QPaintEngineExPrivate();

   void replayClipOperations();
   bool hasClipOperations() const;

   QStroker stroker;
   QDashStroker dasher;
   StrokeHandler *strokeHandler;
   QStrokerOps *activeStroker;
   QPen strokerPen;

   QRect exDeviceRect;
};



#endif
