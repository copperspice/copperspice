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

#ifndef QPRINTER_H
#define QPRINTER_H

#include <qstring.h>
#include <qscopedpointer.h>
#include <qpagedpaintdevice.h>
#include <qpagelayout.h>

#ifndef QT_NO_PRINTER

class QPrinterPrivate;
class QPaintEngine;
class QPrintEngine;
class QPrinterInfo;
class QPageSize;
class QPageMargins;

class Q_GUI_EXPORT QPrinter : public QPagedPaintDevice
{
   Q_DECLARE_PRIVATE(QPrinter)

 public:
   using Orientation = QPageLayout::Orientation;
   using Unit = QPageSize::Unit;

   // Keep in sync with QAbstractPrintDialog::PrintRange
   enum PrintRange   { AllPages, Selection, PageRange, CurrentPage };

   enum ColorMode    { GrayScale, Color };
   enum OutputFormat { NativeFormat, PdfFormat };
   enum PageOrder    { FirstPageFirst, LastPageFirst };
   enum PrinterMode  { ScreenResolution, PrinterResolution, HighResolution };

   enum PaperSource {
      OnlyOne,
      Lower,
      Middle,
      Manual,
      Envelope,
      EnvelopeManual,
      Auto,
      Tractor,
      SmallFormat,
      LargeFormat,
      LargeCapacity,
      Cassette,
      FormSource,
      MaxPageSource,       // Deprecated
      CustomSource,
      LastPaperSource = CustomSource,
      Upper = OnlyOne     // As defined in Windows
   };

   enum PrinterState {
      Idle,
      Active,
      Aborted,
      Error
   };

   enum DuplexMode {
      DuplexNone = 0,
      DuplexAuto,
      DuplexLongSide,
      DuplexShortSide
   };

   explicit QPrinter(PrinterMode mode = ScreenResolution);
   explicit QPrinter(const QPrinterInfo &printer, PrinterMode mode = ScreenResolution);

   QPrinter(const QPrinter &) = delete;
   QPrinter &operator=(const QPrinter &) = delete;

   ~QPrinter();

   int devType() const override;

   void setOutputFormat(OutputFormat format);
   OutputFormat outputFormat() const;

   void setPrinterName(const QString &name);
   QString printerName() const;

   bool isValid() const;

   void setOutputFileName(const QString &fileName);
   QString outputFileName()const;

   void setPrintProgram(const QString &command);
   QString printProgram() const;

   void setDocName(const QString &name);
   QString docName() const;

   void setCreator(const QString &creator);
   QString creator() const;

   void setMargins(const QMarginsF &margins) override;
   QMarginsF margins(QPageSize::Unit unit) const;

   bool setPageLayout(const QPageLayout &newPageLayout) override;
   QPageLayout pageLayout() const override;

   bool setPageOrientation(QPageLayout::Orientation orientation) override;
   void setOrientation(Orientation orientation);
   Orientation orientation() const;

   bool setPageMargins(const QMarginsF &margins, QPageSize::Unit units) override;
   bool setPageMargins(const QMarginsF &margins) override;

   bool setPageSize(const QPageSize &pageSize) override;
   void setPageSize(QPageSize::PageSizeId sizeId) override;
   QPageSize::PageSizeId pageSize() const;

   void setPageSizeMM(const QSizeF &size) override;

   void setPaperSize(QPageSize::PageSizeId paperSize);
   void setPaperSize(const QSizeF &paperSize, Unit unit);
   QPageSize::PageSizeId paperSize() const;
   QSizeF paperSize(Unit unit) const;

   void setPaperName(const QString &paperName);
   QString paperName() const;

   void setPrintRange(PrintRange range);
   PrintRange printRange() const;

   void setPageOrder(PageOrder pageOrder);
   PageOrder pageOrder() const;

   void setResolution(int dpi);
   int resolution() const;

   void setColorMode(ColorMode colorMode);
   ColorMode colorMode() const;

   void setCollateCopies(bool collate);
   bool collateCopies() const;

   void setFullPage(bool fullPage);
   bool fullPage() const;

   void setNumCopies(int numCopies);
   int numCopies() const;

   int actualNumCopies() const;

   void setCopyCount(int count);
   int copyCount() const;
   bool supportsMultipleCopies() const;

   void setPaperSource(PaperSource source);
   PaperSource paperSource() const;

   void setDuplex(DuplexMode enable);
   DuplexMode duplex() const;

   QList<int> supportedResolutions() const;

#ifdef Q_OS_WIN
   QList<PaperSource> supportedPaperSources() const;
#endif

   void setFontEmbeddingEnabled(bool enable);
   bool fontEmbeddingEnabled() const;

   void setDoubleSidedPrinting(bool enable);
   bool doubleSidedPrinting() const;

   void setWinPageSize(int pageSize);
   int winPageSize() const;

   QRect paperRect() const;
   QRect pageRect() const;
   QRectF paperRect(Unit unit) const;
   QRectF pageRect(Unit unit) const;

   QString printerSelectionOption() const;
   void setPrinterSelectionOption(const QString &option);

   bool newPage() override;
   bool abort();

   PrinterState printerState() const;

   QPaintEngine *paintEngine() const override;
   QPrintEngine *printEngine() const;

   void setFromTo(int fromPage, int toPage);
   int fromPage() const;
   int toPage() const;

 protected:
   int metric(PaintDeviceMetric) const override;
   void setEngines(QPrintEngine *printEngine, QPaintEngine *paintEngine);

 private:
   QScopedPointer<QPrinterPrivate> d_ptr;

   friend class QPrintDialogPrivate;
   friend class QAbstractPrintDialog;
   friend class QAbstractPrintDialogPrivate;
   friend class QPrintPreviewWidgetPrivate;
   friend class QTextDocument;
   friend class QPageSetupWidget;
};

#endif // QT_NO_PRINTER

#endif
