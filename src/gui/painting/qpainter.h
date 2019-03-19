/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QPAINTER_H
#define QPAINTER_H

#include <QtCore/qnamespace.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qimage.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qdrawutil.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qpen.h>
#include <QtGui/qbrush.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfontinfo.h>
#include <QtGui/qfontmetrics.h>

QT_BEGIN_NAMESPACE

class QBrush;
class QFontInfo;
class QFontMetrics;
class QPaintDevice;
class QPainterPath;
class QPainterPrivate;
class QPen;
class QPolygon;
class QTextItem;
class QMatrix;
class QTransform;
class QStaticText;
class QGlyphRun;
class QPainterPrivateDeleter;

class Q_GUI_EXPORT QPainter
{
   GUI_CS_GADGET(QPainter)
   Q_DECLARE_PRIVATE(QPainter)

   GUI_CS_FLAG(RenderHint, RenderHints)

 public:
   enum RenderHint {
      Antialiasing = 0x01,
      TextAntialiasing = 0x02,
      SmoothPixmapTransform = 0x04,
      HighQualityAntialiasing = 0x08,
      NonCosmeticDefaultPen = 0x10
   };

   using RenderHints = QFlags<RenderHint>;

   class PixmapFragment
   {
    public:
      qreal x;
      qreal y;
      qreal sourceLeft;
      qreal sourceTop;
      qreal width;
      qreal height;
      qreal scaleX;
      qreal scaleY;
      qreal rotation;
      qreal opacity;
      static PixmapFragment Q_GUI_EXPORT create(const QPointF &pos, const QRectF &sourceRect,
            qreal scaleX = 1, qreal scaleY = 1,
            qreal rotation = 0, qreal opacity = 1);
   };

   enum PixmapFragmentHint {
      OpaqueHint = 0x01
   };

   using PixmapFragmentHints = QFlags<PixmapFragmentHint>;

   QPainter();
   explicit QPainter(QPaintDevice *);
   ~QPainter();

   QPaintDevice *device() const;

   bool begin(QPaintDevice *);
   bool end();
   bool isActive() const;

   void initFrom(const QWidget *widget);

   enum CompositionMode {
      CompositionMode_SourceOver,
      CompositionMode_DestinationOver,
      CompositionMode_Clear,
      CompositionMode_Source,
      CompositionMode_Destination,
      CompositionMode_SourceIn,
      CompositionMode_DestinationIn,
      CompositionMode_SourceOut,
      CompositionMode_DestinationOut,
      CompositionMode_SourceAtop,
      CompositionMode_DestinationAtop,
      CompositionMode_Xor,

      //svg 1.2 blend modes
      CompositionMode_Plus,
      CompositionMode_Multiply,
      CompositionMode_Screen,
      CompositionMode_Overlay,
      CompositionMode_Darken,
      CompositionMode_Lighten,
      CompositionMode_ColorDodge,
      CompositionMode_ColorBurn,
      CompositionMode_HardLight,
      CompositionMode_SoftLight,
      CompositionMode_Difference,
      CompositionMode_Exclusion,

      // ROPs
      RasterOp_SourceOrDestination,
      RasterOp_SourceAndDestination,
      RasterOp_SourceXorDestination,
      RasterOp_NotSourceAndNotDestination,
      RasterOp_NotSourceOrNotDestination,
      RasterOp_NotSourceXorDestination,
      RasterOp_NotSource,
      RasterOp_NotSourceAndDestination,
      RasterOp_SourceAndNotDestination
   };
   void setCompositionMode(CompositionMode mode);
   CompositionMode compositionMode() const;

   const QFont &font() const;
   void setFont(const QFont &f);

   QFontMetrics fontMetrics() const;
   QFontInfo fontInfo() const;

   void setPen(const QColor &color);
   void setPen(const QPen &pen);
   void setPen(Qt::PenStyle style);
   const QPen &pen() const;

