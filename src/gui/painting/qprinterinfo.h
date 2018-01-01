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

#ifndef QPRINTERINFO_H
#define QPRINTERINFO_H

#include <QtCore/QList>
#include <QtGui/QPrinter>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER
class QPrinterInfoPrivate;
class QPrinterInfoPrivateDeleter;

class Q_GUI_EXPORT QPrinterInfo
{
 public:
   QPrinterInfo();
   QPrinterInfo(const QPrinterInfo &other);
   QPrinterInfo(const QPrinter &printer);
   ~QPrinterInfo();

   QPrinterInfo &operator=(const QPrinterInfo &other);

   QString printerName() const;
   bool isNull() const;
   bool isDefault() const;
   QList<QPrinter::PaperSize> supportedPaperSizes() const;

   static QList<QPrinterInfo> availablePrinters();
   static QPrinterInfo defaultPrinter();

 private:
   QPrinterInfo(const QString &name);

   Q_DECLARE_PRIVATE(QPrinterInfo)
   QScopedPointer<QPrinterInfoPrivate, QPrinterInfoPrivateDeleter> d_ptr;
};

#endif // QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QPRINTERINFO_H
