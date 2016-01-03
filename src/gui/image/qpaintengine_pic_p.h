/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QPAINTENGINE_PIC_P_H
#define QPAINTENGINE_PIC_P_H

#include <QtGui/qpaintengine.h>

#ifndef QT_NO_PICTURE

QT_BEGIN_NAMESPACE

class QPicturePaintEnginePrivate;
class QBuffer;

class QPicturePaintEngine : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QPicturePaintEngine)

 public:
   QPicturePaintEngine();
   ~QPicturePaintEngine();

   bool begin(QPaintDevice *pdev);
   bool end();

   void updateState(const QPaintEngineState &state);

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

   void drawEllipse(const QRectF &rect);
   void drawPath(const QPainterPath &path);
   void drawPolygon(const QPointF *points, int numPoints, PolygonDrawMode mode);

   using QPaintEngine::drawPolygon;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
   void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                  Qt::ImageConversionFlags flags = Qt::AutoColor);
   void drawTextItem(const QPointF &p, const QTextItem &ti);

   Type type() const {
      return Picture;
   }

 protected:
   QPicturePaintEngine(QPaintEnginePrivate &dptr);

 private:
   Q_DISABLE_COPY(QPicturePaintEngine)

   void writeCmdLength(int pos, const QRectF &r, bool corr);
};

QT_END_NAMESPACE

#endif // QT_NO_PICTURE

#endif // QPAINTENGINE_PIC_P_H
