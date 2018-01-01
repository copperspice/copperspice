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

#ifndef QT_NO_PRINTDIALOG

#include <qt_mac_p.h>
#include <qhash.h>
#include <qprintdialog.h>
#include <qapplication_p.h>
#include <qabstractprintdialog_p.h>
#include <qprintengine_mac_p.h>

QT_BEGIN_NAMESPACE

class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
   Q_DECLARE_PUBLIC(QPrintDialog)

 public:
   QPrintDialogPrivate() : ep(0), printPanel(0) {
   }

   void openCocoaPrintPanel(Qt::WindowModality modality);
   void closeCocoaPrintPanel();
   void initBeforeRun();

   inline QPrintDialog *printDialog() {
      return q_func();
   }

   inline void _q_printToFileChanged(int) {}
   inline void _q_rbPrintRangeToggled(bool) {}
   inline void _q_printerChanged(int) {}

#ifndef QT_NO_MESSAGEBOX
   inline void _q_checkFields() {}
#endif

   inline void _q_chbPrintLastFirstToggled(bool) {}
   inline void _q_paperSizeChanged(int) {}
   inline void _q_btnBrowseClicked() {}
   inline void _q_btnPropertiesClicked() {}

   QMacPrintEnginePrivate *ep;
   NSPrintPanel *printPanel;
};

QT_END_NAMESPACE

QT_USE_NAMESPACE


@class QT_MANGLE_NAMESPACE(QCocoaPrintPanelDelegate);

@interface QT_MANGLE_NAMESPACE(QCocoaPrintPanelDelegate) : NSObject
{
}
- (void)printPanelDidEnd: (NSPrintPanel *)printPanel
              returnCode: (int)returnCode contextInfo: (void *)contextInfo;
@end

@implementation QT_MANGLE_NAMESPACE(QCocoaPrintPanelDelegate)
- (void)printPanelDidEnd: (NSPrintPanel *)printPanel
              returnCode: (int)returnCode contextInfo: (void *)contextInfo
{
   Q_UNUSED(printPanel);

   QPrintDialogPrivate *d = static_cast<QPrintDialogPrivate *>(contextInfo);
   QPrintDialog *dialog = d->printDialog();

   if (returnCode == NSOKButton) {
      UInt32 frompage, topage;
      PMGetFirstPage(d->ep->settings, &frompage);
      PMGetLastPage(d->ep->settings, &topage);

      topage = qMin(UInt32(INT_MAX), topage);
      dialog->setFromTo(frompage, topage);


      if (dialog->fromPage() == 1 && dialog->toPage() == INT_MAX) {
         // print all

         dialog->setPrintRange(QPrintDialog::AllPages);
         dialog->setFromTo(0, 0);

      } else {
         dialog->setPrintRange(QPrintDialog::PageRange);

         // Carbon hands us back a very large number, set 'toPage' to max
         // to follow the behavior of the other print dialogs

         if (dialog->maxPage() < dialog->toPage()) {
            dialog->setFromTo(dialog->fromPage(), dialog->maxPage());
         }
      }

      // keep in sync with file output
      PMDestinationType dest;

      // If the user selected print to file, the session has been
      // changed behind our back and our d->ep->session object is a
      // dangling pointer. Update it based on the "current" session
      d->ep->session = static_cast<PMPrintSession>([d->ep->printInfo PMPrintSession]);

      PMSessionGetDestinationType(d->ep->session, d->ep->settings, &dest);
      if (dest == kPMDestinationFile) {
         QCFType<CFURLRef> file;
         PMSessionCopyDestinationLocation(d->ep->session, d->ep->settings, &file);

         // assume POSIX file system here
         UInt8 localFile[2048];

         CFURLGetFileSystemRepresentation(file, true, localFile, sizeof(localFile));
         d->ep->outputFilename = QString::fromUtf8(reinterpret_cast<const char *>(localFile));

      } else {
         // Keep output format
         QPrinter::OutputFormat format;
         format = d->printer->outputFormat();
         d->printer->setOutputFileName(QString());
         d->printer->setOutputFormat(format);
      }
   }

   dialog->done((returnCode == NSOKButton) ? QDialog::Accepted : QDialog::Rejected);
}
@end


QT_BEGIN_NAMESPACE

extern void macStartInterceptWindowTitle(QWidget *window);
extern void macStopInterceptWindowTitle();


