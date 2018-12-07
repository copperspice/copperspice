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

#include <qprinter_p.h>
#include <qprinter.h>

#ifndef QT_NO_PRINTER

#include <qcoreapplication.h>
#include <qpicture.h>
#include <qprintengine.h>
#include <qplatform_printplugin.h>
#include <qplatform_printersupport.h>
#include <qlist.h>
#include <qfileinfo.h>

#include <qpagedpaintdevice_p.h>
#include <qprintengine_pdf_p.h>
#include <qpaintengine_preview_p.h>

#define ABORT_IF_ACTIVE(location) \
    if (d->printEngine->printerState() == QPrinter::Active) { \
        qWarning("%s: Can not be changed while printer is active", location); \
        return; \
    }

#define ABORT_IF_ACTIVE_RETURN(location, retValue) \
    if (d->printEngine->printerState() == QPrinter::Active) { \
        qWarning("%s: Cannot be changed while printer is active", location); \
        return retValue; \
    }
extern qreal qt_pixelMultiplier(int resolution);
extern QMarginsF qt_convertMargins(const QMarginsF &margins, QPageLayout::Unit fromUnits, QPageLayout::Unit toUnits);

/// return the multiplier of converting from the unit value to postscript-points.
double qt_multiplierForUnit(QPrinter::Unit unit, int resolution)
{
   switch (unit) {
      case QPrinter::Millimeter:
         return 2.83464566929;
      case QPrinter::Point:
         return 1.0;
      case QPrinter::Inch:
         return 72.0;
      case QPrinter::Pica:
         return 12;
      case QPrinter::Didot:
         return 1.065826771;
      case QPrinter::Cicero:
         return 12.789921252;
      case QPrinter::DevicePixel:
         return 72.0 / resolution;
   }
   return 1.0;
}

// not static: it's needed in qpagesetupdialog_unix.cpp
QSizeF qt_printerPaperSize(QPrinter::Orientation orientation,
   QPrinter::PaperSize paperSize,
   QPrinter::Unit unit,
   int resolution)
{
   QPageSize pageSize = QPageSize(QPageSize::PageSizeId(paperSize));
   QSizeF sizef;
   if (unit == QPrinter::DevicePixel) {
      sizef = pageSize.size(QPageSize::Point) * qt_multiplierForUnit(unit, resolution);
   } else {
      sizef = pageSize.size(QPageSize::Unit(unit));
   }
   return orientation == QPrinter::Landscape ? sizef.transposed() : sizef;
}

QPrinterInfo QPrinterPrivate::findValidPrinter(const QPrinterInfo &printer)
{
   // Try find a valid printer to use, either the one given, the default or the first available
   QPrinterInfo printerToUse = printer;
   if (printerToUse.isNull()) {
      printerToUse = QPrinterInfo::defaultPrinter();
      if (printerToUse.isNull()) {
         QStringList availablePrinterNames = QPrinterInfo::availablePrinterNames();
         if (!availablePrinterNames.isEmpty()) {
            printerToUse = QPrinterInfo::printerInfo(availablePrinterNames.at(0));
         }
      }
   }
   return printerToUse;
}

void QPrinterPrivate::initEngines(QPrinter::OutputFormat format, const QPrinterInfo &printer)
{
   outputFormat = QPrinter::PdfFormat;
   QPlatformPrinterSupport *ps = 0;
   QString printerName;
   if (format == QPrinter::NativeFormat) {
      ps = QPlatformPrinterSupportPlugin::get();
      QPrinterInfo printerToUse = findValidPrinter(printer);
      if (ps && !printerToUse.isNull()) {
         outputFormat = QPrinter::NativeFormat;
         printerName = printerToUse.printerName();
      }
   }
   if (outputFormat == QPrinter::NativeFormat) {
      printEngine = ps->createNativePrintEngine(printerMode);
      paintEngine = ps->createPaintEngine(printEngine, printerMode);
   } else {
      QPdfPrintEngine *pdfEngine = new QPdfPrintEngine(printerMode);
      paintEngine = pdfEngine;
      printEngine = pdfEngine;
   }
   use_default_engine = true;
   had_default_engines = true;
   setProperty(QPrintEngine::PPK_PrinterName, printerName);
   validPrinter = true;
}