   void setBrush(const QBrush &brush);
   void setBrush(Qt::BrushStyle style);
   const QBrush &brush() const;

   // attributes/modes
   void setBackgroundMode(Qt::BGMode mode);
   Qt::BGMode backgroundMode() const;

   QPoint brushOrigin() const;
   inline void setBrushOrigin(int x, int y);
   inline void setBrushOrigin(const QPoint &);
   void setBrushOrigin(const QPointF &);

   void setBackground(const QBrush &bg);
   const QBrush &background() const;

   qreal opacity() const;
   void setOpacity(qreal opacity);

   // Clip functions
   QRegion clipRegion() const;
   QPainterPath clipPath() const;

   void setClipRect(const QRectF &, Qt::ClipOperation op = Qt::ReplaceClip);
   void setClipRect(const QRect &, Qt::ClipOperation op = Qt::ReplaceClip);
   inline void setClipRect(int x, int y, int w, int h, Qt::ClipOperation op = Qt::ReplaceClip);

   void setClipRegion(const QRegion &, Qt::ClipOperation op = Qt::ReplaceClip);

   void setClipPath(const QPainterPath &path, Qt::ClipOperation op = Qt::ReplaceClip);

   void setClipping(bool enable);
   bool hasClipping() const;

   QRectF clipBoundingRect() const;

   void save();
   void restore();

   // XForm functions
   void setMatrix(const QMatrix &matrix, bool combine = false);
   const QMatrix &matrix() const;
   const QMatrix &deviceMatrix() const;
   void resetMatrix();

   void setTransform(const QTransform &transform, bool combine = false);
   const QTransform &transform() const;
   const QTransform &deviceTransform() const;
   void resetTransform();

   void setWorldMatrix(const QMatrix &matrix, bool combine = false);
   const QMatrix &worldMatrix() const;

   void setWorldTransform(const QTransform &matrix, bool combine = false);
   const QTransform &worldTransform() const;

   QMatrix combinedMatrix() const;
   QTransform combinedTransform() const;

   void setMatrixEnabled(bool enabled);
   bool matrixEnabled() const;

   void setWorldMatrixEnabled(bool enabled);
   bool worldMatrixEnabled() const;

   void scale(qreal sx, qreal sy);
   void shear(qreal sh, qreal sv);
   void rotate(qreal a);

   void translate(const QPointF &offset);
   inline void translate(const QPoint &offset);
   inline void translate(qreal dx, qreal dy);

   QRect window() const;
   void setWindow(const QRect &window);
   inline void setWindow(int x, int y, int w, int h);

   QRect viewport() const;
   void setViewport(const QRect &viewport);
   inline void setViewport(int x, int y, int w, int h);

   void setViewTransformEnabled(bool enable);
   bool viewTransformEnabled() const;

   // drawing functions
   void strokePath(const QPainterPath &path, const QPen &pen);
   void fillPath(const QPainterPath &path, const QBrush &brush);
   void drawPath(const QPainterPath &path);

   inline void drawPoint(const QPointF &pt);
   inline void drawPoint(const QPoint &p);
   inline void drawPoint(int x, int y);

   void drawPoints(const QPointF *points, int pointCount);
   inline void drawPoints(const QPolygonF &points);
   void drawPoints(const QPoint *points, int pointCount);
   inline void drawPoints(const QPolygon &points);

   inline void drawLine(const QLineF &line);
   inline void drawLine(const QLine &line);
   inline void drawLine(int x1, int y1, int x2, int y2);
   inline void drawLine(const QPoint &p1, const QPoint &p2);
   inline void drawLine(const QPointF &p1, const QPointF &p2);

   void drawLines(const QLineF *lines, int lineCount);
   inline void drawLines(const QVector<QLineF> &lines);
   void drawLines(const QPointF *pointPairs, int lineCount);
   inline void drawLines(const QVector<QPointF> &pointPairs);
   void drawLines(const QLine *lines, int lineCount);
   inline void drawLines(const QVector<QLine> &lines);
   void drawLines(const QPoint *pointPairs, int lineCount);
   inline void drawLines(const QVector<QPoint> &pointPairs);

