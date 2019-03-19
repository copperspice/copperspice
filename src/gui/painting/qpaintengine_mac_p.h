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

#ifndef QPAINTENGINE_MAC_P_H
#define QPAINTENGINE_MAC_P_H

#include <qpaintengine.h>
#include <qt_mac_p.h>
#include <qpaintengine_p.h>
#include <qpolygonclipper_p.h>
#include <qfont_p.h>
#include <qhash.h>

typedef struct CGColorSpace *CGColorSpaceRef;
QT_BEGIN_NAMESPACE

class QCoreGraphicsPaintEnginePrivate;

class QCoreGraphicsPaintEngine : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QCoreGraphicsPaintEngine)

 public:
   QCoreGraphicsPaintEngine();
   ~QCoreGraphicsPaintEngine();

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   static CGColorSpaceRef macGenericColorSpace();
   static CGColorSpaceRef macDisplayColorSpace(const QWidget *widget = 0);

   void updateState(const QPaintEngineState &state) override;

   void updatePen(const QPen &pen);
   void updateBrush(const QBrush &brush, const QPointF &pt);
   void updateFont(const QFont &font);
   void updateOpacity(qreal opacity);
   void updateMatrix(const QTransform &matrix);
   void updateTransform(const QTransform &matrix);
   void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
   void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
   void updateCompositionMode(QPainter::CompositionMode mode);
   void updateRenderHints(QPainter::RenderHints hints);

   void drawLines(const QLineF *lines, int lineCount) override;
   void drawRects(const QRectF *rects, int rectCount) override;
   void drawPoints(const QPointF *p, int pointCount) override;
   void drawEllipse(const QRectF &r) override;
   void drawPath(const QPainterPath &path) override;

   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;

   void drawTextItem(const QPointF &pos, const QTextItem &item) override;
   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                  Qt::ImageConversionFlags flags = Qt::AutoColor) override;

   Type type() const override {
      return QPaintEngine::CoreGraphics;
   }

   CGContextRef handle() const;

   static void initialize();
   static void cleanup();
   static void clearColorSpace(QWidget *w);

   QPainter::RenderHints supportedRenderHints() const;
 
   using QPaintEngine::drawLines;
   using QPaintEngine::drawRects;
   using QPaintEngine::drawPoints;
   using QPaintEngine::drawEllipse;
   using QPaintEngine::drawPolygon;

   bool supportsTransformations(qreal, const QTransform &) const {
      return true;
   }

 protected:
   friend class QMacPrintEngine;
   friend class QMacPrintEnginePrivate;
   friend void qt_mac_display_change_callbk(CGDirectDisplayID, CGDisplayChangeSummaryFlags, void *);
   friend void qt_color_profile_changed(CFNotificationCenterRef center, void *,
                                        CFStringRef , const void *, CFDictionaryRef);
   QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr);

 private:
   static bool m_postRoutineRegistered;
   static CGColorSpaceRef m_genericColorSpace;
   static QHash<QWidget *, CGColorSpaceRef> m_displayColorSpaceHash; // window -> color space
   static void cleanUpMacColorSpaces();
   Q_DISABLE_COPY(QCoreGraphicsPaintEngine)
};

class QCoreGraphicsPaintEnginePrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QCoreGraphicsPaintEngine)

 public:
   QCoreGraphicsPaintEnginePrivate()
      : hd(0), shading(0), stackCount(0), complexXForm(false), disabledSmoothFonts(false) {
   }

   struct {
      QPen pen;
      QBrush brush;
      uint clipEnabled : 1;
      QRegion clip;
      QTransform transform;
   } current;

   //state info (shared with QD)
   CGAffineTransform orig_xform;

   //cg structures
   CGContextRef hd;
   CGShadingRef shading;
   int stackCount;
   bool complexXForm;
   bool disabledSmoothFonts;
   enum { CosmeticNone, CosmeticTransformPath, CosmeticSetPenWidth } cosmeticPen;

   // pixel and cosmetic pen size in user coordinates.
   QPointF pixelSize;
   float cosmeticPenSize;

   //internal functions
   enum { CGStroke = 0x01, CGEOFill = 0x02, CGFill = 0x04 };
   void drawPath(uchar ops, CGMutablePathRef path = 0);
   void setClip(const QRegion *rgn = 0);
   void resetClip();
   void setFillBrush(const QPointF &origin = QPoint());
   void setStrokePen(const QPen &pen);
   inline void saveGraphicsState();
   inline void restoreGraphicsState();
   float penOffset();
   QPointF devicePixelSize(CGContextRef context);
   float adjustPenWidth(float penWidth);

   inline void setTransform(const QTransform *matrix = 0) {
      CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
      CGAffineTransform xform = orig_xform;
      if (matrix) {
         extern CGAffineTransform qt_mac_convert_transform_to_cg(const QTransform &);
         xform = CGAffineTransformConcat(qt_mac_convert_transform_to_cg(*matrix), xform);
      }
      CGContextConcatCTM(hd, xform);
      CGContextSetTextMatrix(hd, xform);
   }
};

inline void QCoreGraphicsPaintEnginePrivate::saveGraphicsState()
{
   ++stackCount;
   CGContextSaveGState(hd);
}

inline void QCoreGraphicsPaintEnginePrivate::restoreGraphicsState()
{
   --stackCount;
   Q_ASSERT(stackCount >= 0);
   CGContextRestoreGState(hd);
}

class QMacQuartzPaintDevice : public QPaintDevice
{
 public:
   QMacQuartzPaintDevice(CGContextRef cg, int width, int height, int bytesPerLine)
      : mCG(cg), mWidth(width), mHeight(height), mBytesPerLine(bytesPerLine) {
   }
   int devType() const {
      return QInternal::MacQuartz;
   }
   CGContextRef cgContext() const {
      return mCG;
   }
   int metric(PaintDeviceMetric metric) const {
      switch (metric) {
         case PdmWidth:
            return mWidth;
         case PdmHeight:
            return mHeight;
         case PdmWidthMM:
            return (qt_defaultDpiX() * mWidth) / 2.54;
         case PdmHeightMM:
            return (qt_defaultDpiY() * mHeight) / 2.54;
         case PdmNumColors:
            return 0;
         case PdmDepth:
            return 32;
         case PdmDpiX:
         case PdmPhysicalDpiX:
            return qt_defaultDpiX();
         case PdmDpiY:
         case PdmPhysicalDpiY:
            return qt_defaultDpiY();
      }
      return 0;
   }
   QPaintEngine *paintEngine() const {
      qWarning("This function should never be called.");
      return 0;
   }

 private:
   CGContextRef mCG;
   int mWidth;
   int mHeight;
   int mBytesPerLine;
};

QT_END_NAMESPACE

#endif // QPAINTENGINE_MAC_P_H
