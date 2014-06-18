/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QEMULATIONPAINTENGINE_P_H
#define QEMULATIONPAINTENGINE_P_H

#include <qpaintengineex_p.h>

QT_BEGIN_NAMESPACE

class QEmulationPaintEngine : public QPaintEngineEx
{
 public:
   QEmulationPaintEngine(QPaintEngineEx *engine);

   virtual bool begin(QPaintDevice *pdev);
   virtual bool end();

   virtual Type type() const;
   virtual QPainterState *createState(QPainterState *orig) const;

   virtual void fill(const QVectorPath &path, const QBrush &brush);
   virtual void stroke(const QVectorPath &path, const QPen &pen);
   virtual void clip(const QVectorPath &path, Qt::ClipOperation op);

   virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
   virtual void drawTextItem(const QPointF &p, const QTextItem &textItem);
   virtual void drawStaticTextItem(QStaticTextItem *item);
   virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
   virtual void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags);

   virtual void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                                    QPainter::PixmapFragmentHints hints);
   virtual void drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects, int fragmentCount,
                                    const QPixmap &pixmap,
                                    QPainter::PixmapFragmentHints hints);

   virtual void clipEnabledChanged();
   virtual void penChanged();
   virtual void brushChanged();
   virtual void brushOriginChanged();
   virtual void opacityChanged();
   virtual void compositionModeChanged();
   virtual void renderHintsChanged();
   virtual void transformChanged();

   virtual void setState(QPainterState *s);

   virtual void beginNativePainting();
   virtual void endNativePainting();

   virtual uint flags() const {
      return QPaintEngineEx::IsEmulationEngine | QPaintEngineEx::DoNotEmulate;
   }

   inline QPainterState *state() {
      return (QPainterState *)QPaintEngine::state;
   }
   inline const QPainterState *state() const {
      return (const QPainterState *)QPaintEngine::state;
   }

   QPaintEngineEx *real_engine;

 private:
   void fillBGRect(const QRectF &r);
};

QT_END_NAMESPACE

#endif
