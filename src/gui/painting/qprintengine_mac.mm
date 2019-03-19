/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qprintengine_mac_p.h>
#include <qthread.h>
#include <quuid.h>
#include <qcoreapplication.h>

#ifndef QT_NO_PRINTER

QT_BEGIN_NAMESPACE

extern QSizeF qt_paperSizeToQSizeF(QPrinter::PaperSize size);

QMacPrintEngine::QMacPrintEngine(QPrinter::PrinterMode mode) : QPaintEngine(*(new QMacPrintEnginePrivate))
{
   Q_D(QMacPrintEngine);
   d->mode = mode;
   d->initialize();
}

bool QMacPrintEngine::begin(QPaintDevice *dev)
{
   Q_D(QMacPrintEngine);

   Q_ASSERT(dev && dev->devType() == QInternal::Printer);
   if (!static_cast<QPrinter *>(dev)->isValid()) {
      return false;
   }

   if (d->state == QPrinter::Idle && !d->isPrintSessionInitialized()) { // Need to reinitialize
      d->initialize();
   }

   d->paintEngine->state = state;
   d->paintEngine->begin(dev);
   Q_ASSERT_X(d->state == QPrinter::Idle, "QMacPrintEngine", "printer already active");

   if (PMSessionValidatePrintSettings(d->session, d->settings, kPMDontWantBoolean) != noErr
         || PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean) != noErr) {
      d->state = QPrinter::Error;
      return false;
   }

   if (!d->outputFilename.isEmpty()) {
      QCFType<CFURLRef> outFile = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault,
                                  QCFString(d->outputFilename),
                                  kCFURLPOSIXPathStyle,
                                  false);
      if (PMSessionSetDestination(d->session, d->settings, kPMDestinationFile,
                                  kPMDocumentFormatPDF, outFile) != noErr) {
         qWarning("QMacPrintEngine::begin: Problem setting file [%s]", d->outputFilename.toUtf8().constData());
         return false;
      }
   }

   OSStatus status = noErr;
   status = PMSessionBeginCGDocumentNoDialog(d->session, d->settings, d->format);


   if (status != noErr) {
      d->state = QPrinter::Error;
      return false;
   }

   d->state = QPrinter::Active;
   setActive(true);
   d->newPage_helper();
   return true;
}

bool QMacPrintEngine::end()
{
   Q_D(QMacPrintEngine);
   if (d->state == QPrinter::Aborted) {
      return true;   // I was just here a function call ago :)
   }
   if (d->paintEngine->type() == QPaintEngine::CoreGraphics) {
      // We dont need the paint engine to call restoreGraphicsState()
      static_cast<QCoreGraphicsPaintEngine *>(d->paintEngine)->d_func()->stackCount = 0;
      static_cast<QCoreGraphicsPaintEngine *>(d->paintEngine)->d_func()->hd = 0;
   }
   d->paintEngine->end();
   if (d->state != QPrinter::Idle) {
      d->releaseSession();
   }
   d->state  = QPrinter::Idle;
   return true;
}

QPaintEngine *
QMacPrintEngine::paintEngine() const
{
   return d_func()->paintEngine;
}

Qt::HANDLE QMacPrintEngine::handle() const
{
   QCoreGraphicsPaintEngine *cgEngine = static_cast<QCoreGraphicsPaintEngine *>(paintEngine());
   return cgEngine->d_func()->hd;
}

QMacPrintEnginePrivate::~QMacPrintEnginePrivate()
{
   [printInfo release];
   delete paintEngine;
}

