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

#ifndef QPAINTENGINEEX_P_H
#define QPAINTENGINEEX_P_H

#include <QtGui/qpaintengine.h>
#include <QtGui/qdrawutil.h>
#include <qpaintengine_p.h>
#include <qstroker_p.h>
#include <qpainter_p.h>
#include <qvectorpath_p.h>

QT_BEGIN_NAMESPACE

class QPainterState;
class QPaintEngineExPrivate;
class QStaticTextItem;
struct StrokeHandler;

struct QIntRect {
   int x1, y1, x2, y2;
   inline void set(const QRect &r) {
      x1 = r.x();
      y1 = r.y();
      x2 = r.right() + 1;
      y2 = r.bottom() + 1;
      // We will assume normalized for later...
      Q_ASSERT(x2 >= x1);
      Q_ASSERT(y2 >= y1);
   }
};

class QRectVectorPath : public QVectorPath
{
 public:
   inline void set(const QRect &r) {
      qreal left = r.x();
      qreal right = r.x() + r.width();
      qreal top = r.y();
      qreal bottom = r.y() + r.height();
      pts[0] = left;
      pts[1] = top;
      pts[2] = right;
      pts[3] = top;
      pts[4] = right;
      pts[5] = bottom;
      pts[6] = left;
      pts[7] = bottom;
   }

   inline void set(const QRectF &r) {
      qreal left = r.x();
      qreal right = r.x() + r.width();
      qreal top = r.y();
      qreal bottom = r.y() + r.height();
      pts[0] = left;
      pts[1] = top;
      pts[2] = right;
      pts[3] = top;
      pts[4] = right;
      pts[5] = bottom;
      pts[6] = left;
      pts[7] = bottom;
   }
   inline QRectVectorPath(const QRect &r)
      : QVectorPath(pts, 4, 0, QVectorPath::RectangleHint | QVectorPath::ImplicitClose) {
      set(r);
   }
   inline QRectVectorPath(const QRectF &r)
      : QVectorPath(pts, 4, 0, QVectorPath::RectangleHint | QVectorPath::ImplicitClose) {
      set(r);
   }
   inline QRectVectorPath()
      : QVectorPath(pts, 4, 0, QVectorPath::RectangleHint | QVectorPath::ImplicitClose) {
   }

   qreal pts[8];
};

QDebug Q_GUI_EXPORT &operator<<(QDebug &, const QVectorPath &path);


class QPixmapFilter;

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

   void drawEllipse(const QRectF &r) override;
   void drawEllipse(const QRect &r) override;

   void drawPath(const QPainterPath &path) override;

   void drawPoints(const QPointF *points, int pointCount) override;
   void drawPoints(const QPoint *points, int pointCount) override;

   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;

   using QPaintEngine::drawPixmap;
   virtual void drawPixmap(const QPointF &pos, const QPixmap &pm);

   using QPaintEngine::drawImage;                 
   void drawImage(const QPointF &pos, const QImage &image);

   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;

   virtual void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                  QPainter::PixmapFragmentHints hints);

   virtual void drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects, int fragmentCount,
                  const QPixmap &pixmap, QPainter::PixmapFragmentHints hints);

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

   // Return a pixmap filter of "type" that can render the parameters
   // in "prototype".  The returned filter is owned by the engine and
   // will be destroyed when the engine is destroyed.  The "prototype"
   // allows the engine to pick different filters based on the parameters
   // that will be requested, and not just the "type".
   virtual QPixmapFilter *pixmapFilter(int /*type*/, const QPixmapFilter * /*prototype*/) {
      return 0;
   }

   // These flags are needed in the implementation of paint buffers.
   enum Flags {
      DoNotEmulate = 0x01,        // If set, QPainter will not wrap this engine in an emulation engine.
      IsEmulationEngine = 0x02    // If set, this object is a QEmulationEngine.
   };
   virtual uint flags() const {
      return 0;
   }
   virtual bool supportsTransformations(qreal pixelSize, const QTransform &m) const;

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

inline uint QVectorPath::polygonFlags(QPaintEngine::PolygonDrawMode mode)
{
   switch (mode) {
      case QPaintEngine::ConvexMode:
         return ConvexPolygonHint | ImplicitClose;
      case QPaintEngine::OddEvenMode:
         return PolygonHint | OddEvenFill | ImplicitClose;
      case QPaintEngine::WindingMode:
         return PolygonHint | WindingFill | ImplicitClose;
      case QPaintEngine::PolylineMode:
         return PolygonHint;
      default:
         return 0;
   }
}

QT_END_NAMESPACE

#endif
