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

#include <qcupsprintersupport_p.h>

#include <qppdprintdevice.h>
#include <qprinterinfo.h>

#include <qcupsprintengine_p.h>
#include <qprinterinfo_p.h>
#include <qprintdevice_p.h>

#include <cups/ppd.h>

// LSB merges everything into cups.h
#ifndef QT_LINUXBASE
# include <cups/language.h>
#endif

QCupsPrinterSupport::QCupsPrinterSupport()
   : QPlatformPrinterSupport()
{
}

QCupsPrinterSupport::~QCupsPrinterSupport()
{
}

QPrintEngine *QCupsPrinterSupport::createNativePrintEngine(QPrinter::PrinterMode printerMode)
{
   return new QCupsPrintEngine(printerMode);
}

QPaintEngine *QCupsPrinterSupport::createPaintEngine(QPrintEngine *engine, QPrinter::PrinterMode printerMode)
{
   (void) printerMode;

   return static_cast<QCupsPrintEngine *>(engine);
}

QPrintDevice QCupsPrinterSupport::createPrintDevice(const QString &id)
{
   return QPlatformPrinterSupport::createPrintDevice(new QPpdPrintDevice(id));
}

QStringList QCupsPrinterSupport::availablePrintDeviceIds() const
{
   QStringList list;
   cups_dest_t *dests;
   int count = cupsGetDests(&dests);

   for (int i = 0; i < count; ++i) {
      QString printerId = QString::fromUtf8(dests[i].name);

      if (dests[i].instance) {
         printerId += QChar('/') + QString::fromUtf8(dests[i].instance);
      }
      list.append(printerId);
   }

   cupsFreeDests(count, dests);
   return list;
}

QString QCupsPrinterSupport::defaultPrintDeviceId() const
{
   QString printerId;
   cups_dest_t *dests;
   int count = cupsGetDests(&dests);

   for (int i = 0; i < count; ++i) {
      if (dests[i].is_default) {
         printerId = QString::fromUtf8(dests[i].name);
         if (dests[i].instance) {
            printerId += QChar('/') + QString::fromUtf8(dests[i].instance);
            break;
         }
      }
   }

   cupsFreeDests(count, dests);
   return printerId;
}