void QMacPrintEnginePrivate::setPaperSize(QPrinter::PaperSize ps)
{
   Q_Q(QMacPrintEngine);
   if (hasCustomPaperSize) {
      PMRelease(customPaper);
      customPaper = 0;
   }
   hasCustomPaperSize = (ps == QPrinter::Custom);
   PMPrinter printer;

   if (PMSessionGetCurrentPrinter(session, &printer) == noErr) {
      if (ps != QPrinter::Custom) {
         QSize newSize = qt_paperSizeToQSizeF(ps).toSize();
         QCFType<CFArrayRef> formats;
         if (PMSessionCreatePageFormatList(session, printer, &formats) == noErr) {
            CFIndex total = CFArrayGetCount(formats);
            PMPageFormat tmp;
            PMRect paper;
            for (CFIndex idx = 0; idx < total; ++idx) {
               tmp = static_cast<PMPageFormat>(const_cast<void *>(CFArrayGetValueAtIndex(formats, idx)));
               PMGetUnadjustedPaperRect(tmp, &paper);
               int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
               int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
               if (newSize.width() == wMM && newSize.height() == hMM) {
                  PMCopyPageFormat(tmp, format);
                  // reset the orientation and resolution as they are lost in the copy.
                  q->setProperty(QPrintEngine::PPK_Orientation, orient);
                  if (PMSessionValidatePageFormat(session, format, kPMDontWantBoolean) != noErr) {
                     // Don't know, warn for the moment.
                     qWarning("QMacPrintEngine, problem setting format and resolution for this page size");
                  }
                  break;
               }
            }
         }
      } else {
         QCFString paperId = QCFString::toCFStringRef(QUuid::createUuid().toString());
         PMPaperMargins paperMargins;
         paperMargins.left = leftMargin;
         paperMargins.top = topMargin;
         paperMargins.right = rightMargin;
         paperMargins.bottom = bottomMargin;
         PMPaperCreateCustom(printer, paperId, QCFString("Custom size"), customSize.width(), customSize.height(), &paperMargins,
                             &customPaper);
         PMPageFormat tmp;
         PMCreatePageFormatWithPMPaper(&tmp, customPaper);
         PMCopyPageFormat(tmp, format);
         if (PMSessionValidatePageFormat(session, format, kPMDontWantBoolean) != noErr) {
            // Don't know, warn for the moment.
            qWarning("QMacPrintEngine, problem setting paper name");
         }
      }
   }
}

QPrinter::PaperSize QMacPrintEnginePrivate::paperSize() const
{
   if (hasCustomPaperSize) {
      return QPrinter::Custom;
   }
   PMRect paper;
   PMGetUnadjustedPaperRect(format, &paper);
   int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
   int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
   for (int i = QPrinter::A4; i < QPrinter::NPaperSize; ++i) {
      QSize s = qt_paperSizeToQSizeF(QPrinter::PaperSize(i)).toSize();
      if (s.width() == wMM && s.height() == hMM) {
         return (QPrinter::PaperSize)i;
      }
   }
   return QPrinter::Custom;
}

QList<QVariant> QMacPrintEnginePrivate::supportedResolutions() const
{
   Q_ASSERT_X(session, "QMacPrinterEngine::supportedResolutions",
              "must have a valid printer session");
   UInt32 resCount;
   QList<QVariant> resolutions;
   PMPrinter printer;
   if (PMSessionGetCurrentPrinter(session, &printer) == noErr) {
      PMResolution res;
      OSStatus status = PMPrinterGetPrinterResolutionCount(printer, &resCount);
      if (status  == kPMNotImplemented) {

      } else if (status == noErr) {
         // According to the docs, index start at 1.
         for (UInt32 i = 1; i <= resCount; ++i) {
            if (PMPrinterGetIndexedPrinterResolution(printer, i, &res) == noErr) {
               resolutions.append(QVariant(int(res.hRes)));
            }
         }
      } else {
         qWarning("QMacPrintEngine::supportedResolutions: Unexpected error: %ld", long(status));
      }
   }
   return resolutions;
}

bool QMacPrintEnginePrivate::shouldSuppressStatus() const
{
   if (suppressStatus == true) {
      return true;
   }

   // Supress displaying the automatic progress dialog if we are printing
   // from a non-gui thread.
   return (qApp->thread() != QThread::currentThread());
}

QPrinter::PrinterState QMacPrintEngine::printerState() const
{
   return d_func()->state;
}

