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

#include <qpagesetupdialog.h>

#ifndef QT_NO_PRINTDIALOG

#include <qapplication.h>
#include <qprinter.h>
#include <qplatform_nativeinterface.h>

#include <qprintengine_win_p.h>
#include <qpagesetupdialog_p.h>

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
   : QDialog(*(new QPageSetupDialogPrivate(printer)), parent)
{
   setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
   setAttribute(Qt::WA_DontShowOnScreen);
}

QPageSetupDialog::QPageSetupDialog(QWidget *parent)
   : QDialog(*(new QPageSetupDialogPrivate(nullptr)), parent)
{
   setWindowTitle(QCoreApplication::translate("QPrintPreviewDialog", "Page Setup"));
   setAttribute(Qt::WA_DontShowOnScreen);
}

int QPageSetupDialog::exec()
{
   Q_D(QPageSetupDialog);

   if (d->printer->outputFormat() != QPrinter::NativeFormat) {
      return Rejected;
   }

   QWin32PrintEngine *engine = static_cast<QWin32PrintEngine *>(d->printer->paintEngine());
   QWin32PrintEnginePrivate *ep = static_cast<QWin32PrintEnginePrivate *>(engine->d_ptr.data());

   PAGESETUPDLG psd;
   memset(&psd, 0, sizeof(PAGESETUPDLG));
   psd.lStructSize = sizeof(PAGESETUPDLG);

   // we need a temp DEVMODE struct if we don't have a global DEVMODE
   HGLOBAL hDevMode = nullptr;
   int devModeSize  = 0;

   if (!engine->globalDevMode()) {
      devModeSize = sizeof(DEVMODE) + ep->devMode->dmDriverExtra;
      hDevMode = GlobalAlloc(GHND, devModeSize);
      if (hDevMode) {
         void *dest = GlobalLock(hDevMode);
         memcpy(dest, ep->devMode, devModeSize);
         GlobalUnlock(hDevMode);
      }
      psd.hDevMode = hDevMode;
   } else {
      psd.hDevMode = engine->globalDevMode();
   }

   HGLOBAL *tempDevNames = engine->createGlobalDevNames();
   psd.hDevNames = tempDevNames;

   QWidget *parent = parentWidget();
   parent = parent ? parent->window() : QApplication::activeWindow();
   Q_ASSERT(!parent || parent->testAttribute(Qt::WA_WState_Created));

   QWindow *parentWindow = parent ? parent->windowHandle() : nullptr;
   psd.hwndOwner = parentWindow ? (HWND)QGuiApplication::platformNativeInterface()->nativeResourceForWindow("handle", parentWindow) : nullptr;

   psd.Flags = PSD_MARGINS;
   QPageLayout layout = d->printer->pageLayout();

   switch (layout.units()) {
      case QPageSize::Unit::Millimeter:
      case QPageSize::Unit::Inch:
         break;

      case QPageSize::Unit::Point:
      case QPageSize::Unit::Pica:
      case QPageSize::Unit::Didot:
      case QPageSize::Unit::Cicero:
         layout.setUnits(QLocale::system().measurementSystem() == QLocale::MetricSystem ?
                  QPageSize::Unit::Millimeter : QPageSize::Unit::Inch);
         break;

      case QPageSize::Unit::DevicePixel:
         break;
   }

   qreal multiplier = 1.0;
   if (layout.units() == QPageSize::Unit::Millimeter) {
      psd.Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
      multiplier = 100.0;

   } else {
      // QPageSize::Unit::Inch)

      psd.Flags |= PSD_INTHOUSANDTHSOFINCHES;
      multiplier = 1000.0;
   }


   psd.rtMargin.left   = layout.margins().left() * multiplier;
   psd.rtMargin.top    = layout.margins().top() * multiplier;
   psd.rtMargin.right  = layout.margins().right() * multiplier;
   psd.rtMargin.bottom = layout.margins().bottom() * multiplier;

   QDialog::setVisible(true);
   bool result = PageSetupDlg(&psd);
   QDialog::setVisible(false);

   if (result) {
      engine->setGlobalDevMode(psd.hDevNames, psd.hDevMode);
      const QMarginsF margins(psd.rtMargin.left, psd.rtMargin.top, psd.rtMargin.right, psd.rtMargin.bottom);

      d->printer->setPageMargins(margins / multiplier, layout.units());


      if (!engine->globalDevMode() && hDevMode) {
         // copy from our temp DEVMODE struct
         if (ep->ownsDevMode && ep->devMode) {
            free(ep->devMode);
         }
         ep->devMode = (DEVMODE *) malloc(devModeSize);
         ep->ownsDevMode = true;
         void *src = GlobalLock(hDevMode);
         memcpy(ep->devMode, src, devModeSize);
         GlobalUnlock(hDevMode);
      }
   }

   if (!engine->globalDevMode() && hDevMode) {
      GlobalFree(hDevMode);
   }

   GlobalFree(tempDevNames);
   done(result);
   return result;
}

void QPageSetupDialog::setVisible(bool visible)
{
   if (!visible) {
      return;
   }
   exec();
}

#endif
