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

#ifndef QPLATFORMPRINTERSUPPORT_H
#define QPLATFORMPRINTERSUPPORT_H

#include <qprinter.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <QtCore/qhash.h>

#ifndef QT_NO_PRINTER

typedef QHash<QString, QString> PrinterOptions;

class QPageSize;
class QPlatformPrintDevice;
class QPrintDevice;
class QPrintEngine;

class Q_GUI_EXPORT QPlatformPrinterSupport
{
public:
    QPlatformPrinterSupport();
    virtual ~QPlatformPrinterSupport();

    virtual QPrintEngine *createNativePrintEngine(QPrinter::PrinterMode printerMode);
    virtual QPaintEngine *createPaintEngine(QPrintEngine *, QPrinter::PrinterMode printerMode);

    virtual QPrintDevice createPrintDevice(const QString &id);
    virtual QPrintDevice createDefaultPrintDevice();
    virtual QStringList availablePrintDeviceIds() const;
    virtual QString defaultPrintDeviceId() const;

protected:
    static QPrintDevice createPrintDevice(QPlatformPrintDevice *device);
    static QPageSize createPageSize(const QString &id, QSize size, const QString &localizedName);
};

#endif // QT_NO_PRINTER



#endif // QPLATFORMPRINTERSUPPORT_H