void QPrinterPrivate::changeEngines(QPrinter::OutputFormat format, const QPrinterInfo &printer)
{
   QPrintEngine *oldPrintEngine = printEngine;
   const bool def_engine = use_default_engine;

   initEngines(format, printer);

   if (oldPrintEngine) {
      foreach (QPrintEngine::PrintEnginePropertyKey key, m_properties) {
         QVariant prop;
         // PPK_NumberOfCopies need special treatmeant since it in most cases
         // will return 1, disregarding the actual value that was set
         // PPK_PrinterName also needs special treatment as initEngines has set it already
         if (key == QPrintEngine::PPK_NumberOfCopies) {
            prop = QVariant(q_ptr->copyCount());
         } else if (key != QPrintEngine::PPK_PrinterName) {
            prop = oldPrintEngine->property(key);
         }
         if (prop.isValid()) {
            setProperty(key, prop);
         }
      }
   }

   if (def_engine) {
      delete oldPrintEngine;
   }
}
#ifndef QT_NO_PRINTPREVIEWWIDGET
QList<const QPicture *> QPrinterPrivate::previewPages() const
{
   if (previewEngine) {
      return previewEngine->pages();
   }
   return QList<const QPicture *>();
}

void QPrinterPrivate::setPreviewMode(bool enable)
{
   Q_Q(QPrinter);
   if (enable) {
      if (!previewEngine) {
         previewEngine = new QPreviewPaintEngine;
      }
      had_default_engines = use_default_engine;
      use_default_engine = false;
      realPrintEngine = printEngine;
      realPaintEngine = paintEngine;
      q->setEngines(previewEngine, previewEngine);
      previewEngine->setProxyEngines(realPrintEngine, realPaintEngine);
   } else {
      q->setEngines(realPrintEngine, realPaintEngine);
      use_default_engine = had_default_engines;
   }
}
#endif // QT_NO_PRINTPREVIEWWIDGET

void QPrinterPrivate::setProperty(QPrintEngine::PrintEnginePropertyKey key, const QVariant &value)
{
   printEngine->setProperty(key, value);
   m_properties.insert(key);
}

class QPrinterPagedPaintDevicePrivate : public QPagedPaintDevicePrivate
{
 public:
   QPrinterPagedPaintDevicePrivate(QPrinterPrivate *d)
      : QPagedPaintDevicePrivate(), pd(d)
   {}

   virtual ~QPrinterPagedPaintDevicePrivate()
   {}

   bool setPageLayout(const QPageLayout &newPageLayout) override {
      if (pd->paintEngine->type() != QPaintEngine::Pdf
         && pd->printEngine->printerState() == QPrinter::Active) {
         qWarning("QPrinter::setPageLayout: Cannot be changed while printer is active");
         return false;
      }

      // Try to set the print engine page layout
      pd->setProperty(QPrintEngine::PPK_QPageLayout, QVariant::fromValue(newPageLayout));

      // Set QPagedPaintDevice layout to match the current print engine value
      m_pageLayout = pageLayout();

      return pageLayout().isEquivalentTo(newPageLayout);
   }

   bool setPageSize(const QPageSize &pageSize) override {
      if (pd->paintEngine->type() != QPaintEngine::Pdf
         && pd->printEngine->printerState() == QPrinter::Active) {
         qWarning("QPrinter::setPageLayout: Cannot be changed while printer is active");
         return false;
      }


      // Try to set the print engine page size
      pd->setProperty(QPrintEngine::PPK_QPageSize, QVariant::fromValue(pageSize));

      // Set QPagedPaintDevice layout to match the current print engine value
      m_pageLayout = pageLayout();

      return pageLayout().pageSize().isEquivalentTo(pageSize);
   }

   bool setPageOrientation(QPageLayout::Orientation orientation) override {
      // Set the print engine value
      pd->setProperty(QPrintEngine::PPK_Orientation, orientation);

      // Set QPagedPaintDevice layout to match the current print engine value
      m_pageLayout = pageLayout();

      return pageLayout().orientation() == orientation;
   }

