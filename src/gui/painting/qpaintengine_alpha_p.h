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

#ifndef QPAINTENGINE_ALPHA_P_H
#define QPAINTENGINE_ALPHA_P_H

#ifndef QT_NO_PRINTER

#include <qpaintengine_p.h>

QT_BEGIN_NAMESPACE

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

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawImage(const QRectF &r, const QImage &image, const QRectF &sr);
   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;

 protected:
   QAlphaPaintEngine(QAlphaPaintEnginePrivate &data, PaintEngineFeatures devcaps = 0);
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
   QRectF addPenWidth(const QPainterPath &path);
   void drawAlphaImage(const QRectF &rect);
   QRect toRect(const QRectF &rect) const;
   bool fullyContained(const QRectF &rect) const;

   void resetState(QPainter *p);
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPAINTENGINE_ALPHA_P_H
