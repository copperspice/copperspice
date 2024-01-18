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

#include <qpagesetupdialog.h>

#ifndef QT_NO_PRINTDIALOG

#include <qpagesetupdialog_p.h>

#include <qplatform_nativeinterface.h>
#include <qprintengine.h>

@class QCocoaPageLayoutDelegate;

@interface QCocoaPageLayoutDelegate : NSObject
{
    NSPrintInfo *printInfo;
}
- (id)initWithNSPrintInfo:(NSPrintInfo *)nsPrintInfo;
- (void)pageLayoutDidEnd: (NSPageLayout *)pageLayout
              returnCode: (int)returnCode contextInfo: (void *)contextInfo;
@end

@implementation QCocoaPageLayoutDelegate
- (id)initWithNSPrintInfo:(NSPrintInfo *)nsPrintInfo
{
   self = [super init];
   if (self) {
        printInfo = nsPrintInfo;
   }
   return self;

}
- (void)pageLayoutDidEnd: (NSPageLayout *)pageLayout
              returnCode: (int)returnCode contextInfo: (void *)contextInfo
{
   QPageSetupDialog *dialog = static_cast<QPageSetupDialog *>(contextInfo);
   QPrinter *printer = dialog->printer();

   if (returnCode == NSModalResponseOK) {
        PMPageFormat format = static_cast<PMPageFormat>([printInfo PMPageFormat]);
        PMRect paperRect;
        PMGetUnadjustedPaperRect(format, &paperRect);
        PMOrientation orientation;
        PMGetOrientation(format, &orientation);

        QSizeF paperSize = QSizeF(paperRect.right - paperRect.left, paperRect.bottom - paperRect.top);
        printer->printEngine()->setProperty(QPrintEngine::PPK_CustomPaperSize, paperSize);
        printer->printEngine()->setProperty(QPrintEngine::PPK_Orientation, orientation == kPMLandscape ?
                  QPageLayout::Orientation::Landscape : QPageLayout::Orientation::Portrait);
   }

   dialog->done((returnCode == NSModalResponseOK) ? QDialog::Accepted : QDialog::Rejected);
}
@end



class QMacPageSetupDialogPrivate : public QPageSetupDialogPrivate
{
   Q_DECLARE_PUBLIC(QPageSetupDialog)

 public:
    QMacPageSetupDialogPrivate(QPrinter *printer)
        :  QPageSetupDialogPrivate(printer), printInfo(nullptr), pageLayout(nullptr)
   {
   }

    ~QMacPageSetupDialogPrivate()
   {
   }

   void openCocoaPageLayout(Qt::WindowModality modality);
   void closeCocoaPageLayout();

   NSPrintInfo *printInfo;
   NSPageLayout *pageLayout;

};

void QMacPageSetupDialogPrivate::openCocoaPageLayout(Qt::WindowModality modality)
{
   Q_Q(QPageSetupDialog);

   // If someone is reusing a QPrinter object, the end released all our old

   void *voidp = nullptr;

   (void) QMetaObject::invokeMethod(qApp->platformNativeInterface(), "NSPrintInfoForPrintEngine",
                  Q_RETURN_ARG(void *, voidp), Q_ARG(QPrintEngine *, printer->printEngine()));

    printInfo = static_cast<NSPrintInfo *>(voidp);
    [printInfo retain];

   pageLayout = [NSPageLayout pageLayout];

   // Keep a copy to this since we plan on using it for a bit.
   [pageLayout retain];

    QCocoaPageLayoutDelegate *delegate = [[QCocoaPageLayoutDelegate alloc] initWithNSPrintInfo:printInfo];

    if (modality == Qt::ApplicationModal) {

        // Make sure we don't interrupt the runModalWithPrintInfo call.
        (void) QMetaObject::invokeMethod(qApp->platformNativeInterface(),
                                         "clearCurrentThreadCocoaEventDispatcherInterruptFlag");

        int rval = [pageLayout runModalWithPrintInfo:printInfo];
        [delegate pageLayoutDidEnd:pageLayout returnCode:rval contextInfo:q];
    } else {
        Q_ASSERT(q->parentWidget());
        QWindow *parentWindow = q->parentWidget()->windowHandle();
        NSWindow *window = static_cast<NSWindow *>(qApp->platformNativeInterface()->nativeResourceForWindow("nswindow", parentWindow));
        [pageLayout beginSheetWithPrintInfo:printInfo
                             modalForWindow:window
                                   delegate:delegate
                             didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:)
                                contextInfo:q];
    }
}

void QMacPageSetupDialogPrivate::closeCocoaPageLayout()
{
    [printInfo release];
    printInfo  = nullptr;
    [pageLayout release];
    pageLayout = nullptr;
}

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QMacPageSetupDialogPrivate(printer)), parent)
{
    setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
    setAttribute(Qt::WA_DontShowOnScreen);
}

QPageSetupDialog::QPageSetupDialog(QWidget *parent)
    : QDialog(*(new QMacPageSetupDialogPrivate(nullptr)), parent)
{
    setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
    setAttribute(Qt::WA_DontShowOnScreen);
}

void QPageSetupDialog::setVisible(bool visible)
{
   Q_D(QPageSetupDialog);

    if (d->printer->outputFormat() != QPrinter::NativeFormat) {
      return;
   }

    bool isCurrentlyVisible = (static_cast <QMacPageSetupDialogPrivate*>(d)->pageLayout != nullptr);

   if (!visible == !isCurrentlyVisible) {
      return;
   }

   QDialog::setVisible(visible);

   if (visible) {
        Qt::WindowModality modality = windowModality();
        if (modality == Qt::NonModal) {
            // NSPrintPanels can only be modal, so we must pick a type
            modality = parentWidget() ? Qt::WindowModal : Qt::ApplicationModal;
        }

        static_cast <QMacPageSetupDialogPrivate*>(d)->openCocoaPageLayout(modality);
        return;

    } else {
        if (static_cast <QMacPageSetupDialogPrivate*>(d)->pageLayout) {
            static_cast <QMacPageSetupDialogPrivate*>(d)->closeCocoaPageLayout();
            return;
        }
    }
}

int QPageSetupDialog::exec()
{
   Q_D(QPageSetupDialog);

   if (d->printer->outputFormat() != QPrinter::NativeFormat) {
      return Rejected;
   }
    QDialog::setVisible(true);

    QMacAutoReleasePool pool;
    static_cast <QMacPageSetupDialogPrivate*>(d)->openCocoaPageLayout(Qt::ApplicationModal);
    static_cast <QMacPageSetupDialogPrivate*>(d)->closeCocoaPageLayout();

    QDialog::setVisible(false);

   return result();
}


#endif /* QT_NO_PRINTDIALOG */