   bool setPageMargins(const QMarginsF &margins) override {
      return setPageMargins(margins, pageLayout().units());
   }

   bool setPageMargins(const QMarginsF &margins, QPageLayout::Unit units) override {
      // Try to set print engine margins
      QPair<QMarginsF, QPageLayout::Unit> pair = qMakePair(margins, units);
      pd->setProperty(QPrintEngine::PPK_QPageMargins, QVariant::fromValue(pair));

      // Set QPagedPaintDevice layout to match the current print engine value
      m_pageLayout = pageLayout();

      return pageLayout().margins() == margins && pageLayout().units() == units;
   }

   QPageLayout pageLayout() const override {
      return pd->printEngine->property(QPrintEngine::PPK_QPageLayout).value<QPageLayout>();
   }

   QPrinterPrivate *pd;
};

QPrinter::QPrinter(PrinterMode mode)
   : QPagedPaintDevice(), d_ptr(new QPrinterPrivate(this))
{
   delete d_devicePtr;

   d_devicePtr = new QPrinterPagedPaintDevicePrivate(d_func());
   d_ptr->init(QPrinterInfo(), mode);
}

QPrinter::QPrinter(const QPrinterInfo &printer, PrinterMode mode)
   : QPagedPaintDevice(), d_ptr(new QPrinterPrivate(this))
{
   delete d_devicePtr;

   d_devicePtr = new QPrinterPagedPaintDevicePrivate(d_func());
   d_ptr->init(printer, mode);
}

void QPrinterPrivate::init(const QPrinterInfo &printer, QPrinter::PrinterMode mode)
{
   if (! QCoreApplication::instance()) {
      qFatal("QPrinter: Must construct a QCoreApplication before a QPrinter");
      return;
   }

   printerMode = mode;
   initEngines(QPrinter::NativeFormat, printer);
}

void QPrinter::setEngines(QPrintEngine *printEngine, QPaintEngine *paintEngine)
{
   Q_D(QPrinter);

   if (d->use_default_engine) {
      delete d->printEngine;
   }

   d->printEngine = printEngine;
   d->paintEngine = paintEngine;
   d->use_default_engine = false;
}

/*!
    Destroys the printer object and frees any allocated resources. If
    the printer is destroyed while a print job is in progress this may
    or may not affect the print job.
*/
QPrinter::~QPrinter()
{
   Q_D(QPrinter);

   if (d->use_default_engine) {
      delete d->printEngine;
   }

#ifndef QT_NO_PRINTPREVIEWWIDGET
   delete d->previewEngine;
#endif
}

void QPrinter::setOutputFormat(OutputFormat format)
{
   Q_D(QPrinter);
   if (d->outputFormat == format) {
      return;
   }

   if (format == QPrinter::NativeFormat) {
      QPrinterInfo printerToUse = d->findValidPrinter();
      if (!printerToUse.isNull()) {
         d->changeEngines(format, printerToUse);
      }
   } else {
      d->changeEngines(format, QPrinterInfo());
   }
}

QPrinter::OutputFormat QPrinter::outputFormat() const
{
   Q_D(const QPrinter);
   return d->outputFormat;
}



/*! \internal
*/
int QPrinter::devType() const
{
   return QInternal::Printer;
}


QString QPrinter::printerName() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_PrinterName).toString();
}

void QPrinter::setPrinterName(const QString &name)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setPrinterName");

   if (printerName() == name) {
      return;
   }
   if (name.isEmpty()) {
      setOutputFormat(QPrinter::PdfFormat);
      return;
   }

   QPrinterInfo printerToUse = QPrinterInfo::printerInfo(name);
   if (printerToUse.isNull()) {
      return;
   }

   if (outputFormat() == QPrinter::PdfFormat) {
      d->changeEngines(QPrinter::NativeFormat, printerToUse);
   } else {
      d->setProperty(QPrintEngine::PPK_PrinterName, name);
   }

}


bool QPrinter::isValid() const
{
   Q_D(const QPrinter);

   if (! qApp) {
      return false;
   }

   return d->validPrinter;
}