bool QMacPrintEngine::newPage()
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);

   OSStatus err =  PMSessionEndPageNoDialog(d->session);

   if (err != noErr)  {
      if (err == kPMCancel) {
         // User canceled, we need to abort!
         abort();
      } else {
         // Not sure what the problem is...
         qWarning("QMacPrintEngine::newPage: Cannot end current page. %ld", long(err));
         d->state = QPrinter::Error;
      }
      return false;
   }
   return d->newPage_helper();
}

bool QMacPrintEngine::abort()
{
   Q_D(QMacPrintEngine);
   if (d->state != QPrinter::Active) {
      return false;
   }
   bool ret = end();
   d->state = QPrinter::Aborted;
   return ret;
}

static inline int qt_get_PDMWidth(PMPageFormat pformat, bool fullPage,
                                  const PMResolution &resolution)
{
   int val = 0;
   PMRect r;
   qreal hRatio = resolution.hRes / 72;
   if (fullPage) {
      if (PMGetAdjustedPaperRect(pformat, &r) == noErr) {
         val = qRound((r.right - r.left) * hRatio);
      }
   } else {
      if (PMGetAdjustedPageRect(pformat, &r) == noErr) {
         val = qRound((r.right - r.left) * hRatio);
      }
   }
   return val;
}

static inline int qt_get_PDMHeight(PMPageFormat pformat, bool fullPage,
                                   const PMResolution &resolution)
{
   int val = 0;
   PMRect r;
   qreal vRatio = resolution.vRes / 72;
   if (fullPage) {
      if (PMGetAdjustedPaperRect(pformat, &r) == noErr) {
         val = qRound((r.bottom - r.top) * vRatio);
      }
   } else {
      if (PMGetAdjustedPageRect(pformat, &r) == noErr) {
         val = qRound((r.bottom - r.top) * vRatio);
      }
   }
   return val;
}


int QMacPrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
   Q_D(const QMacPrintEngine);
   int val = 1;
   switch (m) {
      case QPaintDevice::PdmWidth:
         if (d->hasCustomPaperSize) {
            val = qRound(d->customSize.width());
            if (d->hasCustomPageMargins) {
               val -= qRound(d->leftMargin + d->rightMargin);
            } else {
               QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
               val -= qRound(margins.at(0).toDouble() + margins.at(2).toDouble());
            }
         } else {
            val = qt_get_PDMWidth(d->format, property(PPK_FullPage).toBool(), d->resolution);
         }
         break;
      case QPaintDevice::PdmHeight:
         if (d->hasCustomPaperSize) {
            val = qRound(d->customSize.height());
            if (d->hasCustomPageMargins) {
               val -= qRound(d->topMargin + d->bottomMargin);
            } else {
               QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
               val -= qRound(margins.at(1).toDouble() + margins.at(3).toDouble());
            }
         } else {
            val = qt_get_PDMHeight(d->format, property(PPK_FullPage).toBool(), d->resolution);
         }
         break;
      case QPaintDevice::PdmWidthMM:
         val = metric(QPaintDevice::PdmWidth);
         val = int((val * 254 + 5 * d->resolution.hRes) / (10 * d->resolution.hRes));
         break;
      case QPaintDevice::PdmHeightMM:
         val = metric(QPaintDevice::PdmHeight);
         val = int((val * 254 + 5 * d->resolution.vRes) / (10 * d->resolution.vRes));
         break;
      case QPaintDevice::PdmPhysicalDpiX:
      case QPaintDevice::PdmPhysicalDpiY: {
         PMPrinter printer;
         if (PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
            PMResolution resolution;

            PMPrinterGetOutputResolution(printer, d->settings, &resolution);
            val = (int)resolution.vRes;
            break;
         }
         //otherwise fall through
      }
      case QPaintDevice::PdmDpiY:
         val = (int)d->resolution.vRes;
         break;
      case QPaintDevice::PdmDpiX:
         val = (int)d->resolution.hRes;
         break;
      case QPaintDevice::PdmNumColors:
         val = (1 << metric(QPaintDevice::PdmDepth));
         break;
      case QPaintDevice::PdmDepth:
         val = 24;
         break;
      default:
         val = 0;
         qWarning("QPrinter::metric: Invalid metric command");
   }
   return val;
}

