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

#ifndef QPAINTER_H
#define QPAINTER_H

#include <qnamespace.h>
#include <qrect.h>
#include <qpoint.h>
#include <qscopedpointer.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtextoption.h>

#include <qpolygon.h>
#include <qpen.h>
#include <qbrush.h>
#include <qmatrix.h>
#include <qtransform.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>

class QBrush;
class QFontInfo;
class QFontMetrics;
class QPaintDevice;
class QPainterPath;
class QPainterPrivate;
class QPen;
class QPolygon;
class QTextItem;
class QTextEngine;
class QMatrix;
class QTransform;
class QStaticText;
class QGlyphRun;
class QPainterPrivateDeleter;

class Q_GUI_EXPORT QPainter
{
   GUI_CS_GADGET(QPainter)

   GUI_CS_ENUM(RenderHint)
   GUI_CS_FLAG(RenderHint, RenderHints)

 public:
   GUI_CS_REGISTER_ENUM(
      enum RenderHint {
         Antialiasing            = 0x01,
         TextAntialiasing        = 0x02,
         SmoothPixmapTransform   = 0x04,
         HighQualityAntialiasing = 0x08,
         NonCosmeticDefaultPen   = 0x10
      };
   )
   using RenderHints = QFlags<RenderHint>;

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
      RasterOp_SourceAndNotDestination,
      RasterOp_NotSourceOrDestination,
      RasterOp_SourceOrNotDestination,
      RasterOp_ClearDestination,
      RasterOp_SetDestination,
      RasterOp_NotDestination
   };

   enum PixmapFragmentHint {
      OpaqueHint = 0x01
   };
   using PixmapFragmentHints = QFlags<PixmapFragmentHint>;

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

   QPainter();
   explicit QPainter(QPaintDevice *device);

   QPainter(const QPainter &) = delete;
   QPainter &operator=(const QPainter &) = delete;

   ~QPainter();

   QPaintDevice *device() const;

   bool begin(QPaintDevice *device);
   bool end();
   bool isActive() const;

   void initFrom(const QPaintDevice *device);

   void setCompositionMode(CompositionMode mode);
   CompositionMode compositionMode() const;

   const QFont &font() const;
   void setFont(const QFont &font);

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
   inline void setBrushOrigin(const QPoint &point);
   void setBrushOrigin(const QPointF &point);

   void setBackground(const QBrush &brush);
   const QBrush &background() const;

   qreal opacity() const;
   void setOpacity(qreal opacity);

   // Clip functions
   QRegion clipRegion() const;
   QPainterPath clipPath() const;

   void setClipRect(const QRectF &rect, Qt::ClipOperation operation = Qt::ReplaceClip);
   void setClipRect(const QRect &rect, Qt::ClipOperation operation = Qt::ReplaceClip);
   inline void setClipRect(int x, int y, int width, int height, Qt::ClipOperation operation = Qt::ReplaceClip);

   void setClipRegion(const QRegion &region, Qt::ClipOperation operation = Qt::ReplaceClip);

   void setClipPath(const QPainterPath &path, Qt::ClipOperation operation = Qt::ReplaceClip);

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

   void setMatrixEnabled(bool enable);
   bool matrixEnabled() const;

   void setWorldMatrixEnabled(bool enable);
   bool worldMatrixEnabled() const;

   void scale(qreal sx, qreal sy);
   void shear(qreal sh, qreal sv);
   void rotate(qreal angle);

   void translate(const QPointF &offset);
   inline void translate(const QPoint &offset);
   inline void translate(qreal dx, qreal dy);

   QRect window() const;
   void setWindow(const QRect &window);
   inline void setWindow(int x, int y, int width, int height);

   QRect viewport() const;
   void setViewport(const QRect &viewport);
   inline void setViewport(int x, int y, int width, int height);

   void setViewTransformEnabled(bool enable);
   bool viewTransformEnabled() const;

   // drawing functions
   void strokePath(const QPainterPath &path, const QPen &pen);
   void fillPath(const QPainterPath &path, const QBrush &brush);
   void drawPath(const QPainterPath &path);

   inline void drawPoint(const QPointF &point);
   inline void drawPoint(const QPoint &point);
   inline void drawPoint(int x, int y);

   void drawPoints(const QPointF *points, int pointCount);
   inline void drawPoints(const QPolygonF &polygon);
   void drawPoints(const QPoint *points, int pointCount);
   inline void drawPoints(const QPolygon &polygon);

   inline void drawLine(const QLineF &line);
   inline void drawLine(const QLine &line);
   inline void drawLine(int x1, int y1, int x2, int y2);
   inline void drawLine(const QPoint &point1, const QPoint &point2);
   inline void drawLine(const QPointF &point1, const QPointF &point2);

   void drawLines(const QLineF *lines, int lineCount);
   inline void drawLines(const QVector<QLineF> &lines);

   void drawLines(const QPointF *points, int lineCount);
   inline void drawLines(const QVector<QPointF> &points);

   void drawLines(const QLine *lines, int lineCount);
   inline void drawLines(const QVector<QLine> &lines);

   void drawLines(const QPoint *points, int lineCount);
   inline void drawLines(const QVector<QPoint> &points);

   inline void drawRect(const QRectF &rect);
   inline void drawRect(int x, int y, int width, int height);
   inline void drawRect(const QRect &rect);

   void drawRects(const QRectF *rects, int rectCount);
   inline void drawRects(const QVector<QRectF> &rectangles);

   void drawRects(const QRect *rects, int rectCount);
   inline void drawRects(const QVector<QRect> &rectangles);

   void drawEllipse(const QRectF &rect);
   void drawEllipse(const QRect &rect);
   inline void drawEllipse(int x, int y, int width, int height);

   inline void drawEllipse(const QPointF &center, qreal rx, qreal ry);
   inline void drawEllipse(const QPoint &center, int rx, int ry);

   void drawPolyline(const QPointF *points, int pointCount);
   inline void drawPolyline(const QPolygonF &polygon);
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

   void drawArc(const QRectF &rect, int startAngle, int arcLength);
   inline void drawArc(const QRect &rect, int startAngle, int arcLength);
   inline void drawArc(int x, int y, int width, int height, int startAngle, int arcLength);

   void drawPie(const QRectF &rect, int startAngle, int arcLength);
   inline void drawPie(int x, int y, int width, int height, int startAngle, int arcLength);
   inline void drawPie(const QRect &rect, int startAngle, int arcLength);

   void drawChord(const QRectF &rect, int startAngle, int arcLength);
   inline void drawChord(int x, int y, int width, int height, int startAngle, int arcLength);
   inline void drawChord(const QRect &rect, int startAngle, int arcLength);

   void drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius,
      Qt::SizeMode mode = Qt::AbsoluteSize);

   inline void drawRoundedRect(int x, int y, int width, int height, qreal xRadius, qreal yRadius,
      Qt::SizeMode mode = Qt::AbsoluteSize);

   inline void drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius,
      Qt::SizeMode mode = Qt::AbsoluteSize);

   void drawRoundRect(const QRectF &rect, int xRound = 25, int yRound = 25);
   inline void drawRoundRect(int x, int y, int width, int height, int xRound = 25, int yRound = 25);
   inline void drawRoundRect(const QRect &rect, int xRound = 25, int yRound = 25);

   void drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &point = QPointF());
   inline void drawTiledPixmap(int x, int y, int width, int height, const QPixmap &pixmap, int sx = 0, int sy = 0);
   inline void drawTiledPixmap(const QRect &rect, const QPixmap &pixmap, const QPoint &point = QPoint());