   inline void drawRect(const QRectF &rect);
   inline void drawRect(int x1, int y1, int w, int h);
   inline void drawRect(const QRect &rect);

   void drawRects(const QRectF *rects, int rectCount);
   inline void drawRects(const QVector<QRectF> &rectangles);
   void drawRects(const QRect *rects, int rectCount);
   inline void drawRects(const QVector<QRect> &rectangles);

   void drawEllipse(const QRectF &r);
   void drawEllipse(const QRect &r);
   inline void drawEllipse(int x, int y, int w, int h);

   inline void drawEllipse(const QPointF &center, qreal rx, qreal ry);
   inline void drawEllipse(const QPoint &center, int rx, int ry);

   void drawPolyline(const QPointF *points, int pointCount);
   inline void drawPolyline(const QPolygonF &polyline);
   void drawPolyline(const QPoint *points, int pointCount);
   inline void drawPolyline(const QPolygon &polygon);

   void drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
   inline void drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule = Qt::OddEvenFill);
   void drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill);
   inline void drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule = Qt::OddEvenFill);

   void drawConvexPolygon(const QPointF *points, int pointCount);
   inline void drawConvexPolygon(const QPolygonF &polygon);
   void drawConvexPolygon(const QPoint *points, int pointCount);
   inline void drawConvexPolygon(const QPolygon &polygon);

   void drawArc(const QRectF &rect, int a, int alen);
   inline void drawArc(const QRect &, int a, int alen);
   inline void drawArc(int x, int y, int w, int h, int a, int alen);

   void drawPie(const QRectF &rect, int a, int alen);
   inline void drawPie(int x, int y, int w, int h, int a, int alen);
   inline void drawPie(const QRect &, int a, int alen);

   void drawChord(const QRectF &rect, int a, int alen);
   inline void drawChord(int x, int y, int w, int h, int a, int alen);
   inline void drawChord(const QRect &, int a, int alen);

   void drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius,
                        Qt::SizeMode mode = Qt::AbsoluteSize);

   inline void drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius,
                               Qt::SizeMode mode = Qt::AbsoluteSize);

   inline void drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius,
                               Qt::SizeMode mode = Qt::AbsoluteSize);

   void drawRoundRect(const QRectF &r, int xround = 25, int yround = 25);
   inline void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
   inline void drawRoundRect(const QRect &r, int xround = 25, int yround = 25);

   void drawTiledPixmap(const QRectF &rect, const QPixmap &pm, const QPointF &offset = QPointF());
   inline void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &, int sx = 0, int sy = 0);
   inline void drawTiledPixmap(const QRect &, const QPixmap &, const QPoint & = QPoint());

#ifndef QT_NO_PICTURE
   void drawPicture(const QPointF &p, const QPicture &picture);
   inline void drawPicture(int x, int y, const QPicture &picture);
   inline void drawPicture(const QPoint &p, const QPicture &picture);
#endif

   void drawPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect);
   inline void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect);
   inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy, int sw, int sh);
   inline void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh);
   inline void drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr);
   inline void drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr);
   void drawPixmap(const QPointF &p, const QPixmap &pm);
   inline void drawPixmap(const QPoint &p, const QPixmap &pm);
   inline void drawPixmap(int x, int y, const QPixmap &pm);
   inline void drawPixmap(const QRect &r, const QPixmap &pm);
   inline void drawPixmap(int x, int y, int w, int h, const QPixmap &pm);

   void drawPixmapFragments(const PixmapFragment *fragments, int fragmentCount,
                            const QPixmap &pixmap, PixmapFragmentHints hints = 0);
   void drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects, int fragmentCount,
                            const QPixmap &pixmap, PixmapFragmentHints hints = 0);

   void drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                  Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                         Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                         Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                         Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QRectF &r, const QImage &image);
   inline void drawImage(const QRect &r, const QImage &image);
   void drawImage(const QPointF &p, const QImage &image);
   inline void drawImage(const QPoint &p, const QImage &image);
   inline void drawImage(int x, int y, const QImage &image, int sx = 0, int sy = 0,
                         int sw = -1, int sh = -1, Qt::ImageConversionFlags flags = Qt::AutoColor);

   void setLayoutDirection(Qt::LayoutDirection direction);
   Qt::LayoutDirection layoutDirection() const;