void QMacPrintEnginePrivate::initialize()
{
   Q_Q(QMacPrintEngine);

   Q_ASSERT(!printInfo);

   if (!paintEngine) {
      paintEngine = new QCoreGraphicsPaintEngine();
   }

   q->gccaps = paintEngine->gccaps;

   fullPage = false;


   QMacCocoaAutoReleasePool pool;
   printInfo = [[NSPrintInfo alloc] initWithDictionary: [NSDictionary dictionary]];
   session = static_cast<PMPrintSession>([printInfo PMPrintSession]);

   PMPrinter printer;
   if (session && PMSessionGetCurrentPrinter(session, &printer) == noErr) {
      QList<QVariant> resolutions = supportedResolutions();
      if (!resolutions.isEmpty() && mode != QPrinter::ScreenResolution) {
         if (resolutions.count() > 1 && mode == QPrinter::HighResolution) {
            int max = 0;
            for (int i = 0; i < resolutions.count(); ++i) {
               int value = resolutions.at(i).toInt();
               if (value > max) {
                  max = value;
               }
            }
            resolution.hRes = resolution.vRes = max;
         } else {
            resolution.hRes = resolution.vRes = resolutions.at(0).toInt();
         }
         if (resolution.hRes == 0) {
            resolution.hRes = resolution.vRes = 600;
         }
      } else {
         resolution.hRes = resolution.vRes = qt_defaultDpi();
      }
   }


   settings = static_cast<PMPrintSettings>([printInfo PMPrintSettings]);
   format = static_cast<PMPageFormat>([printInfo PMPageFormat]);

   QHash<QMacPrintEngine::PrintEnginePropertyKey, QVariant>::const_iterator propC;
   for (propC = valueCache.constBegin(); propC != valueCache.constEnd(); propC++) {
      q->setProperty(propC.key(), propC.value());
   }
}

void QMacPrintEnginePrivate::releaseSession()
{
   PMSessionEndPageNoDialog(session);
   PMSessionEndDocumentNoDialog(session);
   [printInfo release];

   if (hasCustomPaperSize) {
      PMRelease(customPaper);
   }
   printInfo = 0;
   session = 0;
}

bool QMacPrintEnginePrivate::newPage_helper()
{
   Q_Q(QMacPrintEngine);
   Q_ASSERT(state == QPrinter::Active);

   if (PMSessionError(session) != noErr) {
      q->abort();
      return false;
   }

   // pop the stack of saved graphic states, in case we get the same
   // context back - either way, the stack count should be 0 when we
   // get the new one
   QCoreGraphicsPaintEngine *cgEngine = static_cast<QCoreGraphicsPaintEngine *>(paintEngine);
   while (cgEngine->d_func()->stackCount > 0) {
      cgEngine->d_func()->restoreGraphicsState();
   }

   OSStatus status = PMSessionBeginPageNoDialog(session, format, 0);

   if (status != noErr) {
      state = QPrinter::Error;
      return false;
   }

   QRect page = q->property(QPrintEngine::PPK_PageRect).toRect();
   QRect paper = q->property(QPrintEngine::PPK_PaperRect).toRect();

   CGContextRef cgContext;
   OSStatus err = noErr;
   err = PMSessionGetCGGraphicsContext(session, &cgContext);
   if (err != noErr) {
      qWarning("QMacPrintEngine::newPage: Cannot retrieve CoreGraphics context: %ld", long(err));
      state = QPrinter::Error;
      return false;
   }
   cgEngine->d_func()->hd = cgContext;

   // Set the resolution as a scaling ration of 72 (the default).
   CGContextScaleCTM(cgContext, 72 / resolution.hRes, 72 / resolution.vRes);

   CGContextScaleCTM(cgContext, 1, -1);
   CGContextTranslateCTM(cgContext, 0, -paper.height());
   if (!fullPage) {
      CGContextTranslateCTM(cgContext, page.x() - paper.x(), page.y() - paper.y());
   }
   cgEngine->d_func()->orig_xform = CGContextGetCTM(cgContext);
   cgEngine->d_func()->setClip(0);
   cgEngine->state->dirtyFlags = QPaintEngine::DirtyFlag(QPaintEngine::AllDirty
                                 & ~(QPaintEngine::DirtyClipEnabled
                                     | QPaintEngine::DirtyClipRegion
                                     | QPaintEngine::DirtyClipPath));
   if (cgEngine->painter()->hasClipping()) {
      cgEngine->state->dirtyFlags |= QPaintEngine::DirtyClipEnabled;
   }
   cgEngine->syncState();
   return true;
}