QString QPrinter::outputFileName() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_OutputFileName).toString();
}



void QPrinter::setOutputFileName(const QString &fileName)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setOutputFileName");

   QFileInfo fi(fileName);
   if (! fi.suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive)) {
      setOutputFormat(QPrinter::PdfFormat);

   } else if (fileName.isEmpty()) {
      setOutputFormat(QPrinter::NativeFormat);
   }

   d->setProperty(QPrintEngine::PPK_OutputFileName, fileName);

}

QString QPrinter::printProgram() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_PrinterProgram).toString();
}

void QPrinter::setPrintProgram(const QString &printProg)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setPrintProgram");
   d->setProperty(QPrintEngine::PPK_PrinterProgram, printProg);
}

QString QPrinter::docName() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_DocumentName).toString();
}


void QPrinter::setDocName(const QString &name)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setDocName");
   d->setProperty(QPrintEngine::PPK_DocumentName, name);
}

QString QPrinter::creator() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_Creator).toString();
}

void QPrinter::setCreator(const QString &creator)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setCreator");
   d->setProperty(QPrintEngine::PPK_Creator, creator);
}

QPrinter::Orientation QPrinter::orientation() const
{
   return QPrinter::Orientation(pageLayout().orientation());
}




void QPrinter::setOrientation(Orientation orientation)
{
   setPageOrientation(QPageLayout::Orientation(orientation));
}




QPrinter::PaperSize QPrinter::paperSize() const
{
   return pageSize();
}
void QPrinter::setPaperSize(PaperSize newPaperSize)
{
   setPageSize(QPageSize(QPageSize::PageSizeId(newPaperSize)));
}


QPrinter::PageSize QPrinter::pageSize() const
{
   return QPrinter::PaperSize(pageLayout().pageSize().id());
}



void QPrinter::setPageSize(PageSize newPageSize)
{
   setPageSize(QPageSize(QPageSize::PageSizeId(newPageSize)));
}



void QPrinter::setPaperSize(const QSizeF &paperSize, QPrinter::Unit unit)
{
   if (unit == QPrinter::DevicePixel) {
      setPageSize(QPageSize(paperSize * qt_pixelMultiplier(resolution()), QPageSize::Point));
   } else {
      setPageSize(QPageSize(paperSize, QPageSize::Unit(unit)));
   }
}


void QPrinter::setPageSizeMM(const QSizeF &size)
{
   setPageSize(QPageSize(size, QPageSize::Millimeter));
}


QSizeF QPrinter::paperSize(Unit unit) const
{
   if (unit == QPrinter::DevicePixel) {
      return pageLayout().fullRectPixels(resolution()).size();
   } else {
      return pageLayout().fullRect(QPageLayout::Unit(unit)).size();
   }
}

void QPrinter::setPaperName(const QString &paperName)
{
   Q_D(QPrinter);

   if (d->paintEngine->type() != QPaintEngine::Pdf) {
      ABORT_IF_ACTIVE("QPrinter::setPaperName");
   }

   d->setProperty(QPrintEngine::PPK_PaperName, paperName);
}
QString QPrinter::paperName() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_PaperName).toString();
}

void QPrinter::setPageOrder(PageOrder pageOrder)
{
   d_devicePtr->pageOrderAscending = (pageOrder == FirstPageFirst);

   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setPageOrder");
   d->setProperty(QPrintEngine::PPK_PageOrder, pageOrder);
}

QPrinter::PageOrder QPrinter::pageOrder() const
{
   Q_D(const QPrinter);
   return QPrinter::PageOrder(d->printEngine->property(QPrintEngine::PPK_PageOrder).toInt());
}

void QPrinter::setColorMode(ColorMode newColorMode)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setColorMode");
   d->setProperty(QPrintEngine::PPK_ColorMode, newColorMode);
}


/*!
  Returns the current color mode.

  \sa setColorMode()
*/
QPrinter::ColorMode QPrinter::colorMode() const
{
   Q_D(const QPrinter);
   return QPrinter::ColorMode(d->printEngine->property(QPrintEngine::PPK_ColorMode).toInt());
}



