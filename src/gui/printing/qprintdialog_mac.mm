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

#include <Cocoa/Cocoa.h>

#include <qplatform_printdevice.h>
#include <qprintdialog.h>
#include <qprintengine.h>
#include <qprinter.h>

#include <qabstractprintdialog_p.h>
#include <qapplication_p.h>
#include <qcore_mac_p.h>

#ifndef QT_NO_PRINTDIALOG

extern qreal qt_pointMultiplier(QPageLayout::Unit unit);
class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
   Q_DECLARE_PUBLIC(QPrintDialog)

 public:
   QPrintDialogPrivate()
      : printInfo(nullptr), printPanel(nullptr)
   {
   }

   void openCocoaPrintPanel(Qt::WindowModality modality);
   void closeCocoaPrintPanel();

   inline QPrintDialog *printDialog() { return q_func(); }

   NSPrintInfo *printInfo;
   NSPrintPanel *printPanel;
};


@class QCocoaPrintPanelDelegate;

@interface QCocoaPrintPanelDelegate : NSObject
{
    NSPrintInfo *printInfo;
}
- (id)initWithNSPrintInfo:(NSPrintInfo *)nsPrintInfo;
- (void)printPanelDidEnd: (NSPrintPanel *)printPanel
              returnCode: (int)returnCode contextInfo: (void *)contextInfo;
@end

@implementation QCocoaPrintPanelDelegate
- (id)initWithNSPrintInfo:(NSPrintInfo *)nsPrintInfo
{
    if (self = [super init]) {
        printInfo = nsPrintInfo;
    }
    return self;
}
- (void)printPanelDidEnd: (NSPrintPanel *)printPanel
              returnCode: (int)returnCode contextInfo: (void *)contextInfo
{
    (void) printPanel;

    QPrintDialog *dialog = static_cast<QPrintDialog *>(contextInfo);
    QPrinter *printer = dialog->printer();

   if (returnCode == NSModalResponseOK) {
        PMPrintSession session = static_cast<PMPrintSession>([printInfo PMPrintSession]);
        PMPrintSettings settings = static_cast<PMPrintSettings>([printInfo PMPrintSettings]);
      UInt32 frompage, topage;
        PMGetFirstPage(settings, &frompage);
        PMGetLastPage(settings, &topage);

      topage = qMin(UInt32(INT_MAX), topage);
      dialog->setFromTo(frompage, topage);


      if (dialog->fromPage() == 1 && dialog->toPage() == INT_MAX) {


         dialog->setPrintRange(QPrintDialog::AllPages);
         dialog->setFromTo(0, 0);

      } else {
         dialog->setPrintRange(QPrintDialog::PageRange);

         if (dialog->maxPage() < dialog->toPage()) {
            dialog->setFromTo(dialog->fromPage(), dialog->maxPage());
         }
      }

      // Keep us in sync with chosen destination
      PMDestinationType dest;


      PMSessionGetDestinationType(session, settings, &dest);
      if (dest == kPMDestinationFile) {
      	// QTBUG-38820

      } else {
            PMPrinter macPrinter;
            PMSessionGetCurrentPrinter(session, &macPrinter);
            QString printerId = QString::fromCFString(PMPrinterGetID(macPrinter));
            if (printer->printerName() != printerId)
                printer->setPrinterName(printerId);
        }
    }

    // Note this code should be in QCocoaPrintDevice, but that implementation is in the plugin
    PMPageFormat pageFormat = static_cast<PMPageFormat>([printInfo PMPageFormat]);
    PMPaper paper;
    PMGetPageFormatPaper(pageFormat, &paper);
    PMOrientation orientation;
    PMGetOrientation(pageFormat, &orientation);
    QPageSize pageSize;
    CFStringRef key;
    double width = 0;
    double height = 0;

    // If the PPD name is empty then is custom, for some reason PMPaperIsCustom doesn't work here
    PMPaperGetPPDPaperName(paper, &key);

    if (PMPaperGetWidth(paper, &width) == noErr && PMPaperGetHeight(paper, &height) == noErr) {
        QString ppdKey = QString::fromCFString(key);

        if (ppdKey.isEmpty()) {
            // Is using a custom page size as defined in the Print Dialog custom settings using mm or inches.
            // We can't ask PMPaper what those units actually are, we can only get the point size which may return
            // slightly wrong results due to rounding.
            // Testing shows if using metric/mm then is rounded mm, if imperial/inch is rounded to 2 decimal places
            // Even if we pass in our own custom size in mm with decimal places, the dialog will still round it!
            // Suspect internal storage is in rounded mm?

            if (QLocale().measurementSystem() == QLocale::MetricSystem) {
                QSizeF sizef = QSizeF(width, height) / qt_pointMultiplier(QPageSize::Unit::Millimeter);
                // Round to 0 decimal places
                pageSize = QPageSize(sizef.toSize(), QPageSize::Unit::Millimeter);

            } else {
                qreal multiplier = qt_pointMultiplier(QPageSize::Unit::Inch);
                const int w = qRound(width * 100 / multiplier);
                const int h = qRound(height * 100 / multiplier);
                pageSize = QPageSize(QSizeF(w / 100.0, h / 100.0), QPageSize::Unit::Inch);
            }
        } else {
            pageSize = QPlatformPrintDevice::createPageSize(ppdKey, QSize(width, height), QString());
        }
    }

    if (pageSize.isValid() && !pageSize.isEquivalentTo(printer->pageLayout().pageSize())) {
        printer->setPageSize(pageSize);
    }

    printer->setOrientation(orientation == kPMLandscape ? QPageLayout::Landscape : QPageLayout::Portrait);

   dialog->done((returnCode == NSModalResponseOK) ? QDialog::Accepted : QDialog::Rejected);
}
@end

