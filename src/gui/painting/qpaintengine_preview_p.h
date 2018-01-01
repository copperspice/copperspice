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

   bool begin(QPaintDevice *dev) override;
   bool end() override;

   void updateState(const QPaintEngineState &state) override;

   void drawPath(const QPainterPath &path) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;

   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &p) override;

   QList<const QPicture *> pages();

   QPaintEngine::Type type() const override {
      return Picture;
   }

   void setProxyEngines(QPrintEngine *printEngine, QPaintEngine *paintEngine);

   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

   bool newPage() override;
   bool abort() override;

   int metric(QPaintDevice::PaintDeviceMetric) const override;

   QPrinter::PrinterState printerState() const override;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTPREVIEWWIDGET

#endif
