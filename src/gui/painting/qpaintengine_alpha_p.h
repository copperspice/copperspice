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

#ifndef QPAINTENGINE_ALPHA_P_H
#define QPAINTENGINE_ALPHA_P_H

#ifndef QT_NO_PRINTER

#include <qpaintengine_p.h>

class QAlphaPaintEnginePrivate;

class QAlphaPaintEngine : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QAlphaPaintEngine)

 public:
   ~QAlphaPaintEngine();

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   void updateState(const QPaintEngineState &state) override;

   void drawPath(const QPainterPath &path) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;

   void drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &srcRect) override;
   void drawTextItem(const QPointF &point, const QTextItem &textItem) override;
   void drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &point) override;

 protected:
   QAlphaPaintEngine(QAlphaPaintEnginePrivate &data, PaintEngineFeatures devcaps = Qt::EmptyFlag);
   QRegion alphaClipping() const;
   bool continueCall() const;
   void flushAndInit(bool init = true);
   void cleanUp();
};

class QAlphaPaintEnginePrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QAlphaPaintEngine)

 public:
   QAlphaPaintEnginePrivate();
   ~QAlphaPaintEnginePrivate();

   int m_pass;
   QPicture *m_pic;
   QPaintEngine *m_picengine;
   QPainter *m_picpainter;

   QPaintEngine::PaintEngineFeatures m_savedcaps;
   QPaintDevice *m_pdev;

   QRegion m_alphargn;
   QRegion m_cliprgn;

   mutable QRegion m_cachedDirtyRgn;
   mutable int m_numberOfCachedRects;
   QVector<QRect> m_dirtyRects;

   bool m_hasalpha;
   bool m_alphaPen;
   bool m_alphaBrush;
   bool m_alphaOpacity;
   bool m_advancedPen;
   bool m_advancedBrush;
   bool m_complexTransform;
   bool m_emulateProjectiveTransforms;
   bool m_continueCall;

   QTransform m_transform;
   QPen m_pen;

   void addAlphaRect(const QRectF &rect);

   void addDirtyRect(const QRectF &rect) {
      m_dirtyRects.append(rect.toAlignedRect());
   }
   bool canSeeTroughBackground(bool somethingInRectHasAlpha, const QRectF &rect) const;
   QRectF addPenWidth(const QPainterPath &path);
   void drawAlphaImage(const QRectF &rect);
   QRect toRect(const QRectF &rect) const;
   bool fullyContained(const QRectF &rect) const;

   void resetState(QPainter *p);
};

#endif // QT_NO_PRINTER

#endif