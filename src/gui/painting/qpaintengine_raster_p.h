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

#ifndef QPAINTENGINE_RASTER_P_H
#define QPAINTENGINE_RASTER_P_H

#include <qpaintengineex_p.h>
#include <QtGui/qpainterpath.h>
#include <qdatabuffer_p.h>
#include <qdrawhelper_p.h>
#include <qpaintengine_p.h>
#include <qrasterizer_p.h>
#include <qstroker_p.h>
#include <qpainter_p.h>
#include <qtextureglyphcache_p.h>
#include <qoutlinemapper_p.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

class QOutlineMapper;
class QRasterPaintEnginePrivate;
class QRasterBuffer;
class QClipData;
class QCustomRasterPaintDevice;

class QRasterPaintEngineState : public QPainterState
{
 public:
   QRasterPaintEngineState(QRasterPaintEngineState &other);
   QRasterPaintEngineState();
   ~QRasterPaintEngineState();


   QPen lastPen;
   QSpanData penData;
   QStrokerOps *stroker;
   uint strokeFlags;

   QBrush lastBrush;
   QSpanData brushData;
   uint fillFlags;

   uint pixmapFlags;
   int intOpacity;

   qreal txscale;

   QClipData *clip;
   //     QRect clipRect;
   //     QRegion clipRegion;

   //     QPainter::RenderHints hints;
   //     QPainter::CompositionMode compositionMode;

   uint dirty;

   struct Flags {
      uint has_clip_ownership : 1;        // should delete the clip member..
      uint fast_pen : 1;                  // cosmetic 1-width pens, using midpoint drawlines
      uint non_complex_pen : 1;           // can use rasterizer, rather than stroker
      uint antialiased : 1;
      uint bilinear : 1;
      uint fast_text : 1;
      uint int_xform : 1;
      uint tx_noshear : 1;
      uint fast_images : 1;
   };

   union {
      Flags flags;
      uint flag_bits;
   };
};




/*******************************************************************************
 * QRasterPaintEngine
 */
class
#ifdef Q_WS_QWS
   Q_GUI_EXPORT
#endif
   QRasterPaintEngine : public QPaintEngineEx
{
   Q_DECLARE_PRIVATE(QRasterPaintEngine)
 public:

   QRasterPaintEngine(QPaintDevice *device);
   ~QRasterPaintEngine();

   bool begin(QPaintDevice *device) override;
   bool end() override;

   void penChanged() override;
   void brushChanged() override;
   void brushOriginChanged() override;
   void opacityChanged() override;
   void compositionModeChanged() override;
   void renderHintsChanged() override;
   void transformChanged() override;
   void clipEnabledChanged() override;

   void setState(QPainterState *s) override;
   QPainterState *createState(QPainterState *orig) const override;

   inline QRasterPaintEngineState *state() {
      return static_cast<QRasterPaintEngineState *>(QPaintEngineEx::state());
   }

   inline const QRasterPaintEngineState *state() const {
      return static_cast<const QRasterPaintEngineState *>(QPaintEngineEx::state());
   }

   void updateBrush(const QBrush &brush);
   void updatePen(const QPen &pen);

   void updateMatrix(const QTransform &matrix);

   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;
   void fillPath(const QPainterPath &path, QSpanData *fillData);
   void fillPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

   void drawEllipse(const QRectF &rect) override;

   void fillRect(const QRectF &rect, const QBrush &brush) override;
   void fillRect(const QRectF &rect, const QColor &color) override;

   void drawRects(const QRect  *rects, int rectCount) override;
   void drawRects(const QRectF *rects, int rectCount) override;

   void drawPixmap(const QPointF &p, const QPixmap &pm) override;
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;

   void drawImage(const QPointF &p, const QImage &img);
   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags = Qt::AutoColor) override;

   void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &sr) override;
   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;

   void drawLines(const QLine *line, int lineCount) override;
   void drawLines(const QLineF *line, int lineCount) override;

   void drawPoints(const QPointF *points, int pointCount) override;
   void drawPoints(const QPoint *points, int pointCount) override;

   void stroke(const QVectorPath &path, const QPen &pen) override;
   void fill(const QVectorPath &path, const QBrush &brush) override;

   void clip(const QVectorPath &path, Qt::ClipOperation op) override;
   void clip(const QRect &rect, Qt::ClipOperation op) override;
   void clip(const QRegion &region, Qt::ClipOperation op) override;

   void drawStaticTextItem(QStaticTextItem *textItem) override;

   enum ClipType {
      RectClip,
      ComplexClip
   };

   ClipType clipType() const;
   QRect clipBoundingRect() const;

   using QPaintEngineEx::drawPolygon;
   using QPaintEngineEx::drawEllipse;

   void releaseBuffer();

   QSize size() const;