#if !defined(QT_NO_RAWFONT)
   void drawGlyphRun(const QPointF &position, const QGlyphRun &glyphRun);
#endif

   void drawStaticText(const QPointF &topLeftPosition, const QStaticText &staticText);
   inline void drawStaticText(const QPoint &topLeftPosition, const QStaticText &staticText);
   inline void drawStaticText(int left, int top, const QStaticText &staticText);

   void drawText(const QPointF &p, const QString &s);
   inline void drawText(const QPoint &p, const QString &s);
   inline void drawText(int x, int y, const QString &s);

   void drawText(const QPointF &p, const QString &str, int tf, int justificationPadding);

   void drawText(const QRectF &r, int flags, const QString &text, QRectF *br = 0);
   void drawText(const QRect &r, int flags, const QString &text, QRect *br = 0);
   inline void drawText(int x, int y, int w, int h, int flags, const QString &text, QRect *br = 0);

   void drawText(const QRectF &r, const QString &text, const QTextOption &o = QTextOption());

   QRectF boundingRect(const QRectF &rect, int flags, const QString &text);
   QRect boundingRect(const QRect &rect, int flags, const QString &text);
   inline QRect boundingRect(int x, int y, int w, int h, int flags, const QString &text);

   QRectF boundingRect(const QRectF &rect, const QString &text, const QTextOption &o = QTextOption());

   void drawTextItem(const QPointF &p, const QTextItem &ti);
   inline void drawTextItem(int x, int y, const QTextItem &ti);
   inline void drawTextItem(const QPoint &p, const QTextItem &ti);

   void fillRect(const QRectF &, const QBrush &);
   inline void fillRect(int x, int y, int w, int h, const QBrush &);
   void fillRect(const QRect &, const QBrush &);

   void fillRect(const QRectF &, const QColor &color);
   inline void fillRect(int x, int y, int w, int h, const QColor &color);
   void fillRect(const QRect &, const QColor &color);

   inline void fillRect(int x, int y, int w, int h, Qt::GlobalColor c);
   inline void fillRect(const QRect &r, Qt::GlobalColor c);
   inline void fillRect(const QRectF &r, Qt::GlobalColor c);

   inline void fillRect(int x, int y, int w, int h, Qt::BrushStyle style);
   inline void fillRect(const QRect &r, Qt::BrushStyle style);
   inline void fillRect(const QRectF &r, Qt::BrushStyle style);

   void eraseRect(const QRectF &);
   inline void eraseRect(int x, int y, int w, int h);
   inline void eraseRect(const QRect &);

   void setRenderHint(RenderHint hint, bool on = true);
   void setRenderHints(RenderHints hints, bool on = true);
   RenderHints renderHints() const;
   inline bool testRenderHint(RenderHint hint) const {
      return renderHints() & hint;
   }

   QPaintEngine *paintEngine() const;

   static void setRedirected(const QPaintDevice *device, QPaintDevice *replacement,
                             const QPoint &offset = QPoint());
   static QPaintDevice *redirected(const QPaintDevice *device, QPoint *offset = 0);
   static void restoreRedirected(const QPaintDevice *device);

   void beginNativePainting();
   void endNativePainting();

 private:
   Q_DISABLE_COPY(QPainter)

   QScopedPointer<QPainterPrivate> d_ptr;

   friend class QFontEngine;
   friend class QFontEngineBox;
   friend class QFontEngineFT;
   friend class QFontEngineMac;
   friend class QFontEngineWin;
   friend class QFontEngineXLFD;
   friend class QWSManager;
   friend class QPaintEngine;
   friend class QPaintEngineExPrivate;
   friend class QOpenGLPaintEngine;
   friend class QVGPaintEngine;
   friend class QX11PaintEngine;
   friend class QX11PaintEnginePrivate;
   friend class QWin32PaintEngine;
   friend class QWin32PaintEnginePrivate;
   friend class QRasterPaintEngine;
   friend class QAlphaPaintEngine;
   friend class QPreviewPaintEngine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPainter::RenderHints)