#ifndef QT_NO_PICTURE
   void drawPicture(const QPointF &point, const QPicture &picture);
   inline void drawPicture(int x, int y, const QPicture &picture);
   inline void drawPicture(const QPoint &point, const QPicture &picture);
#endif

   void drawPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect);
   inline void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect);
   inline void drawPixmap(int x, int y, int width, int height, const QPixmap &pixmap, int sx, int sy, int sw, int sh);
   inline void drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh);
   inline void drawPixmap(const QPointF &point, const QPixmap &pixmap, const QRectF &sourceRect);
   inline void drawPixmap(const QPoint &point, const QPixmap &pixmap, const QRect &sourceRect);
   void drawPixmap(const QPointF &point, const QPixmap &pixmap);
   inline void drawPixmap(const QPoint &point, const QPixmap &pixmap);
   inline void drawPixmap(int x, int y, const QPixmap &pixmap);
   inline void drawPixmap(const QRect &rect, const QPixmap &pixmap);
   inline void drawPixmap(int x, int y, int width, int height, const QPixmap &pixmap);

   void drawPixmapFragments(const PixmapFragment *fragments, int fragmentCount,
      const QPixmap &pixmap, PixmapFragmentHints hints = PixmapFragmentHints());

   void drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
      Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
      Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QPointF &point, const QImage &image, const QRectF &sourceRect,
      Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QPoint &point, const QImage &image, const QRect &sourceRect,
      Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline void drawImage(const QRectF &rect, const QImage &image);
   inline void drawImage(const QRect &rect, const QImage &image);
   void drawImage(const QPointF &point, const QImage &image);
   inline void drawImage(const QPoint &point, const QImage &image);
   inline void drawImage(int x, int y, const QImage &image, int sx = 0, int sy = 0,
      int sw = -1, int sh = -1, Qt::ImageConversionFlags flags = Qt::AutoColor);

   void setLayoutDirection(Qt::LayoutDirection direction);
   Qt::LayoutDirection layoutDirection() const;

   void drawGlyphRun(const QPointF &point, const QGlyphRun &glyphs);

   void drawStaticText(const QPointF &topLeftPosition, const QStaticText &staticText);
   inline void drawStaticText(const QPoint &topLeftPosition, const QStaticText &staticText);
   inline void drawStaticText(int left, int top, const QStaticText &staticText);

   void drawText(const QPointF &point, const QString &text);
   inline void drawText(const QPoint &point, const QString &text);
   inline void drawText(int x, int y, const QString &text);

   void drawText(const QPointF &point, const QString &text, int tf, int justificationPadding);

   void drawText(const QRectF &rect, int flags, const QString &text, QRectF *boundingRect = nullptr);
   void drawText(const QRect &rect, int flags, const QString &text, QRect *boundingRect  = nullptr);
   inline void drawText(int x, int y, int width, int height, int flags, const QString &text, QRect *boundingRect  = nullptr);

   void drawText(const QRectF &rect, const QString &text, const QTextOption &option = QTextOption());

   QRectF boundingRect(const QRectF &rect, int flags, const QString &text);
   QRect boundingRect(const QRect &rect, int flags, const QString &text);
   inline QRect boundingRect(int x, int y, int width, int height, int flags, const QString &text);

   QRectF boundingRect(const QRectF &rect, const QString &text,
         const QTextOption &option = QTextOption());

   void drawTextItem(const QPointF &topLeftPosition, const QTextItem &staticText);
   inline void drawTextItem(int x, int y, const QTextItem &staticText);
   inline void drawTextItem(const QPoint &point, const QTextItem &staticText);

   void fillRect(const QRectF &rect, const QBrush &brush);
   inline void fillRect(int x, int y, int width, int height, const QBrush &brush);
   void fillRect(const QRect &rect, const QBrush &brush);

   void fillRect(const QRectF &rect, const QColor &color);
   inline void fillRect(int x, int y, int width, int height, const QColor &color);
   void fillRect(const QRect &rect, const QColor &color);

   inline void fillRect(int x, int y, int width, int height, Qt::GlobalColor color);
   inline void fillRect(const QRect &rect, Qt::GlobalColor color);
   inline void fillRect(const QRectF &rect, Qt::GlobalColor color);

   inline void fillRect(int x, int y, int width, int height, Qt::BrushStyle style);
   inline void fillRect(const QRect &rect, Qt::BrushStyle style);
   inline void fillRect(const QRectF &rect, Qt::BrushStyle style);

   void eraseRect(const QRectF &rect);
   inline void eraseRect(int x, int y, int width, int height);
   inline void eraseRect(const QRect &rect);

   void setRenderHint(RenderHint hint, bool on = true);
   void setRenderHints(RenderHints hints, bool on = true);
   RenderHints renderHints() const;

   bool testRenderHint(RenderHint hint) const {
      return renderHints() & hint;
   }

   QPaintEngine *paintEngine() const;

   static void setRedirected(const QPaintDevice *device, QPaintDevice *replacement,
      const QPoint &point = QPoint());

   static QPaintDevice *redirected(const QPaintDevice *device, QPoint *point = nullptr);
   static void restoreRedirected(const QPaintDevice *device);

   void beginNativePainting();
   void endNativePainting();

 private:
   Q_DECLARE_PRIVATE(QPainter)
   QScopedPointer<QPainterPrivate> d_ptr;

   friend class QWidget;
   friend class QFontEngine;
   friend class QFontEngineBox;
   friend class QFontEngineFT;
   friend class QFontEngineMac;
   friend class QFontEngineWin;

   friend class QPaintEngine;
   friend class QPaintEngineExPrivate;
   friend class QOpenGLPaintEngine;

   friend class QWin32PaintEngine;
   friend class QWin32PaintEnginePrivate;
   friend class QRasterPaintEngine;
   friend class QAlphaPaintEngine;
   friend class QPreviewPaintEngine;
   friend class QTextEngine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPainter::RenderHints)

