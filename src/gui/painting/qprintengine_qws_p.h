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

#ifndef QPRINTENGINE_QWS_P_H
#define QPRINTENGINE_QWS_P_H

#include <QtGui/qprinter.h>

#ifndef QT_NO_PRINTER

#include <QtGui/qprintengine.h>
#include <QtCore/qbytearray.h>
#include <qpaintengine_p.h>

QT_BEGIN_NAMESPACE

class QtopiaPrintEnginePrivate;
class QRasterPaintEngine;
class QPrinterPrivate;
class QImage;

class QtopiaPrintEngine : public QPaintEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QtopiaPrintEngine)

 public:
   QtopiaPrintEngine(QPrinter::PrinterMode mode);

   // override QWSPaintEngine
   bool begin(QPaintDevice *dev);
   bool end();
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
   void drawTextItem(const QPointF &p, const QTextItem &ti);
   QPaintEngine::Type type() const {
      return QPaintEngine::X11;
   }

   QPaintEngine *paintEngine() const;

   void updateState(const QPaintEngineState &state);

   QRect paperRect() const;
   QRect pageRect() const;

   bool newPage();
   bool abort();

   QPrinter::PrinterState printerState() const;

   int metric(QPaintDevice::PaintDeviceMetric metricType) const;

   QVariant property(PrintEnginePropertyKey key) const;
   void setProperty(PrintEnginePropertyKey key, const QVariant &value);

 private:
   friend class QPrintDialog;
   friend class QPageSetupDialog;

   void clearPage();
   void flushPage();
};

class QtopiaPrintBuffer
{
 public:
   QtopiaPrintBuffer( bool bigEndian = FALSE ) {
      _bigEndian = bigEndian;
   }
   ~QtopiaPrintBuffer() {}

   const QByteArray &data() const {
      return _data;
   }

   int size() const {
      return _data.size();
   }

   void clear() {
      _data.clear();
   }

   void append( char value ) {
      _data.append( value );
   }
   void append( short value );
   void append( int value );
   void append( const QByteArray &array ) {
      _data.append( array );
   }

   void patch( int posn, int value );

   void pad();

 private:
   QByteArray _data;
   bool _bigEndian;
};

#define	QT_QWS_PRINTER_DEFAULT_DPI	   200

class QtopiaPrintEnginePrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QtopiaPrintEngine)

 public:
   QtopiaPrintEnginePrivate(QPrinter::PrinterMode m) :
      mode(m),
      printerState(QPrinter::Idle),
      orientation(QPrinter::Portrait),
      paperSize(QPrinter::A4),
      pageOrder(QPrinter::FirstPageFirst),
      colorMode(QPrinter::GrayScale),
      paperSource(QPrinter::OnlyOne),
      resolution(QT_QWS_PRINTER_DEFAULT_DPI),
      _paintEngine(0),
      numCopies(1),
      outputToFile(false),
      fullPage(false),
      collateCopies(false),
      pageNumber(0),
      pageImage(0),
      partialByte(0),
      partialBits(0) {
   }
   ~QtopiaPrintEnginePrivate();

   void initialize();
   QPaintEngine *paintEngine();

   QPrinter::PrinterMode mode;

   QString printerName;
   QString outputFileName;
   QString printProgram;
   QString docName;
   QString creator;

   QPrinter::PrinterState printerState;

   QPrinter::Orientation orientation;
   QPrinter::PaperSize paperSize;
   QPrinter::PageOrder pageOrder;
   QPrinter::ColorMode colorMode;
   QPrinter::PaperSource paperSource;

   int resolution;
   QPaintEngine *_paintEngine;
   int numCopies;

   bool outputToFile;
   bool fullPage;
   bool collateCopies;

   int pageNumber;

   QImage *pageImage;

   QtopiaPrintBuffer buffer;

   // Definitions that are only relevant to G3FAX output.
   int ifdPatch;
   int partialByte;
   int partialBits;
   void writeG3FaxHeader();
   void writeG3FaxPage();
   int writeG3IFDEntry( int tag, int type, int count, int value );
   void writeG3Code( int code, int bits );
   void writeG3WhiteRun( int len );
   void writeG3BlackRun( int len );
   void writeG3EOL();
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_QWS_P_H