#ifndef QT_NO_DEBUG
   void saveBuffer(const QString &s) const;
#endif

#ifdef Q_OS_MAC
   void setCGContext(CGContextRef ref);
   CGContextRef getCGContext() const;
#endif

#ifdef Q_OS_WIN
   void setDC(HDC hdc);
   HDC getDC() const override;
   void releaseDC(HDC hdc) const override;
#endif

   void alphaPenBlt(const void *src, int bpl, int depth, int rx, int ry, int w, int h);

   Type type() const  override{
      return Raster;
   }

   QPoint coordinateOffset() const override;

#if defined(Q_WS_QWS) && !defined(QT_NO_RASTERCALLBACKS)
   virtual void drawColorSpans(const QSpan *spans, int count, uint color);
   virtual void drawBufferSpan(const uint *buffer, int bufsize,
                               int x, int y, int length, uint const_alpha);
#endif
   bool supportsTransformations(const QFontEngine *fontEngine) const;
   bool supportsTransformations(qreal pixelSize, const QTransform &m) const override;

 protected:
   QRasterPaintEngine(QRasterPaintEnginePrivate &d, QPaintDevice *);

 private:
   friend struct QSpanData;
   friend class QBlitterPaintEngine;
   friend class QBlitterPaintEnginePrivate;

   void init();

   void fillRect(const QRectF &rect, QSpanData *data);
   void drawBitmap(const QPointF &pos, const QImage &image, QSpanData *fill);

   bool drawCachedGlyphs(int numGlyphs, const glyph_t *glyphs, const QFixedPoint *positions,
                         QFontEngine *fontEngine);

   bool setClipRectInDeviceCoords(const QRect &r, Qt::ClipOperation op);

   inline void ensureBrush(const QBrush &brush) {
      if (!qbrush_fast_equals(state()->lastBrush, brush) || (brush.style() != Qt::NoBrush && state()->fillFlags)) {
         updateBrush(brush);
      }
   }
   inline void ensureBrush() {
      ensureBrush(state()->brush);
   }

   inline void ensurePen(const QPen &pen) {
      if (!qpen_fast_equals(state()->lastPen, pen) || (pen.style() != Qt::NoPen && state()->strokeFlags)) {
         updatePen(pen);
      }
   }
   inline void ensurePen() {
      ensurePen(state()->pen);
   }

   void updateOutlineMapper();
   inline void ensureOutlineMapper();

   void updateState();
   inline void ensureState() {
      if (state()->dirty) {
         updateState();
      }
   }
};

#ifdef Q_WS_QWS
   class Q_GUI_EXPORT QRasterPaintEnginePrivate : public QPaintEngineExPrivate
#else
   class QRasterPaintEnginePrivate : public QPaintEngineExPrivate
