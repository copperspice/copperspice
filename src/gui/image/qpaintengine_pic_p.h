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

#ifndef QPAINTENGINE_PIC_P_H
#define QPAINTENGINE_PIC_P_H

#include <qpaintengine.h>

#ifndef QT_NO_PICTURE

class QPicturePaintEnginePrivate;
class QBuffer;

class QPicturePaintEngine : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QPicturePaintEngine)

 public:
   QPicturePaintEngine();

   QPicturePaintEngine(const QPicturePaintEngine &) = delete;
   QPicturePaintEngine &operator=(const QPicturePaintEngine &) = delete;

   ~QPicturePaintEngine();

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   void updateState(const QPaintEngineState &state) override;

   void updatePen(const QPen &pen);
   void updateBrush(const QBrush &brush);
   void updateBrushOrigin(const QPointF &origin);
   void updateFont(const QFont &font);
   void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
   void updateMatrix(const QTransform &matrix);
   void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
   void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
   void updateRenderHints(QPainter::RenderHints hints);
   void updateCompositionMode(QPainter::CompositionMode cmode);
   void updateClipEnabled(bool enabled);
   void updateOpacity(qreal opacity);

   void drawEllipse(const QRectF &rect) override;
   void drawPath(const QPainterPath &path) override;
   void drawPolygon(const QPointF *points, int numPoints, PolygonDrawMode mode) override;

   using QPaintEngine::drawPolygon;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;
   void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
      Qt::ImageConversionFlags flags = Qt::AutoColor) override;
   void drawTextItem(const QPointF &p, const QTextItem &ti) override;

   Type type() const override {
      return Picture;
   }

 protected:
   QPicturePaintEngine(QPaintEnginePrivate &dptr);

 private:
   void writeCmdLength(int pos, const QRectF &r, bool corr);
};

#endif // QT_NO_PICTURE

#endif
