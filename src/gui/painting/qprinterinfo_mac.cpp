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

#include <qprinterinfo.h>
#include <qprinterinfo_p.h>
#include <qt_mac_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

extern QPrinter::PaperSize qSizeFTopaperSize(const QSizeF &size);

QList<QPrinterInfo> QPrinterInfo::availablePrinters()
{
   QList<QPrinterInfo> printers;

   QCFType<CFArrayRef> array;
   if (PMServerCreatePrinterList(kPMServerLocal, &array) == noErr) {
      CFIndex count = CFArrayGetCount(array);
      for (int i = 0; i < count; ++i) {
         PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(array, i)));
         QString printerName = QCFString::toQString(PMPrinterGetName(printer));

         QPrinterInfo printerInfo(printerName);
         if (PMPrinterIsDefault(printer)) {
            printerInfo.d_ptr->isDefault = true;
         }
         printers.append(printerInfo);
      }
   }

   return printers;
}

QPrinterInfo QPrinterInfo::defaultPrinter()
{
   QList<QPrinterInfo> printers = availablePrinters();
   for (const QPrinterInfo & printerInfo : printers) {
      if (printerInfo.isDefault()) {
         return printerInfo;
      }
   }

   return printers.value(0);
}

QList<QPrinter::PaperSize> QPrinterInfo::supportedPaperSizes() const
{
   const Q_D(QPrinterInfo);

   QList<QPrinter::PaperSize> paperSizes;
   if (isNull()) {
      return paperSizes;
   }

   PMPrinter cfPrn = PMPrinterCreateFromPrinterID(QCFString::toCFStringRef(d->name));
   if (!cfPrn) {
      return paperSizes;
   }

   CFArrayRef array;
   if (PMPrinterGetPaperList(cfPrn, &array) != noErr) {
      PMRelease(cfPrn);
      return paperSizes;
   }

   int count = CFArrayGetCount(array);
   for (int i = 0; i < count; ++i) {
      PMPaper paper = static_cast<PMPaper>(const_cast<void *>(CFArrayGetValueAtIndex(array, i)));
      double width, height;
      if (PMPaperGetWidth(paper, &width) == noErr && PMPaperGetHeight(paper, &height) == noErr) {
         QSizeF size(width * 0.3527, height * 0.3527);
         paperSizes.append(qSizeFTopaperSize(size));
      }
   }

   PMRelease(cfPrn);

   return paperSizes;
}

#endif // QT_NO_PRINTER

QT_END_NAMESPACE
