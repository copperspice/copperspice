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

#ifndef QPAINTENGINE_PREVIEW_P_H
#define QPAINTENGINE_PREVIEW_P_H

#include <QtGui/qpaintengine.h>
#include <QtGui/qprintengine.h>

#ifndef QT_NO_PRINTPREVIEWWIDGET

QT_BEGIN_NAMESPACE

class QPreviewPaintEnginePrivate;

class QPreviewPaintEngine : public QPaintEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QPreviewPaintEngine)
 public:
   QPreviewPaintEngine();
   ~QPreviewPaintEngine();

   bool begin(QPaintDevice *dev);
   bool end();

   void updateState(const QPaintEngineState &state);

   void drawPath(const QPainterPath &path);
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
   void drawTextItem(const QPointF &p, const QTextItem &textItem);

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
   void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &p);

   QList<const QPicture *> pages();

   QPaintEngine::Type type() const {
      return Picture;
   }

   void setProxyEngines(QPrintEngine *printEngine, QPaintEngine *paintEngine);

   void setProperty(PrintEnginePropertyKey key, const QVariant &value);
   QVariant property(PrintEnginePropertyKey key) const;

   bool newPage();
   bool abort();

   int metric(QPaintDevice::PaintDeviceMetric) const;

   QPrinter::PrinterState printerState() const;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTPREVIEWWIDGET

#endif