//
// functions
//
inline void QPainter::drawLine(const QLineF &l)
{
   drawLines(&l, 1);
}

inline void QPainter::drawLine(const QLine &line)
{
   drawLines(&line, 1);
}

inline void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
   QLine l(x1, y1, x2, y2);
   drawLines(&l, 1);
}

inline void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
{
   QLine l(p1, p2);
   drawLines(&l, 1);
}

inline void QPainter::drawLine(const QPointF &p1, const QPointF &p2)
{
   drawLine(QLineF(p1, p2));
}

inline void QPainter::drawLines(const QVector<QLineF> &lines)
{
   drawLines(lines.constData(), lines.size());
}

inline void QPainter::drawLines(const QVector<QLine> &lines)
{
   drawLines(lines.constData(), lines.size());
}

inline void QPainter::drawLines(const QVector<QPointF> &pointPairs)
{
   drawLines(pointPairs.constData(), pointPairs.size() / 2);
}

inline void QPainter::drawLines(const QVector<QPoint> &pointPairs)
{
   drawLines(pointPairs.constData(), pointPairs.size() / 2);
}

inline void QPainter::drawPolyline(const QPolygonF &polyline)
{
   drawPolyline(polyline.constData(), polyline.size());
}

inline void QPainter::drawPolyline(const QPolygon &polyline)
{
   drawPolyline(polyline.constData(), polyline.size());
}

inline void QPainter::drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule)
{
   drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void QPainter::drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule)
{
   drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void QPainter::drawConvexPolygon(const QPolygonF &poly)
{
   drawConvexPolygon(poly.constData(), poly.size());
}

inline void QPainter::drawConvexPolygon(const QPolygon &poly)
{
   drawConvexPolygon(poly.constData(), poly.size());
}

inline void QPainter::drawRect(const QRectF &rect)
{
   drawRects(&rect, 1);
}

inline void QPainter::drawRect(int x, int y, int w, int h)
{
   QRect r(x, y, w, h);
   drawRects(&r, 1);
}

inline void QPainter::drawRect(const QRect &r)
{
   drawRects(&r, 1);
}

inline void QPainter::drawRects(const QVector<QRectF> &rects)
{
   drawRects(rects.constData(), rects.size());
}

inline void QPainter::drawRects(const QVector<QRect> &rects)
{
   drawRects(rects.constData(), rects.size());
}

inline void QPainter::drawPoint(const QPointF &p)
{
   drawPoints(&p, 1);
}

inline void QPainter::drawPoint(int x, int y)
{
   QPoint p(x, y);
   drawPoints(&p, 1);
}

inline void QPainter::drawPoint(const QPoint &p)
{
   drawPoints(&p, 1);
}

inline void QPainter::drawPoints(const QPolygonF &points)
{
   drawPoints(points.constData(), points.size());
}

inline void QPainter::drawPoints(const QPolygon &points)
{
   drawPoints(points.constData(), points.size());
}

inline void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
   drawRoundRect(QRectF(x, y, w, h), xRnd, yRnd);
}