int QPrinter::numCopies() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_NumberOfCopies).toInt();
}


int QPrinter::actualNumCopies() const
{
   return copyCount();
}




void QPrinter::setNumCopies(int numCopies)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setNumCopies");
   d->setProperty(QPrintEngine::PPK_NumberOfCopies, numCopies);
}



void QPrinter::setCopyCount(int count)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setCopyCount;");
   d->setProperty(QPrintEngine::PPK_CopyCount, count);
}



int QPrinter::copyCount() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_CopyCount).toInt();
}



bool QPrinter::supportsMultipleCopies() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_SupportsMultipleCopies).toBool();
}


bool QPrinter::collateCopies() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_CollateCopies).toBool();
}


void QPrinter::setCollateCopies(bool collate)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setCollateCopies");
   d->setProperty(QPrintEngine::PPK_CollateCopies, collate);
}


void QPrinter::setFullPage(bool fp)
{
   Q_D(QPrinter);
   d->setProperty(QPrintEngine::PPK_FullPage, fp);
   // Set QPagedPaintDevice layout to match the current print engine value
   devicePageLayout() = pageLayout();
}


bool QPrinter::fullPage() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_FullPage).toBool();
}


void QPrinter::setResolution(int dpi)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setResolution");
   d->setProperty(QPrintEngine::PPK_Resolution, dpi);
}



int QPrinter::resolution() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_Resolution).toInt();
}

void QPrinter::setPaperSource(PaperSource source)
{
   Q_D(QPrinter);
   d->setProperty(QPrintEngine::PPK_PaperSource, source);
}

QPrinter::PaperSource QPrinter::paperSource() const
{
   Q_D(const QPrinter);
   return QPrinter::PaperSource(d->printEngine->property(QPrintEngine::PPK_PaperSource).toInt());
}

void QPrinter::setFontEmbeddingEnabled(bool enable)
{
   Q_D(QPrinter);
   d->setProperty(QPrintEngine::PPK_FontEmbedding, enable);
}


bool QPrinter::fontEmbeddingEnabled() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_FontEmbedding).toBool();
}


void QPrinter::setDoubleSidedPrinting(bool doubleSided)
{
   setDuplex(doubleSided ? DuplexAuto : DuplexNone);
}



bool QPrinter::doubleSidedPrinting() const
{
   return duplex() != DuplexNone;
}


void QPrinter::setDuplex(DuplexMode duplex)
{
   Q_D(QPrinter);
   d->setProperty(QPrintEngine::PPK_Duplex, duplex);
}

QPrinter::DuplexMode QPrinter::duplex() const
{
   Q_D(const QPrinter);
   return static_cast <DuplexMode> (d->printEngine->property(QPrintEngine::PPK_Duplex).toInt());
}


QRectF QPrinter::pageRect(Unit unit) const
{
   if (unit == QPrinter::DevicePixel) {
      return pageLayout().paintRectPixels(resolution());
   } else {
      return pageLayout().paintRect(QPageLayout::Unit(unit));
   }
}


QRectF QPrinter::paperRect(Unit unit) const
{
   // the page rect is in device pixels
   if (unit == QPrinter::DevicePixel) {
      return pageLayout().fullRectPixels(resolution());
   } else {
      return pageLayout().fullRect(QPageLayout::Unit(unit));
   }
}


QRect QPrinter::pageRect() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_PageRect).toRect();
}


QRect QPrinter::paperRect() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_PaperRect).toRect();
}


void QPrinter::setPageMargins(qreal left, qreal top, qreal right, qreal bottom, QPrinter::Unit unit)
{
   if (unit == QPrinter::DevicePixel) {
      QMarginsF margins = QMarginsF(left, top, right, bottom);
      margins *= qt_pixelMultiplier(resolution());
      margins = qt_convertMargins(margins, QPageLayout::Point, pageLayout().units());
      setPageMargins(margins, pageLayout().units());
   } else {
      setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Unit(unit));
   }
}


