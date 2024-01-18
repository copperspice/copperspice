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

#include <qwindowsprintersupport.h>

#include <qwindowsprintdevice.h>
#include <qstringlist.h>

#include <qprintengine_win_p.h>
#include <qprintdevice_p.h>

QWindowsPrinterSupport::QWindowsPrinterSupport()
   : QPlatformPrinterSupport()
{
}

QWindowsPrinterSupport::~QWindowsPrinterSupport()
{
}

QPrintEngine *QWindowsPrinterSupport::createNativePrintEngine(QPrinter::PrinterMode printerMode)
{
   return new QWin32PrintEngine(printerMode);
}

QPaintEngine *QWindowsPrinterSupport::createPaintEngine(QPrintEngine *engine, QPrinter::PrinterMode printerMode)
{
   (void) printerMode;
   return static_cast<QWin32PrintEngine *>(engine);
}

QPrintDevice QWindowsPrinterSupport::createPrintDevice(const QString &id)
{
   return QPlatformPrinterSupport::createPrintDevice(new QWindowsPrintDevice(id));
}

QStringList QWindowsPrinterSupport::availablePrintDeviceIds() const
{
   return QWindowsPrintDevice::availablePrintDeviceIds();
}

QString QWindowsPrinterSupport::defaultPrintDeviceId() const
{
   return QWindowsPrintDevice::defaultPrintDeviceId();
}