void QMacPrintEngine::updateState(const QPaintEngineState &state)
{
   d_func()->paintEngine->updateState(state);
}

void QMacPrintEngine::drawRects(const QRectF *r, int num)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawRects(r, num);
}

void QMacPrintEngine::drawPoints(const QPointF *points, int pointCount)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawPoints(points, pointCount);
}

void QMacPrintEngine::drawEllipse(const QRectF &r)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawEllipse(r);
}

void QMacPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawLines(lines, lineCount);
}

void QMacPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawPolygon(points, pointCount, mode);
}

void QMacPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawPixmap(r, pm, sr);
}

void QMacPrintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawImage(r, pm, sr, flags);
}

void QMacPrintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawTextItem(p, ti);
}

void QMacPrintEngine::drawTiledPixmap(const QRectF &dr, const QPixmap &pixmap, const QPointF &sr)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawTiledPixmap(dr, pixmap, sr);
}

void QMacPrintEngine::drawPath(const QPainterPath &path)
{
   Q_D(QMacPrintEngine);
   Q_ASSERT(d->state == QPrinter::Active);
   d->paintEngine->drawPath(path);
}


void QMacPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
   Q_D(QMacPrintEngine);

   d->valueCache.insert(key, value);
   if (!d->session) {
      return;
   }

   switch (key) {
      case PPK_CollateCopies:
         break;
      case PPK_ColorMode:
         break;
      case PPK_Creator:
         break;
      case PPK_DocumentName:
         break;
      case PPK_PageOrder:
         break;
      case PPK_PaperSource:
         break;
      case PPK_SelectionOption:
         break;
      case PPK_Resolution:  {
         PMPrinter printer;
         UInt32 count;
         if (PMSessionGetCurrentPrinter(d->session, &printer) != noErr) {
            break;
         }
         if (PMPrinterGetPrinterResolutionCount(printer, &count) != noErr) {
            break;
         }
         PMResolution resolution = { 0.0, 0.0 };
         PMResolution bestResolution = { 0.0, 0.0 };
         int dpi = value.toInt();
         int bestDistance = INT_MAX;
         for (UInt32 i = 1; i <= count; ++i) {  // Yes, it starts at 1
            if (PMPrinterGetIndexedPrinterResolution(printer, i, &resolution) == noErr) {
               if (dpi == int(resolution.hRes)) {
                  bestResolution = resolution;
                  break;
               } else {
                  int distance = qAbs(dpi - int(resolution.hRes));
                  if (distance < bestDistance) {
                     bestDistance = distance;
                     bestResolution = resolution;
                  }
               }
            }
         }
         PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean);
         break;
      }

      case PPK_FullPage:
         d->fullPage = value.toBool();
         break;
      case PPK_CopyCount: // fallthrough
      case PPK_NumberOfCopies:
         PMSetCopies(d->settings, value.toInt(), false);
         break;
      case PPK_Orientation: {
         if (d->state == QPrinter::Active) {
            qWarning("QMacPrintEngine::setOrientation: Orientation cannot be changed during a print job, ignoring change");
         } else {
            QPrinter::Orientation newOrientation = QPrinter::Orientation(value.toInt());
            if (d->hasCustomPaperSize && (d->orient != newOrientation)) {
               d->customSize = QSizeF(d->customSize.height(), d->customSize.width());
            }
            d->orient = newOrientation;
            PMOrientation o = d->orient == QPrinter::Portrait ? kPMPortrait : kPMLandscape;
            PMSetOrientation(d->format, o, false);
            PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean);
         }
         break;
      }
      case PPK_OutputFileName:
         d->outputFilename = value.toString();
         break;
      case PPK_PaperSize:
         d->setPaperSize(QPrinter::PaperSize(value.toInt()));
         break;
      case PPK_PrinterName: {
         bool printerNameSet = false;
         OSStatus status = noErr;
         QCFType<CFArrayRef> printerList;
         status = PMServerCreatePrinterList(kPMServerLocal, &printerList);
         if (status == noErr) {
            CFIndex count = CFArrayGetCount(printerList);
            for (CFIndex i = 0; i < count; ++i) {
               PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(printerList, i)));
               QString name = QCFString::toQString(PMPrinterGetName(printer));
               if (name == value.toString()) {
                  status = PMSessionSetCurrentPMPrinter(d->session, printer);
                  printerNameSet = true;
                  break;
               }
            }
         }
         if (status != noErr) {
            qWarning("QMacPrintEngine::setPrinterName: Error setting printer: %ld", long(status));
         }
         if (!printerNameSet) {
            qWarning("QMacPrintEngine::setPrinterName: Failed to set printer named '%s'.", qPrintable(value.toString()));
            d->releaseSession();
            d->state = QPrinter::Idle;
         }
         break;
      }
      case PPK_SuppressSystemPrintStatus:
         d->suppressStatus = value.toBool();
         break;
      case PPK_CustomPaperSize: {
         PMOrientation orientation;
         PMGetOrientation(d->format, &orientation);
         d->customSize = value.toSizeF();
         if (orientation != kPMPortrait) {
            d->customSize = QSizeF(d->customSize.height(), d->customSize.width());
         }
         d->setPaperSize(QPrinter::Custom);
         break;
      }
      case PPK_PageMargins: {
         QList<QVariant> margins(value.toList());
         Q_ASSERT(margins.size() == 4);
         d->leftMargin = margins.at(0).toDouble();
         d->topMargin = margins.at(1).toDouble();
         d->rightMargin = margins.at(2).toDouble();
         d->bottomMargin = margins.at(3).toDouble();
         d->hasCustomPageMargins = true;
         break;
      }

      default:
         break;
   }
}

