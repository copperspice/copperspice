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

#include <QtGui/qprintengine.h>

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

QT_BEGIN_NAMESPACE

// #define USE_NATIVE_GRADIENTS

class QImage;
class QDataStream;
class QPen;
class QPointF;
class QRegion;
class QFile;
class QPdfEngine;

class QPdfEnginePrivate;

class QPdfEngine : public QPdfBaseEngine
{
   Q_DECLARE_PRIVATE(QPdfEngine)

 public:
   QPdfEngine(QPrinter::PrinterMode m);
   virtual ~QPdfEngine();

   // reimplementations QPaintEngine
   bool begin(QPaintDevice *pdev) override;
   bool end() override;
   void drawPixmap (const QRectF &rectangle, const QPixmap &pixmap, const QRectF &sr) override;

   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                  Qt::ImageConversionFlags flags = Qt::AutoColor) override;

   void drawTiledPixmap (const QRectF &rectangle, const QPixmap &pixmap, const QPointF &point) override;
   Type type() const override;


   // reimplementations QPrintEngine
   bool abort() override {
      return false;
   }

   bool newPage() override;
   QPrinter::PrinterState printerState() const override {
      return state;
   }

   void setBrush() override;

   // ### unused, should have something for this in QPrintEngine
   void setAuthor(const QString &author);
   QString author() const;

   void setDevice(QIODevice *dev);

 private:
   Q_DISABLE_COPY(QPdfEngine)

   QPrinter::PrinterState state;
};

class QPdfEnginePrivate : public QPdfBaseEnginePrivate
{
   Q_DECLARE_PUBLIC(QPdfEngine)

 public:
   QPdfEnginePrivate(QPrinter::PrinterMode m);
   ~QPdfEnginePrivate();

   void newPage();

   int width() const {
      QRect r = paperRect();
      return qRound(r.width() * 72. / resolution);
   }

   int height() const {
      QRect r = paperRect();
      return qRound(r.height() * 72. / resolution);
   }

   void writeHeader();
   void writeTail();

   int addImage(const QImage &image, bool *bitmap, qint64 serial_no);
   int addConstantAlphaObject(int brushAlpha, int penAlpha = 255);
   int addBrushPattern(const QTransform &matrix, bool *specifyColor, int *gStateObject);

   void drawTextItem(const QPointF &p, const QTextItemInt &ti) override;

   QTransform pageMatrix() const;

 private:
   Q_DISABLE_COPY(QPdfEnginePrivate)

#ifdef USE_NATIVE_GRADIENTS
   int gradientBrush(const QBrush &b, const QMatrix &matrix, int *gStateObject);
#endif

   void writeInfo();
   void writePageRoot();
   void writeFonts();
   void embedFont(QFontSubset *font);

   QVector<int> xrefPositions;
   QDataStream *stream;
   int streampos;

   int writeImage(const QByteArray &data, int width, int height, int depth,
                  int maskObject, int softMaskObject, bool dct = false);
   void writePage();

   int addXrefEntry(int object, bool printostr = true);
   void printString(const QString &string);
   void xprintf(const char *fmt, ...);
   inline void write(const QByteArray &data) {
      stream->writeRawData(data.constData(), data.size());
      streampos += data.size();
   }

   int writeCompressed(const char *src, int len);
   inline int writeCompressed(const QByteArray &data) {
      return writeCompressed(data.constData(), data.length());
   }
   int writeCompressed(QIODevice *dev);

   // various PDF objects
   int pageRoot, catalog, info, graphicsState, patternColorSpace;
   QVector<uint> pages;
   QHash<qint64, uint> imageCache;
   QHash<QPair<uint, uint>, uint > alphaCache;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_PDF_P_H
