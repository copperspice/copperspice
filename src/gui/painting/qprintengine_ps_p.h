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

#ifndef QPRINTENGINE_PS_P_H
#define QPRINTENGINE_PS_P_H

#ifndef QT_NO_PRINTER

#include <qpdf_p.h>
#include <qplatformdefs.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

class QPrinter;
class QPSPrintEnginePrivate;

class QPSPrintEngine : public QPdfBaseEngine
{
   Q_DECLARE_PRIVATE(QPSPrintEngine)

 public:
   // QPrinter uses these
   explicit QPSPrintEngine(QPrinter::PrinterMode m);
   ~QPSPrintEngine();

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   void setBrush() override;

   void drawImage(const QRectF &r, const QImage &img, const QRectF &sr, Qt::ImageConversionFlags) override;
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override;
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s) override;

   void drawImageInternal(const QRectF &r, QImage img, bool bitmap);

   QPaintEngine::Type type() const override {
      return QPaintEngine::PostScript;
   }

   bool newPage() override;
   bool abort() override;

   QPrinter::PrinterState printerState() const override;

   Qt::HANDLE handle() const {
      return 0;
   }

 private:
   Q_DISABLE_COPY(QPSPrintEngine)
};

class QPSPrintEnginePrivate : public QPdfBaseEnginePrivate
{
 public:
   QPSPrintEnginePrivate(QPrinter::PrinterMode m);
   ~QPSPrintEnginePrivate();

   void emitHeader(bool finished);
   void emitPages();
   void drawImage(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask);
   void flushPage(bool last = false);
   void drawImageHelper(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask,
                        bool gray, qreal scaleX, qreal scaleY);

   int pageCount;
   bool epsf;
   QByteArray fontsUsed;

   // stores the descriptions of the n first pages.
   QPdf::ByteStream buffer;
   QByteArray trailer;

   bool firstPage;

   QRect boundingBox;

   QPrinter::PrinterState printerState;
   bool hugeDocument;
   bool headerDone;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_PS_P_H