#endif
{
   Q_DECLARE_PUBLIC(QRasterPaintEngine)

 public:
   QRasterPaintEnginePrivate();

   void rasterizeLine_dashed(QLineF line, qreal width,
                             int *dashIndex, qreal *dashOffset, bool *inDash);
   void rasterize(QT_FT_Outline *outline, ProcessSpans callback, QSpanData *spanData, QRasterBuffer *rasterBuffer);
   void rasterize(QT_FT_Outline *outline, ProcessSpans callback, void *userData, QRasterBuffer *rasterBuffer);
   void updateMatrixData(QSpanData *spanData, const QBrush &brush, const QTransform &brushMatrix);

   void systemStateChanged() override;

   void drawImage(const QPointF &pt, const QImage &img, SrcOverBlendFunc func,
                  const QRect &clip, int alpha, const QRect &sr = QRect());

   QTransform brushMatrix() const {
      Q_Q(const QRasterPaintEngine);
      const QRasterPaintEngineState *s = q->state();
      QTransform m(s->matrix);
      m.translate(s->brushOrigin.x(), s->brushOrigin.y());
      return m;
   }

   bool isUnclipped_normalized(const QRect &rect) const;
   bool isUnclipped(const QRect &rect, int penWidth) const;
   bool isUnclipped(const QRectF &rect, int penWidth) const;
   ProcessSpans getPenFunc(const QRectF &rect, const QSpanData *data) const;
   ProcessSpans getBrushFunc(const QRect &rect, const QSpanData *data) const;
   ProcessSpans getBrushFunc(const QRectF &rect, const QSpanData *data) const;

#ifdef Q_WS_QWS
   void prepare(QCustomRasterPaintDevice *);
#endif

   inline const QClipData *clip() const;

   void initializeRasterizer(QSpanData *data);

   void recalculateFastImages();
   bool canUseFastImageBlending(QPainter::CompositionMode mode, const QImage &image) const;

   QPaintDevice *device;
   QScopedPointer<QOutlineMapper> outlineMapper;
   QScopedPointer<QRasterBuffer>  rasterBuffer;

#if defined (Q_OS_WIN)
   HDC hdc;
#elif defined(Q_OS_MAC)
   CGContextRef cgContext;
#endif

   QRect deviceRect;
   QRect deviceRectUnclipped;

   QStroker basicStroker;
   QScopedPointer<QDashStroker> dashStroker;

   QScopedPointer<QT_FT_Raster> grayRaster;

   QDataBuffer<QLineF> cachedLines;
   QSpanData image_filler;
   QSpanData image_filler_xform;
   QSpanData solid_color_filler;


   QFontEngineGlyphCache::Type glyphCacheType;

   QScopedPointer<QClipData> baseClip;

   int deviceDepth;

   uint mono_surface : 1;
   uint outlinemapper_xform_dirty : 1;

#ifdef Q_OS_WIN
   uint isPlain45DegreeRotation : 1;
#endif

   QScopedPointer<QRasterizer> rasterizer;
};

#ifdef Q_WS_QWS
   class Q_GUI_EXPORT QClipData
#else 
   class QClipData
#endif
   
{
 public:
   QClipData(int height);
   ~QClipData();

   int clipSpanHeight;
   struct ClipLine {
      int count;
      QSpan *spans;
   } *m_clipLines;

   void initialize();

   inline ClipLine *clipLines() {
      if (!m_clipLines) {
         initialize();
      }
      return m_clipLines;
   }

   inline QSpan *spans() {
      if (!m_spans) {
         initialize();
      }
      return m_spans;
   }

   int allocated;
   int count;
   QSpan *m_spans;
   int xmin, xmax, ymin, ymax;

   QRect clipRect;
   QRegion clipRegion;

   uint enabled : 1;
   uint hasRectClip : 1;
   uint hasRegionClip : 1;

   void appendSpan(int x, int length, int y, int coverage);
   void appendSpans(const QSpan *s, int num);

   // ### Should optimize and actually kill the QSpans if the rect is
   // ### a subset of The current region. Thus the "fast" clipspan
   // ### callback can be used
   void setClipRect(const QRect &rect);
   void setClipRegion(const QRegion &region);
   void fixup();
};

