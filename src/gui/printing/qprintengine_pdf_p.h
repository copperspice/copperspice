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

#ifndef QPRINTENGINE_PDF_P_H
#define QPRINTENGINE_PDF_P_H

#include <qprintengine.h>

#ifndef QT_NO_PRINTER
#include <QtCore/qmap.h>
#include <QtGui/qmatrix.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <QtGui/qpaintengine.h>
#include <QtGui/qpainterpath.h>
#include <QtCore/qdatastream.h>
#include <qfontengine_p.h>
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

class QPdfEnginePrivate;

class QPdfPrintEngine: public QPdfEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QPdfPrintEngine)

 public:
   QPdfPrintEngine(QPrinter::PrinterMode m);
   virtual ~QPdfPrintEngine();

   // reimplementations QPaintEngine
   bool begin(QPaintDevice *pdev) Q_DECL_OVERRIDE;
   bool end() Q_DECL_OVERRIDE;

   // reimplementations QPrintEngine
   bool abort() override {
      return false;
   }

   bool newPage() override;
   QPrinter::PrinterState printerState() const override {
      return state;
   }

   int metric(QPaintDevice::PaintDeviceMetric) const override;
   virtual void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   virtual QVariant property(PrintEnginePropertyKey key) const override;

    QPrinter::PrinterState state;
 protected:
    QPdfPrintEngine(QPdfPrintEnginePrivate &p);

 private:
    Q_DISABLE_COPY(QPdfPrintEngine)

};

class Q_GUI_EXPORT QPdfPrintEnginePrivate : public QPdfEnginePrivate
{
    Q_DECLARE_PUBLIC(QPdfPrintEngine)

 public:
    QPdfPrintEnginePrivate(QPrinter::PrinterMode m);
    ~QPdfPrintEnginePrivate();


    virtual bool openPrintDevice();
    virtual void closePrintDevice();
 private:
    Q_DISABLE_COPY(QPdfPrintEnginePrivate)


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

};

#endif // QT_NO_PRINTER

#endif