inline void QPainter::drawRoundRect(const QRect &rect, int xRnd, int yRnd)
{
   drawRoundRect(QRectF(rect), xRnd, yRnd);
}

inline void QPainter::drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius,
                                      Qt::SizeMode mode)
{
   drawRoundedRect(QRectF(x, y, w, h), xRadius, yRadius, mode);
}

inline void QPainter::drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius,
                                      Qt::SizeMode mode)
{
   drawRoundedRect(QRectF(rect), xRadius, yRadius, mode);
}

inline void QPainter::drawEllipse(int x, int y, int w, int h)
{
   drawEllipse(QRect(x, y, w, h));
}

inline void QPainter::drawEllipse(const QPointF &center, qreal rx, qreal ry)
{
   drawEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void QPainter::drawEllipse(const QPoint &center, int rx, int ry)
{
   drawEllipse(QRect(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void QPainter::drawArc(const QRect &r, int a, int alen)
{
   drawArc(QRectF(r), a, alen);
}

inline void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
   drawArc(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::drawPie(const QRect &rect, int a, int alen)
{
   drawPie(QRectF(rect), a, alen);
}

inline void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
   drawPie(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::drawChord(const QRect &rect, int a, int alen)
{
   drawChord(QRectF(rect), a, alen);
}

inline void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
   drawChord(QRectF(x, y, w, h), a, alen);
}

inline void QPainter::setClipRect(int x, int y, int w, int h, Qt::ClipOperation op)
{
   setClipRect(QRect(x, y, w, h), op);
}

inline void QPainter::eraseRect(const QRect &rect)
{
   eraseRect(QRectF(rect));
}

inline void QPainter::eraseRect(int x, int y, int w, int h)
{
   eraseRect(QRectF(x, y, w, h));
}

inline void QPainter::fillRect(int x, int y, int w, int h, const QBrush &b)
{
   fillRect(QRect(x, y, w, h), b);
}

inline void QPainter::fillRect(int x, int y, int w, int h, const QColor &b)
{
   fillRect(QRect(x, y, w, h), b);
}

inline void QPainter::fillRect(int x, int y, int w, int h, Qt::GlobalColor c)
{
   fillRect(QRect(x, y, w, h), QColor(c));
}

inline void QPainter::fillRect(const QRect &r, Qt::GlobalColor c)
{
   fillRect(r, QColor(c));
}

inline void QPainter::fillRect(const QRectF &r, Qt::GlobalColor c)
{
   fillRect(r, QColor(c));
}

inline void QPainter::fillRect(int x, int y, int w, int h, Qt::BrushStyle style)
{
   fillRect(QRectF(x, y, w, h), QBrush(style));
}

inline void QPainter::fillRect(const QRect &r, Qt::BrushStyle style)
{
   fillRect(QRectF(r), QBrush(style));
}

inline void QPainter::fillRect(const QRectF &r, Qt::BrushStyle style)
{
   fillRect(r, QBrush(style));
}


inline void QPainter::setBrushOrigin(int x, int y)
{
   setBrushOrigin(QPoint(x, y));
}

inline void QPainter::setBrushOrigin(const QPoint &p)
{
   setBrushOrigin(QPointF(p));
}

inline void QPainter::drawTiledPixmap(const QRect &rect, const QPixmap &pm, const QPoint &offset)
{
   drawTiledPixmap(QRectF(rect), pm, QPointF(offset));
}

inline void QPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy)
{
   drawTiledPixmap(QRectF(x, y, w, h), pm, QPointF(sx, sy));
}

inline void QPainter::drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect)
{
   drawPixmap(QRectF(targetRect), pixmap, QRectF(sourceRect));
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm)
{
   drawPixmap(QPointF(p), pm);
}

inline void QPainter::drawPixmap(const QRect &r, const QPixmap &pm)
{
   drawPixmap(QRectF(r), pm, QRectF());
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm)
{
   drawPixmap(QPointF(x, y), pm);
}

inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm)
{
   drawPixmap(QRectF(x, y, w, h), pm, QRectF());
}

inline void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh)
{
   drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh));
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pm,
                                 int sx, int sy, int sw, int sh)
{
   drawPixmap(QRectF(x, y, -1, -1), pm, QRectF(sx, sy, sw, sh));
}