inline void QPainter::drawLine(const QLineF &line)
{
   drawLines(&line, 1);
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

inline void QPainter::drawLine(const QPoint &point1, const QPoint &point2)
{
   QLine l(point1, point2);
   drawLines(&l, 1);
}

inline void QPainter::drawLine(const QPointF &point1, const QPointF &point2)
{
   drawLine(QLineF(point1, point2));
}

inline void QPainter::drawLines(const QVector<QLineF> &lines)
{
   drawLines(lines.constData(), lines.size());
}

inline void QPainter::drawLines(const QVector<QLine> &lines)
{
   drawLines(lines.constData(), lines.size());
}

inline void QPainter::drawLines(const QVector<QPointF> &points)
{
   drawLines(points.constData(), points.size() / 2);
}

inline void QPainter::drawLines(const QVector<QPoint> &points)
{
   drawLines(points.constData(), points.size() / 2);
}

inline void QPainter::drawPolyline(const QPolygonF &polygon)
{
   drawPolyline(polygon.constData(), polygon.size());
}

inline void QPainter::drawPolyline(const QPolygon &polygon)
{
   drawPolyline(polygon.constData(), polygon.size());
}

inline void QPainter::drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule)
{
   drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void QPainter::drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule)
{
   drawPolygon(polygon.constData(), polygon.size(), fillRule);
}

