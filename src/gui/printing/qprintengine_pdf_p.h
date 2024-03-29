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

#ifndef QPRINTENGINE_PDF_P_H
#define QPRINTENGINE_PDF_P_H

#include <qprintengine.h>

#ifndef QT_NO_PRINTER

#include <qmap.h>
#include <qmatrix.h>
#include <qstring.h>
#include <qvector.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qdatastream.h>

#include <qpdf_p.h>
#include <qpaintengine_p.h>
#include <qpaintengine_p.h>
#include <qprint_p.h>

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;

class QPdfPrintEnginePrivate;

class QPdfPrintEngine: public QPdfEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QPdfPrintEngine)

 public:
   QPdfPrintEngine(QPrinter::PrinterMode m);

   QPdfPrintEngine(const QPdfPrintEngine &) = delete;
   QPdfPrintEngine &operator=(const QPdfPrintEngine &) = delete;

   virtual ~QPdfPrintEngine();

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   bool abort() override {
      return false;
   }

   bool newPage() override;

   QPrinter::PrinterState printerState() const override {
      return state;
   }

   int metric(QPaintDevice::PaintDeviceMetric) const override;
   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

   QPrinter::PrinterState state;

 protected:
   QPdfPrintEngine(QPdfPrintEnginePrivate &p);
};

class Q_GUI_EXPORT QPdfPrintEnginePrivate : public QPdfEnginePrivate
{
   Q_DECLARE_PUBLIC(QPdfPrintEngine)

 public:
   QPdfPrintEnginePrivate(QPrinter::PrinterMode m);

   QPdfPrintEnginePrivate(const QPdfPrintEnginePrivate &) = delete;
   QPdfPrintEnginePrivate &operator=(const QPdfPrintEnginePrivate &) = delete;

   ~QPdfPrintEnginePrivate();

   virtual bool openPrintDevice();
   virtual void closePrintDevice();

 private:
   friend class QCupsPrintEngine;
   friend class QCupsPrintEnginePrivate;

   QString printerName;
   QString printProgram;
   QString selectionOption;
   QPrint::DuplexMode duplex;
   bool collate;
   int copies;
   QPrinter::PageOrder pageOrder;
   QPrinter::PaperSource paperSource;

   int fd;
};

#endif // QT_NO_PRINTER

#endif