void QPrintDialogPrivate::initBeforeRun()
{
   Q_Q(QPrintDialog);
   // If someone is reusing a QPrinter object, the end released all our old
   // information. In this case, we must reinitialize.
   if (ep->session == 0) {
      ep->initialize();
   }


   // It seems the only way that PM lets you use all is if the minimum
   // for the page range is 1. This _kind of_ makes sense if you think about
   // it. However, calling PMSetFirstPage() or PMSetLastPage() always enforces
   // the range.
   PMSetPageRange(ep->settings, q->minPage(), q->maxPage());
   if (q->printRange() == QAbstractPrintDialog::PageRange) {
      PMSetFirstPage(ep->settings, q->fromPage(), false);
      PMSetLastPage(ep->settings, q->toPage(), false);
   }
}

void QPrintDialogPrivate::openCocoaPrintPanel(Qt::WindowModality modality)
{
   Q_Q(QPrintDialog);

   initBeforeRun();

   QPrintDialog::PrintDialogOptions qtOptions = q->options();
   NSPrintPanelOptions macOptions = NSPrintPanelShowsCopies;
   if (qtOptions & QPrintDialog::PrintPageRange) {
      macOptions |= NSPrintPanelShowsPageRange;
   }
   if (qtOptions & QPrintDialog::PrintShowPageSize)
      macOptions |= NSPrintPanelShowsPaperSize | NSPrintPanelShowsPageSetupAccessory
                    | NSPrintPanelShowsOrientation;

   macStartInterceptWindowTitle(q);
   printPanel = [NSPrintPanel printPanel];
   QT_MANGLE_NAMESPACE(QCocoaPrintPanelDelegate) *delegate = [[QT_MANGLE_NAMESPACE(QCocoaPrintPanelDelegate) alloc] init];

   // Call processEvents in case the event dispatcher has been interrupted, and needs to do
   // cleanup of modal sessions. Do this before showing the native dialog, otherwise it will
   // close down during the cleanup (QTBUG-17913):
   qApp->processEvents(QEventLoop::ExcludeUserInputEvents, QEventLoop::ExcludeSocketNotifiers);

   [printPanel setOptions: macOptions];

   if (modality == Qt::ApplicationModal || !q->parentWidget()) {
      if (modality == Qt::NonModal) {
         qWarning("QPrintDialog is required to be modal on OS X");
      }
      int rval = [printPanel runModalWithPrintInfo: ep->printInfo];
      [delegate printPanelDidEnd: printPanel returnCode: rval contextInfo: this];
   } else {
      Q_ASSERT(q->parentWidget());
      NSWindow *windowRef = qt_mac_window_for(q->parentWidget());
      [printPanel beginSheetWithPrintInfo: ep->printInfo
       modalForWindow: windowRef
       delegate: delegate
       didEndSelector: @selector(printPanelDidEnd: returnCode: contextInfo:)
       contextInfo: this];
   }

   macStopInterceptWindowTitle();
}

void QPrintDialogPrivate::closeCocoaPrintPanel()
{
   // ###
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
   d->ep = static_cast<QMacPrintEngine *>(d->printer->paintEngine())->d_func();
}

QPrintDialog::QPrintDialog(QWidget *parent)
   : QAbstractPrintDialog(*(new QPrintDialogPrivate), 0, parent)
{
   Q_D(QPrintDialog);
   if (!warnIfNotNative(d->printer)) {
      return;
   }
   d->ep = static_cast<QMacPrintEngine *>(d->printer->paintEngine())->d_func();
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

   QMacCocoaAutoReleasePool pool;

   d->openCocoaPrintPanel(Qt::ApplicationModal);
   d->closeCocoaPrintPanel();

   return result();
}

/*!
    \reimp
*/
void QPrintDialog::setVisible(bool visible)
{
   Q_D(QPrintDialog);

   bool isCurrentlyVisible = (d->printPanel != 0);

   if (!visible == !isCurrentlyVisible) {
      return;
   }

   if (d->printer->outputFormat() != QPrinter::NativeFormat) {
      return;
   }

   if (visible) {
      d->openCocoaPrintPanel(parentWidget() ? Qt::WindowModal : Qt::ApplicationModal);
      return;

   } else {
      if (d->printPanel) {
         d->closeCocoaPrintPanel();
         return;
      }
   }
}

// wrappers, duplicated code in _win, _unix, _qws, _mac
void QPrintDialog::_q_chbPrintLastFirstToggled(bool un_named_arg1)
{
   Q_D(QPrintDialog);
   d->_q_chbPrintLastFirstToggled(un_named_arg1);
}

#if defined (Q_OS_UNIX) && !defined (Q_OS_MAC)
void QPrintDialog::_q_collapseOrExpandDialog()
{
   Q_D(QPrintDialog);
   d->_q_collapseOrExpandDialog();
}
#endif


#if defined(Q_OS_UNIX) && !defined (Q_OS_MAC) && !defined(QT_NO_MESSAGEBOX)
void QPrintDialog::_q_checkFields()
{
   Q_D(QPrintDialog);
   d->_q_checkFields();
}
#endif

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