inline void QPainter::drawConvexPolygon(const QPolygonF &polygon)
{
   drawConvexPolygon(polygon.constData(), polygon.size());
}

inline void QPainter::drawConvexPolygon(const QPolygon &polygon)
{
   drawConvexPolygon(polygon.constData(), polygon.size());
}

inline void QPainter::drawRect(const QRectF &rect)
{
   drawRects(&rect, 1);
}

inline void QPainter::drawRect(int x, int y, int width, int height)
{
   QRect r(x, y, width, height);
   drawRects(&r, 1);
}

inline void QPainter::drawRect(const QRect &rect)
{
   drawRects(&rect, 1);
}

inline void QPainter::drawRects(const QVector<QRectF> &rectangles)
{
   drawRects(rectangles.constData(), rectangles.size());
}

inline void QPainter::drawRects(const QVector<QRect> &rectangles)
{
   drawRects(rectangles.constData(), rectangles.size());
}

inline void QPainter::drawPoint(const QPointF &point)
{
   drawPoints(&point, 1);
}

inline void QPainter::drawPoint(int x, int y)
{
   QPoint p(x, y);
   drawPoints(&p, 1);
}

inline void QPainter::drawPoint(const QPoint &point)
{
   drawPoints(&point, 1);
}

inline void QPainter::drawPoints(const QPolygonF &polygon)
{
   drawPoints(polygon.constData(), polygon.size());
}

inline void QPainter::drawPoints(const QPolygon &polygon)
{
   drawPoints(polygon.constData(), polygon.size());
}

inline void QPainter::drawRoundRect(int x, int y, int width, int height, int xRound, int yRound)
{
   drawRoundRect(QRectF(x, y, width, height), xRound, yRound);
}

inline void QPainter::drawRoundRect(const QRect &rect, int xRound, int yRound)
{
   drawRoundRect(QRectF(rect), xRound, yRound);
}

inline void QPainter::drawRoundedRect(int x, int y, int width, int height, qreal xRadius, qreal yRadius,
   Qt::SizeMode mode)
{
   drawRoundedRect(QRectF(x, y, width, height), xRadius, yRadius, mode);
}

