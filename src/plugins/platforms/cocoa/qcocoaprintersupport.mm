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

#include <qcocoaprintersupport.h>

#ifndef QT_NO_PRINTER

#include <qcocoaprintdevice.h>

#include <qprintengine_mac_p.h>
#include <qprinterinfo_p.h>

QCocoaPrinterSupport::QCocoaPrinterSupport()
{ }

QCocoaPrinterSupport::~QCocoaPrinterSupport()
{ }

QPrintEngine *QCocoaPrinterSupport::createNativePrintEngine(QPrinter::PrinterMode printerMode)
{
   return new QMacPrintEngine(printerMode);
}

QPaintEngine *QCocoaPrinterSupport::createPaintEngine(QPrintEngine *printEngine, QPrinter::PrinterMode printerMode)
{
   (void) printerMode;
   /*
       QMacPrintEngine multiply inherits from QPrintEngine and QPaintEngine,
       the cast here allows conversion of QMacPrintEngine* to QPaintEngine*
   */
   return static_cast<QMacPrintEngine *>(printEngine);
}

QPrintDevice QCocoaPrinterSupport::createPrintDevice(const QString &id)
{
   return QPlatformPrinterSupport::createPrintDevice(new QCocoaPrintDevice(id));
}

QStringList QCocoaPrinterSupport::availablePrintDeviceIds() const
{
   QStringList list;
   QCFType<CFArrayRef> printerList;
   if (PMServerCreatePrinterList(kPMServerLocal, &printerList) == noErr) {
      CFIndex count = CFArrayGetCount(printerList);
      for (CFIndex i = 0; i < count; ++i) {
         PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(printerList, i)));
         list.append(QCFString::toQString(PMPrinterGetID(printer)));
      }
   }
   return list;
}

QString QCocoaPrinterSupport::defaultPrintDeviceId() const
{
   QCFType<CFArrayRef> printerList;
   if (PMServerCreatePrinterList(kPMServerLocal, &printerList) == noErr) {
      CFIndex count = CFArrayGetCount(printerList);
      for (CFIndex i = 0; i < count; ++i) {
         PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(printerList, i)));
         if (PMPrinterIsDefault(printer)) {
            return QCFString::toQString(PMPrinterGetID(printer));
         }
      }
   }
   return QString();
}

#endif  //QT_NO_PRINTER