void QPrinter::setMargins(const Margins &m)
{
   setPageMargins(QMarginsF(m.left, m.top, m.right, m.bottom), QPageLayout::Millimeter);
}
void QPrinter::getPageMargins(qreal *left, qreal *top, qreal *right, qreal *bottom, QPrinter::Unit unit) const
{
   QMarginsF margins;
   if (unit == QPrinter::DevicePixel) {
      QMargins tmp = pageLayout().marginsPixels(resolution());
      margins = QMarginsF(tmp.left(), tmp.top(), tmp.right(), tmp.bottom());
   } else {
      margins = pageLayout().margins(QPageLayout::Unit(unit));
   }

   if (left) {
      *left = margins.left();
   }
   if (right) {
      *right = margins.right();
   }
   if (top) {
      *top = margins.top();
   }
   if (bottom) {
      *bottom = margins.bottom();
   }
}

/*!
    \internal

    Returns the metric for the given \a id.
*/
int QPrinter::metric(PaintDeviceMetric id) const
{
   Q_D(const QPrinter);
   return d->printEngine->metric(id);
}


QPaintEngine *QPrinter::paintEngine() const
{
   Q_D(const QPrinter);
   return d->paintEngine;
}


QPrintEngine *QPrinter::printEngine() const
{
   Q_D(const QPrinter);
   return d->printEngine;
}

void QPrinter::setWinPageSize(int pageSize)
{
   Q_D(QPrinter);
   ABORT_IF_ACTIVE("QPrinter::setWinPageSize");
   d->setProperty(QPrintEngine::PPK_WindowsPageSize, pageSize);
}


int QPrinter::winPageSize() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_WindowsPageSize).toInt();
}


QList<int> QPrinter::supportedResolutions() const
{
   Q_D(const QPrinter);

   QList<QVariant> varlist = d->printEngine->property(QPrintEngine::PPK_SupportedResolutions).toList();
   QList<int> intlist;

   const int numSupportedResolutions = varlist.size();

   for (int i = 0; i < numSupportedResolutions; ++i) {
      intlist << varlist.at(i).toInt();
   }

   return intlist;
}

bool QPrinter::newPage()
{
   Q_D(QPrinter);
   if (d->printEngine->printerState() != QPrinter::Active) {
      return false;
   }
   return d->printEngine->newPage();
}

bool QPrinter::abort()
{
   Q_D(QPrinter);
   return d->printEngine->abort();
}

QPrinter::PrinterState QPrinter::printerState() const
{
   Q_D(const QPrinter);
   return d->printEngine->printerState();
}

#ifdef Q_OS_WIN

QList<QPrinter::PaperSource> QPrinter::supportedPaperSources() const
{
   Q_D(const QPrinter);
   QVariant v = d->printEngine->property(QPrintEngine::PPK_PaperSources);

   QList<QVariant> variant_list = v.toList();
   QList<QPrinter::PaperSource> int_list;

   for (int i = 0; i < variant_list.size(); ++i) {
      int_list << (QPrinter::PaperSource) variant_list.at(i).toInt();
   }

   return int_list;
}

#endif

QString QPrinter::printerSelectionOption() const
{
   Q_D(const QPrinter);
   return d->printEngine->property(QPrintEngine::PPK_SelectionOption).toString();
}

void QPrinter::setPrinterSelectionOption(const QString &option)
{
   Q_D(QPrinter);
   d->setProperty(QPrintEngine::PPK_SelectionOption, option);
}

int QPrinter::fromPage() const
{
   return d_devicePtr->fromPage;
}

int QPrinter::toPage() const
{
   return d_devicePtr->toPage;
}

void QPrinter::setFromTo(int from, int to)
{
   if (from > to) {
      qWarning() << "QPrinter::setFromTo: 'from' must be less than or equal to 'to'";
      from = to;
   }

   d_devicePtr->fromPage = from;
   d_devicePtr->toPage = to;
}

void QPrinter::setPrintRange( PrintRange range )
{
   d_devicePtr->printSelectionOnly = (range == Selection);
   Q_D(QPrinter);
   d->printRange = range;
}

QPrinter::PrintRange QPrinter::printRange() const
{
   Q_D(const QPrinter);
   return d->printRange;
}


#endif // QT_NO_PRINTER