inline void QPainter::drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius,
   Qt::SizeMode mode)
{
   drawRoundedRect(QRectF(rect), xRadius, yRadius, mode);
}

inline void QPainter::drawEllipse(int x, int y, int width, int height)
{
   drawEllipse(QRect(x, y, width, height));
}

inline void QPainter::drawEllipse(const QPointF &center, qreal rx, qreal ry)
{
   drawEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void QPainter::drawEllipse(const QPoint &center, int rx, int ry)
{
   drawEllipse(QRect(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
}

inline void QPainter::drawArc(const QRect &rect, int startAngle, int arcLength)
{
   drawArc(QRectF(rect), startAngle, arcLength);
}

inline void QPainter::drawArc(int x, int y, int width, int height, int startAngle, int arcLength)
{
   drawArc(QRectF(x, y, width, height), startAngle, arcLength);
}

inline void QPainter::drawPie(const QRect &rect, int startAngle, int arcLength)
{
   drawPie(QRectF(rect), startAngle, arcLength);
}

inline void QPainter::drawPie(int x, int y, int width, int height, int startAngle, int arcLength)
{
   drawPie(QRectF(x, y, width,height), startAngle, arcLength);
}

inline void QPainter::drawChord(const QRect &rect, int startAngle, int arcLength)
{
   drawChord(QRectF(rect), startAngle, arcLength);
}

inline void QPainter::drawChord(int x, int y, int width, int height, int startAngle, int arcLength)
{
   drawChord(QRectF(x, y, width, height), startAngle, arcLength);
}

inline void QPainter::setClipRect(int x, int y, int width, int height, Qt::ClipOperation operation)
{
   setClipRect(QRect(x, y, width, height), operation);
}

inline void QPainter::eraseRect(const QRect &rect)
{
   eraseRect(QRectF(rect));
}

inline void QPainter::eraseRect(int x, int y, int width, int height)
{
   eraseRect(QRectF(x, y, width, height));
}

inline void QPainter::fillRect(int x, int y, int width, int height, const QBrush &brush)
{
   fillRect(QRect(x, y, width, height), brush);
}

inline void QPainter::fillRect(int x, int y, int width, int height, const QColor &color)
{
   fillRect(QRect(x, y, width, height), color);
}

inline void QPainter::fillRect(int x, int y, int width, int height, Qt::GlobalColor color)
{
   fillRect(QRect(x, y, width, height), QColor(color));
}

inline void QPainter::fillRect(const QRect &rect, Qt::GlobalColor color)
{
   fillRect(rect, QColor(color));
}

inline void QPainter::fillRect(const QRectF &rect, Qt::GlobalColor color)
{
   fillRect(rect, QColor(color));
}

inline void QPainter::fillRect(int x, int y, int width, int height, Qt::BrushStyle style)
{
   fillRect(QRectF(x, y, width, height), QBrush(style));
}

inline void QPainter::fillRect(const QRect &rect, Qt::BrushStyle style)
{
   fillRect(QRectF(rect), QBrush(style));
}

inline void QPainter::fillRect(const QRectF &rect, Qt::BrushStyle style)
{
   fillRect(rect, QBrush(style));
}

inline void QPainter::setBrushOrigin(int x, int y)
{
   setBrushOrigin(QPoint(x, y));
}

inline void QPainter::setBrushOrigin(const QPoint &point)
{
   setBrushOrigin(QPointF(point));
}

inline void QPainter::drawTiledPixmap(const QRect &rect, const QPixmap &pixmap, const QPoint &point)
{
   drawTiledPixmap(QRectF(rect), pixmap, QPointF(point));
}

inline void QPainter::drawTiledPixmap(int x, int y, int width, int height, const QPixmap &pixmap, int sx, int sy)
{
   drawTiledPixmap(QRectF(x, y, width, height), pixmap, QPointF(sx, sy));
}

inline void QPainter::drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect)
{
   drawPixmap(QRectF(targetRect), pixmap, QRectF(sourceRect));
}

inline void QPainter::drawPixmap(const QPoint &point, const QPixmap &pixmap)
{
   drawPixmap(QPointF(point), pixmap);
}

inline void QPainter::drawPixmap(const QRect &rect, const QPixmap &pixmap)
{
   drawPixmap(QRectF(rect), pixmap, QRectF());
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap)
{
   drawPixmap(QPointF(x, y), pixmap);
}

inline void QPainter::drawPixmap(int x, int y, int width, int height, const QPixmap &pixmap)
{
   drawPixmap(QRectF(x, y, width, height), pixmap, QRectF());
}

inline void QPainter::drawPixmap(int x, int y, int width, int height, const QPixmap &pixmap,
   int sx, int sy, int sw, int sh)
{
   drawPixmap(QRectF(x, y, width, height), pixmap, QRectF(sx, sy, sw, sh));
}

inline void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap,
   int sx, int sy, int sw, int sh)
{
   drawPixmap(QRectF(x, y, -1, -1), pixmap, QRectF(sx, sy, sw, sh));
}

inline void QPainter::drawPixmap(const QPointF &point, const QPixmap &pixmap, const QRectF &sourceRect)
{
   drawPixmap(QRectF(point.x(), point.y(), -1, -1), pixmap, sourceRect);
}

inline void QPainter::drawPixmap(const QPoint &point, const QPixmap &pixmap, const QRect &sourceRect)
{
   drawPixmap(QRectF(point.x(), point.y(), -1, -1), pixmap, sourceRect);
}

inline void QPainter::drawTextItem(int x, int y, const QTextItem &staticText)
{
   drawTextItem(QPointF(x, y), staticText);
}

inline void QPainter::drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
   Qt::ImageConversionFlags flags)
{
   drawImage(QRectF(targetRect), image, QRectF(sourceRect), flags);
}