QVariant QMacPrintEngine::property(PrintEnginePropertyKey key) const
{
   Q_D(const QMacPrintEngine);
   QVariant ret;

   if (!d->session && d->valueCache.contains(key)) {
      return *d->valueCache.find(key);
   }

   switch (key) {
      case PPK_CollateCopies:
         ret = false;
         break;
      case PPK_ColorMode:
         ret = QPrinter::Color;
         break;
      case PPK_Creator:
         break;
      case PPK_DocumentName:
         break;
      case PPK_FullPage:
         ret = d->fullPage;
         break;
      case PPK_NumberOfCopies:
         ret = 1;
         break;
      case PPK_CopyCount: {
         UInt32 copies = 1;
         PMGetCopies(d->settings, &copies);
         ret = (uint) copies;
         break;
      }
      case PPK_SupportsMultipleCopies:
         ret = true;
         break;
      case PPK_Orientation:
         PMOrientation orientation;
         PMGetOrientation(d->format, &orientation);
         ret = orientation == kPMPortrait ? QPrinter::Portrait : QPrinter::Landscape;
         break;
      case PPK_OutputFileName:
         ret = d->outputFilename;
         break;
      case PPK_PageOrder:
         break;
      case PPK_PaperSource:
         break;
      case PPK_PageRect: {
         // PageRect is returned in device pixels
         QRect r;
         PMRect macrect, macpaper;
         qreal hRatio = d->resolution.hRes / 72;
         qreal vRatio = d->resolution.vRes / 72;
         if (d->hasCustomPaperSize) {
            r = QRect(0, 0, qRound(d->customSize.width() * hRatio), qRound(d->customSize.height() * vRatio));
            if (d->hasCustomPageMargins) {
               r.adjust(qRound(d->leftMargin * hRatio), qRound(d->topMargin * vRatio),
                        -qRound(d->rightMargin * hRatio), -qRound(d->bottomMargin * vRatio));
            } else {
               QList<QVariant> margins = property(QPrintEngine::PPK_PageMargins).toList();
               r.adjust(qRound(margins.at(0).toDouble() * hRatio),
                        qRound(margins.at(1).toDouble() * vRatio),
                        -qRound(margins.at(2).toDouble() * hRatio),
                        -qRound(margins.at(3).toDouble()) * vRatio);
            }
         } else if (PMGetAdjustedPageRect(d->format, &macrect) == noErr
                    && PMGetAdjustedPaperRect(d->format, &macpaper) == noErr) {
            if (d->fullPage || d->hasCustomPageMargins) {
               r.setCoords(int(macpaper.left * hRatio), int(macpaper.top * vRatio),
                           int(macpaper.right * hRatio), int(macpaper.bottom * vRatio));
               r.translate(-r.x(), -r.y());
               if (d->hasCustomPageMargins) {
                  r.adjust(qRound(d->leftMargin * hRatio), qRound(d->topMargin * vRatio),
                           -qRound(d->rightMargin * hRatio), -qRound(d->bottomMargin * vRatio));
               }
            } else {
               r.setCoords(int(macrect.left * hRatio), int(macrect.top * vRatio),
                           int(macrect.right * hRatio), int(macrect.bottom * vRatio));
               r.translate(int(-macpaper.left * hRatio), int(-macpaper.top * vRatio));
            }
         }
         ret = r;
         break;
      }
      case PPK_PaperSize:
         ret = d->paperSize();
         break;
      case PPK_PaperRect: {
         QRect r;
         PMRect macrect;
         qreal hRatio = d->resolution.hRes / 72;
         qreal vRatio = d->resolution.vRes / 72;
         if (d->hasCustomPaperSize) {
            r = QRect(0, 0, qRound(d->customSize.width() * hRatio), qRound(d->customSize.height() * vRatio));
         } else if (PMGetAdjustedPaperRect(d->format, &macrect) == noErr) {
            r.setCoords(int(macrect.left * hRatio), int(macrect.top * vRatio),
                        int(macrect.right * hRatio), int(macrect.bottom * vRatio));
            r.translate(-r.x(), -r.y());
         }
         ret = r;
         break;
      }
      case PPK_PrinterName: {
         PMPrinter printer;
         OSStatus status = PMSessionGetCurrentPrinter(d->session, &printer);
         if (status != noErr) {
            qWarning("QMacPrintEngine::printerName: Failed getting current PMPrinter: %ld", long(status));
         }
         if (printer) {
            ret = QCFString::toQString(PMPrinterGetName(printer));
         }
         break;
      }
      case PPK_Resolution: {
         ret = d->resolution.hRes;
         break;
      }
      case PPK_SupportedResolutions:
         ret = d->supportedResolutions();
         break;
      case PPK_CustomPaperSize:
         ret = d->customSize;
         break;
      case PPK_PageMargins: {
         QList<QVariant> margins;
         if (d->hasCustomPageMargins) {
            margins << d->leftMargin << d->topMargin
                    << d->rightMargin << d->bottomMargin;
         } else {
            PMPaperMargins paperMargins;
            PMPaper paper;
            PMGetPageFormatPaper(d->format, &paper);
            PMPaperGetMargins(paper, &paperMargins);
            margins << paperMargins.left << paperMargins.top
                    << paperMargins.right << paperMargins.bottom;
         }
         ret = margins;
         break;
      }
      default:
         break;
   }
   return ret;
}

QT_END_NAMESPACE

#endif // QT_NO_PRINTER