inline void QClipData::appendSpan(int x, int length, int y, int coverage)
{
   Q_ASSERT(m_spans); // initialize() has to be called prior to adding spans..

   if (count == allocated) {
      allocated *= 2;
      m_spans = (QSpan *)realloc(m_spans, allocated * sizeof(QSpan));
   }
   m_spans[count].x = x;
   m_spans[count].len = length;
   m_spans[count].y = y;
   m_spans[count].coverage = coverage;
   ++count;
}

inline void QClipData::appendSpans(const QSpan *s, int num)
{
   Q_ASSERT(m_spans);

   if (count + num > allocated) {
      do {
         allocated *= 2;
      } while (count + num > allocated);
      m_spans = (QSpan *)realloc(m_spans, allocated * sizeof(QSpan));
   }
   memcpy(m_spans + count, s, num * sizeof(QSpan));
   count += num;
}

#ifdef Q_WS_QWS
class Q_GUI_EXPORT QCustomRasterPaintDevice : public QPaintDevice
{
 public:
   QCustomRasterPaintDevice(QWidget *w) : widget(w) {}

   int devType() const {
      return QInternal::CustomRaster;
   }

   virtual int metric(PaintDeviceMetric m) const;

   virtual void *memory() const {
      return 0;
   }

   virtual QImage::Format format() const {
      return QImage::Format_ARGB32_Premultiplied;
   }

   virtual int bytesPerLine() const;

   virtual QSize size() const {
      return static_cast<QRasterPaintEngine *>(paintEngine())->size();
   }

 private:
   QWidget *widget;
};
#endif


#ifdef Q_WS_QWS
   class Q_GUI_EXPORT QRasterBuffer
#else
   class QRasterBuffer
#endif
{
 public:
   QRasterBuffer() : m_width(0), m_height(0), m_buffer(0) {
      init();
   }

   ~QRasterBuffer();

   void init();

   QImage::Format prepare(QImage *image);
   QImage::Format prepare(QPixmap *pix);

#ifdef Q_WS_QWS
   void prepare(QCustomRasterPaintDevice *device);
#endif

   void prepare(int w, int h);
   void prepareBuffer(int w, int h);

   void resetBuffer(int val = 0);

   uchar *scanLine(int y) {
      Q_ASSERT(y >= 0);
      Q_ASSERT(y < m_height);
      return m_buffer + y * bytes_per_line;
   }

#ifndef QT_NO_DEBUG
   QImage bufferImage() const;
#endif

   void flushToARGBImage(QImage *image) const;

   int width() const {
      return m_width;
   }
   int height() const {
      return m_height;
   }
   int bytesPerLine() const {
      return bytes_per_line;
   }
   int bytesPerPixel() const {
      return bytes_per_pixel;
   }

   uchar *buffer() const {
      return m_buffer;
   }

   bool monoDestinationWithClut;
   QRgb destColor0;
   QRgb destColor1;

   QPainter::CompositionMode compositionMode;
   QImage::Format format;
   DrawHelper *drawHelper;
   QImage colorizeBitmap(const QImage &image, const QColor &color);

 private:
   int m_width;
   int m_height;
   int bytes_per_line;
   int bytes_per_pixel;
   uchar *m_buffer;
};

inline void QRasterPaintEngine::ensureOutlineMapper()
{
   if (d_func()->outlinemapper_xform_dirty) {
      updateOutlineMapper();
   }
}

inline const QClipData *QRasterPaintEnginePrivate::clip() const
{
   Q_Q(const QRasterPaintEngine);
   if (q->state() && q->state()->clip && q->state()->clip->enabled) {
      return q->state()->clip;
   }
   return baseClip.data();
}


QT_END_NAMESPACE
#endif // QPAINTENGINE_RASTER_P_H