void QPrintDialogPrivate::openCocoaPrintPanel(Qt::WindowModality modality)
{
   Q_Q(QPrintDialog);

   // get the NSPrintInfo from the print engine in the platform plugin
   void *voidp = nullptr;

   (void) QMetaObject::invokeMethod(qApp->platformNativeInterface(), "NSPrintInfoForPrintEngine",
                  Q_RETURN_ARG(void *, voidp), Q_ARG(QPrintEngine *, printer->printEngine()));

   printInfo = static_cast<NSPrintInfo *>(voidp);
   [printInfo retain];

   // It seems the only way that PM lets you use all is if the minimum
   // for the page range is 1. This _kind of_ makes sense if you think about
   // it. However, calling PMSetFirstPage() or PMSetLastPage() always enforces the range.
   // get print settings from the platform plugin

   PMPrintSettings settings = static_cast<PMPrintSettings>([printInfo PMPrintSettings]);
   PMSetPageRange(settings, q->minPage(), q->maxPage());

   if (q->printRange() == QAbstractPrintDialog::PageRange) {
      PMSetFirstPage(settings, q->fromPage(), false);
      PMSetLastPage(settings, q->toPage(), false);
   }

   [printInfo updateFromPMPrintSettings];

   QPrintDialog::PrintDialogOptions qtOptions = q->options();
   NSPrintPanelOptions macOptions = NSPrintPanelShowsCopies;
   if (qtOptions & QPrintDialog::PrintPageRange) {
      macOptions |= NSPrintPanelShowsPageRange;
   }

   if (qtOptions & QPrintDialog::PrintShowPageSize)
      macOptions |= NSPrintPanelShowsPaperSize | NSPrintPanelShowsPageSetupAccessory | NSPrintPanelShowsOrientation;

    printPanel = [NSPrintPanel printPanel];
    [printPanel retain];
    [printPanel setOptions:macOptions];

   // Call processEvents in case the event dispatcher has been interrupted, and needs to do
   // cleanup of modal sessions. Do this before showing the native dialog, otherwise it will
   // close down during the cleanup (QTBUG-17913):
   qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

    QCocoaPrintPanelDelegate *delegate = [[QCocoaPrintPanelDelegate alloc] initWithNSPrintInfo:printInfo];

    if (modality == Qt::ApplicationModal || !q->parentWidget()) {
        if (modality == Qt::NonModal)
            qWarning("QPrintDialog is required to be modal on OS X");

        // Make sure we don't interrupt the runModalWithPrintInfo call.
        (void) QMetaObject::invokeMethod(qApp->platformNativeInterface(),
                                         "clearCurrentThreadCocoaEventDispatcherInterruptFlag");

        int rval = [printPanel runModalWithPrintInfo:printInfo];
        [delegate printPanelDidEnd:printPanel returnCode:rval contextInfo:q];
    } else {
        Q_ASSERT(q->parentWidget());
        QWindow *parentWindow = q->parentWidget()->windowHandle();
        NSWindow *window = static_cast<NSWindow *>(qApp->platformNativeInterface()->nativeResourceForWindow("nswindow", parentWindow));
        [printPanel beginSheetWithPrintInfo:printInfo
                             modalForWindow:window
                                   delegate:delegate
                             didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:)
                                contextInfo:q];
    }
}

