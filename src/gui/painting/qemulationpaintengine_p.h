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

#ifndef QEMULATIONPAINTENGINE_P_H
#define QEMULATIONPAINTENGINE_P_H

#include <qpaintengineex_p.h>

class QEmulationPaintEngine : public QPaintEngineEx
{
 public:
   QEmulationPaintEngine(QPaintEngineEx *engine);

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   Type type() const override;
   QPainterState *createState(QPainterState *orig) const override;

   void fill(const QVectorPath &path, const QBrush &brush) override;
   void stroke(const QVectorPath &path, const QPen &pen) override;
   void clip(const QVectorPath &path, Qt::ClipOperation op) override;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;
   void drawStaticTextItem(QStaticTextItem *item) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;
   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags) override;

   void clipEnabledChanged() override;
   void penChanged() override;
   void brushChanged() override;
   void brushOriginChanged() override;
   void opacityChanged() override;
   void compositionModeChanged() override;
   void renderHintsChanged() override;
   void transformChanged() override;

   void setState(QPainterState *s) override;

   void beginNativePainting() override;
   void endNativePainting() override;

   uint flags() const  override {
      return QPaintEngineEx::IsEmulationEngine | QPaintEngineEx::DoNotEmulate;
   }

   QPainterState *state() {
      return (QPainterState *)QPaintEngine::state;
   }

   const QPainterState *state() const {
      return (const QPainterState *)QPaintEngine::state;
   }

   QPaintEngineEx *real_engine;

 private:
   void fillBGRect(const QRectF &r);
};

#endif