inline void QPainter::drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr)
{
   drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, sr);
}

inline void QPainter::drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr)
{
   drawPixmap(QRectF(p.x(), p.y(), -1, -1), pm, sr);
}

inline void QPainter::drawTextItem(int x, int y, const QTextItem &ti)
{
   drawTextItem(QPointF(x, y), ti);
}

inline void QPainter::drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                                Qt::ImageConversionFlags flags)
{
   drawImage(QRectF(targetRect), image, QRectF(sourceRect), flags);
}

inline void QPainter::drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                                Qt::ImageConversionFlags flags)
{
   drawImage(QRectF(p.x(), p.y(), -1, -1), image, sr, flags);
}

inline void QPainter::drawImage(const QPoint &p, const QImage &image, const QRect &sr,
                                Qt::ImageConversionFlags flags)
{
   drawImage(QRect(p.x(), p.y(), -1, -1), image, sr, flags);
}

inline void QPainter::drawImage(const QRectF &r, const QImage &image)
{
   drawImage(r, image, QRect(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QRect &r, const QImage &image)
{
   drawImage(r, image, QRectF(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QPoint &p, const QImage &image)
{
   drawImage(QPointF(p), image);
}

inline void QPainter::drawImage(int x, int y, const QImage &image, int sx, int sy, int sw, int sh,
                                Qt::ImageConversionFlags flags)
{
   if (sx == 0 && sy == 0 && sw == -1 && sh == -1 && flags == Qt::AutoColor) {
      drawImage(QPointF(x, y), image);
   } else {
      drawImage(QRectF(x, y, -1, -1), image, QRectF(sx, sy, sw, sh), flags);
   }
}

inline void QPainter::drawStaticText(const QPoint &p, const QStaticText &staticText)
{
   drawStaticText(QPointF(p), staticText);
}

inline void QPainter::drawStaticText(int x, int y, const QStaticText &staticText)
{
   drawStaticText(QPointF(x, y), staticText);
}

inline void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti)
{
   drawTextItem(QPointF(p), ti);
}

inline void QPainter::drawText(const QPoint &p, const QString &s)
{
   drawText(QPointF(p), s);
}

inline void QPainter::drawText(int x, int y, int w, int h, int flags, const QString &str, QRect *br)
{
   drawText(QRect(x, y, w, h), flags, str, br);
}

inline void QPainter::drawText(int x, int y, const QString &s)
{
   drawText(QPointF(x, y), s);
}

inline QRect QPainter::boundingRect(int x, int y, int w, int h, int flags, const QString &text)
{
   return boundingRect(QRect(x, y, w, h), flags, text);
}

inline void QPainter::translate(qreal dx, qreal dy)
{
   translate(QPointF(dx, dy));
}

inline void QPainter::translate(const QPoint &offset)
{
   translate(offset.x(), offset.y());
}

inline void QPainter::setViewport(int x, int y, int w, int h)
{
   setViewport(QRect(x, y, w, h));
}

inline void QPainter::setWindow(int x, int y, int w, int h)
{
   setWindow(QRect(x, y, w, h));
}

#ifndef QT_NO_PICTURE
inline void QPainter::drawPicture(int x, int y, const QPicture &p)
{
   drawPicture(QPoint(x, y), p);
}

inline void QPainter::drawPicture(const QPoint &pt, const QPicture &p)
{
   drawPicture(QPointF(pt), p);
}
#endif

QT_END_NAMESPACE

#endif // QPAINTER_H