void QPrintDialogPrivate::closeCocoaPrintPanel()
{
    [printInfo release];
    printInfo  = nullptr;
    [printPanel release];
    printPanel = nullptr;
}

static bool warnIfNotNative(QPrinter *printer)
{
   if (printer->outputFormat() != QPrinter::NativeFormat) {
      qWarning("QPrintDialog: Cannot be used on non-native printers");
      return false;
   }
   return true;
}

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
   : QAbstractPrintDialog(*(new QPrintDialogPrivate), printer, parent)
{
   Q_D(QPrintDialog);
   if (!warnIfNotNative(d->printer)) {
      return;
      }
    setAttribute(Qt::WA_DontShowOnScreen);
}

QPrintDialog::QPrintDialog(QWidget *parent)
   : QAbstractPrintDialog(*(new QPrintDialogPrivate), nullptr, parent)
{
   Q_D(QPrintDialog);
   if (!warnIfNotNative(d->printer)) {
      return;
    }

    setAttribute(Qt::WA_DontShowOnScreen);
}

QPrintDialog::~QPrintDialog()
{
}

int QPrintDialog::exec()
{
   Q_D(QPrintDialog);
   if (!warnIfNotNative(d->printer)) {
      return QDialog::Rejected;
   }
    QDialog::setVisible(true);

    QMacAutoReleasePool pool;

   d->openCocoaPrintPanel(Qt::ApplicationModal);
   d->closeCocoaPrintPanel();

    QDialog::setVisible(false);
   return result();
}

void QPrintDialog::setVisible(bool visible)
{
   Q_D(QPrintDialog);

   bool isCurrentlyVisible = (d->printPanel != nullptr);

   if (!visible == !isCurrentlyVisible) {
      return;
   }

   if (d->printer->outputFormat() != QPrinter::NativeFormat) {
      return;
   }
    QDialog::setVisible(visible);

   if (visible) {
        Qt::WindowModality modality = windowModality();
        if (modality == Qt::NonModal) {
            // NSPrintPanels can only be modal, so we must pick a type
            modality = parentWidget() ? Qt::WindowModal : Qt::ApplicationModal;
        }
        d->openCocoaPrintPanel(modality);
      return;

   } else {
      if (d->printPanel) {
         d->closeCocoaPrintPanel();
         return;
      }
   }
}

// wrappers, duplicated code in _win, _unix, _mac
#if defined (Q_OS_UNIX) && ! defined (Q_OS_DARWIN)
void QPrintDialog::_q_collapseOrExpandDialog()
{
   Q_D(QPrintDialog);
   d->_q_collapseOrExpandDialog();
}
#endif


#if defined(Q_OS_UNIX) && !defined (Q_OS_DARWIN) && !defined(QT_NO_MESSAGEBOX)
void QPrintDialog::_q_checkFields()
{
   Q_D(QPrintDialog);
   d->_q_checkFields();
}
#endif



#endif // QT_NO_PRINTDIALOG
