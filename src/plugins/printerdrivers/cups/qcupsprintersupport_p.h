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

#ifndef QCUPSPRINTERSUPPORT_H
#define QCUPSPRINTERSUPPORT_H

#include <qplatform_printersupport.h>
#include <qstringlist.h>

class QCupsPrinterSupport : public QPlatformPrinterSupport
{
 public:
   QCupsPrinterSupport();
   ~QCupsPrinterSupport();

   QPrintEngine *createNativePrintEngine(QPrinter::PrinterMode printerMode) override;
   QPaintEngine *createPaintEngine(QPrintEngine *printEngine, QPrinter::PrinterMode) override;

   QPrintDevice createPrintDevice(const QString &id) override;
   QStringList availablePrintDeviceIds() const override;
   QString defaultPrintDeviceId() const override;

 private:
   QString cupsOption(int i, const QString &key) const;
};

#endif