inline void QPainter::drawImage(const QPointF &point, const QImage &image, const QRectF &sourceRect,
   Qt::ImageConversionFlags flags)
{
   drawImage(QRectF(point.x(), point.y(), -1, -1), image, sourceRect, flags);
}

inline void QPainter::drawImage(const QPoint &point, const QImage &image, const QRect &sourceRect,
   Qt::ImageConversionFlags flags)
{
   drawImage(QRect(point.x(), point.y(), -1, -1), image, sourceRect, flags);
}

inline void QPainter::drawImage(const QRectF &rect, const QImage &image)
{
   drawImage(rect, image, QRect(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QRect &rect, const QImage &image)
{
   drawImage(rect, image, QRectF(0, 0, image.width(), image.height()));
}

inline void QPainter::drawImage(const QPoint &point, const QImage &image)
{
   drawImage(QPointF(point), image);
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

inline void QPainter::drawStaticText(const QPoint &topLeftPosition, const QStaticText &staticText)
{
   drawStaticText(QPointF(topLeftPosition), staticText);
}

inline void QPainter::drawStaticText(int left, int top, const QStaticText &staticText)
{
   drawStaticText(QPointF(left, top), staticText);
}

inline void QPainter::drawTextItem(const QPoint &topLeftPosition, const QTextItem &staticText)
{
   drawTextItem(QPointF(topLeftPosition), staticText);
}

inline void QPainter::drawText(const QPoint &point, const QString &text)
{
   drawText(QPointF(point), text);
}

inline void QPainter::drawText(int x, int y, int width, int height, int flags, const QString &text, QRect *boundingRect)
{
   drawText(QRect(x, y, width, height), flags, text, boundingRect);
}

inline void QPainter::drawText(int x, int y, const QString &text)
{
   drawText(QPointF(x, y), text);
}

inline QRect QPainter::boundingRect(int x, int y, int width, int height, int flags, const QString &text)
{
   return boundingRect(QRect(x, y, width, height), flags, text);
}

inline void QPainter::translate(qreal dx, qreal dy)
{
   translate(QPointF(dx, dy));
}

inline void QPainter::translate(const QPoint &offset)
{
   translate(offset.x(), offset.y());
}

inline void QPainter::setViewport(int x, int y, int width, int height)
{
   setViewport(QRect(x, y, width, height));
}

inline void QPainter::setWindow(int x, int y, int width, int height)
{
   setWindow(QRect(x, y, width, height));
}

#ifndef QT_NO_PICTURE
inline void QPainter::drawPicture(int x, int y, const QPicture &picture)
{
   drawPicture(QPoint(x, y), picture);
}

inline void QPainter::drawPicture(const QPoint &point, const QPicture &picture)
{
   drawPicture(QPointF(point), picture);
}
#endif


#endif
