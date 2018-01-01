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

#ifndef QPDF_P_H
#define QPDF_P_H

#include <QtGui/qmatrix.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <qstroker_p.h>
#include <qfontengine_p.h>
#include <QtGui/qprinter.h>
#include <qfontsubset_p.h>
#include <qpaintengine_alpha_p.h>
#include <qprintengine.h>
#include <qbuffer.h>

#ifndef QT_NO_PRINTER

QT_BEGIN_NAMESPACE

#define PPK_CupsOptions QPrintEngine::PrintEnginePropertyKey(0xfe00)
#define PPK_CupsPageRect QPrintEngine::PrintEnginePropertyKey(0xfe01)
#define PPK_CupsPaperRect QPrintEngine::PrintEnginePropertyKey(0xfe02)
#define PPK_CupsStringPageSize QPrintEngine::PrintEnginePropertyKey(0xfe03)

const char *qt_real_to_string(qreal val, char *buf);
const char *qt_int_to_string(int val, char *buf);

namespace QPdf {

class ByteStream
{
 public:
   // fileBacking means that ByteStream will buffer the contents on disk
   // if the size exceeds a certain threshold. In this case, if a byte
   // array was passed in, its contents may no longer correspond to the
   // ByteStream contents.
   explicit ByteStream(bool fileBacking = false);
   explicit ByteStream(QByteArray *ba, bool fileBacking = false);
   ~ByteStream();
   ByteStream &operator <<(char chr);
   ByteStream &operator <<(const char *str);
   ByteStream &operator <<(const QByteArray &str);
   ByteStream &operator <<(const ByteStream &src);
   ByteStream &operator <<(qreal val);
   ByteStream &operator <<(int val);
   ByteStream &operator <<(const QPointF &p);

   // Note that the stream may be invalidated by calls that insert data.
   QIODevice *stream();
   void clear();

   static inline int maxMemorySize() {
      return 100000000;
   }
   static inline int chunkSize()     {
      return 10000000;
   }

 protected:
   void constructor_helper(QIODevice *dev);
   void constructor_helper(QByteArray *ba);

 private:
   void prepareBuffer();
 
   QIODevice *dev;
   QByteArray ba;
   bool fileBackingEnabled;
   bool fileBackingActive;
   bool handleDirty;
};

enum PathFlags {
   ClipPath,
   FillPath,
   StrokePath,
   FillAndStrokePath
};
QByteArray generatePath(const QPainterPath &path, const QTransform &matrix, PathFlags flags);
QByteArray generateMatrix(const QTransform &matrix);
QByteArray generateDashes(const QPen &pen);
QByteArray patternForBrush(const QBrush &b);

#ifdef USE_NATIVE_GRADIENTS
QByteArray generateLinearGradientShader(const QLinearGradient *lg, const QPointF *page_rect, bool alpha = false);
#endif

struct Stroker {
   Stroker();
   void setPen(const QPen &pen);
   void strokePath(const QPainterPath &path);
   ByteStream *stream;
   bool first;
   QTransform matrix;
   bool cosmeticPen;

 private:
   QStroker basicStroker;
   QDashStroker dashStroker;
   QStrokerOps *stroker;
};

QByteArray ascii85Encode(const QByteArray &input);

const char *toHex(ushort u, char *buffer);
const char *toHex(uchar u, char *buffer);


struct PaperSize {
   int width, height; // in postscript points
};
PaperSize paperSize(QPrinter::PaperSize paperSize);
const char *paperSizeToString(QPrinter::PaperSize paperSize);

}


class QPdfPage : public QPdf::ByteStream
{
 public:
   QPdfPage();

   QVector<uint> images;
   QVector<uint> graphicStates;
   QVector<uint> patterns;
   QVector<uint> fonts;
   QVector<uint> annotations;

   void streamImage(int w, int h, int object);

   QSize pageSize;
 private:
};


class QPdfBaseEnginePrivate;

class QPdfBaseEngine : public QAlphaPaintEngine, public QPrintEngine
{
   Q_DECLARE_PRIVATE(QPdfBaseEngine)

 public:
   QPdfBaseEngine(QPdfBaseEnginePrivate &d, PaintEngineFeatures f);
   ~QPdfBaseEngine() {}

   // reimplementations QPaintEngine
   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   void drawPoints(const QPointF *points, int pointCount) override;
   void drawLines(const QLineF *lines, int lineCount) override;
   void drawRects(const QRectF *rects, int rectCount) override;
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPath (const QPainterPath &path) override;

   void drawTextItem(const QPointF &p, const QTextItem &textItem) override;

   void updateState(const QPaintEngineState &state) override;

   int metric(QPaintDevice::PaintDeviceMetric metricType) const override;
  
   // printer stuff
   bool newPage() override;
   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

   void setPen();
   virtual void setBrush() = 0;
   void setupGraphicsState(QPaintEngine::DirtyFlags flags);

 private:
   void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
};

class QPdfBaseEnginePrivate : public QAlphaPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QPdfBaseEngine)

 public:
   QPdfBaseEnginePrivate(QPrinter::PrinterMode m);
   ~QPdfBaseEnginePrivate();

   bool openPrintDevice();
   void closePrintDevice();


   virtual void drawTextItem(const QPointF &p, const QTextItemInt &ti);
   inline uint requestObject() {
      return currentObject++;
   }

   QRect paperRect() const;
   QRect pageRect() const;

   bool postscript;
   int currentObject;

   QPdfPage *currentPage;
   QPdf::Stroker stroker;

   QPointF brushOrigin;
   QBrush brush;
   QPen pen;
   QList<QPainterPath> clips;
   bool clipEnabled;
   bool allClipped;
   bool hasPen;
   bool hasBrush;
   bool simplePen;
   qreal opacity;
   bool useAlphaEngine;

   QHash<QFontEngine::FaceId, QFontSubset *> fonts;

   QPaintDevice *pdev;

   // the device the output is in the end streamed to.
   QIODevice *outDevice;
   int fd;

   // printer options
   QString outputFileName;
   QString printerName;
   QString printProgram;
   QString selectionOption;
   QString title;
   QString creator;
   QPrinter::DuplexMode duplex;
   bool collate;
   bool fullPage;
   bool embedFonts;
   int copies;
   int resolution;
   QPrinter::PageOrder pageOrder;
   QPrinter::Orientation orientation;
   QPrinter::PaperSize paperSize;
   QPrinter::ColorMode colorMode;
   QPrinter::PaperSource paperSource;

   QStringList cupsOptions;
   QRect cupsPaperRect;
   QRect cupsPageRect;
   QString cupsStringPageSize;
   QSizeF customPaperSize; // in postscript points
   bool hasCustomPageMargins;
   qreal leftMargin, topMargin, rightMargin, bottomMargin;

#if !defined(QT_NO_CUPS)
   QString cupsTempFile;
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPDF_P_H

