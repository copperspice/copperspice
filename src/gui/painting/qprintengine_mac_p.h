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

#ifndef QPRINTENGINE_MAC_P_H
#define QPRINTENGINE_MAC_P_H

#ifndef QT_NO_PRINTER

#include <QtGui/qprinter.h>
#include <QtGui/qprintengine.h>
#include <qpaintengine_mac_p.h>
#include <qpainter_p.h>

#ifdef __OBJC__
@class NSPrintInfo;
#else
typedef void NSPrintInfo;
#endif

QT_BEGIN_NAMESPACE

class QPrinterPrivate;
class QMacPrintEnginePrivate;

class QMacPrintEngine : public QPaintEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QMacPrintEngine)

 public:
   QMacPrintEngine(QPrinter::PrinterMode mode);

   Qt::HANDLE handle() const;

   bool begin(QPaintDevice *dev) override;
   bool end() override;

   virtual QPaintEngine::Type type() const override {
      return QPaintEngine::MacPrinter;
   }

   QPaintEngine *paintEngine() const;

   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

   QPrinter::PrinterState printerState() const override;

   bool newPage() override;
   bool abort() override;
   int metric(QPaintDevice::PaintDeviceMetric) const override;

   void updateState(const QPaintEngineState &state) override;

   void drawLines(const QLineF *lines, int lineCount) override;
   void drawRects(const QRectF *r, int num) override;
   void drawPoints(const QPointF *p, int pointCount) override;
   void drawEllipse(const QRectF &r) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags) override;
   void drawTextItem(const QPointF &p, const QTextItem &ti) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;
   void drawPath(const QPainterPath &) override;

 private:
   friend class QPrintDialog;
   friend class QPageSetupDialog;
};

class QMacPrintEnginePrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QMacPrintEngine)

 public:
   QPrinter::PrinterMode mode;
   QPrinter::PrinterState state;
   QPrinter::Orientation orient;
   NSPrintInfo *printInfo;
   PMPageFormat format;
   PMPrintSettings settings;
   PMPrintSession session;
   PMResolution resolution;
   QString outputFilename;
   bool fullPage;
   QPaintEngine *paintEngine;
   bool suppressStatus;
   bool hasCustomPaperSize;
   QSizeF customSize;
   bool hasCustomPageMargins;
   qreal leftMargin;
   qreal topMargin;
   qreal rightMargin;
   qreal bottomMargin;
   QHash<QMacPrintEngine::PrintEnginePropertyKey, QVariant> valueCache;
   PMPaper customPaper;

   QMacPrintEnginePrivate() : mode(QPrinter::ScreenResolution), state(QPrinter::Idle),
      orient(QPrinter::Portrait), printInfo(0), format(0), settings(0),
      session(0), paintEngine(0), suppressStatus(false),
      hasCustomPaperSize(false), hasCustomPageMargins(false) {}

   ~QMacPrintEnginePrivate();

   void initialize();
   void releaseSession();
   bool newPage_helper();
   void setPaperSize(QPrinter::PaperSize ps);
   QPrinter::PaperSize paperSize() const;
   QList<QVariant> supportedResolutions() const;

   inline bool isPrintSessionInitialized() const {
      return printInfo != 0;
   }

   bool shouldSuppressStatus() const;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
