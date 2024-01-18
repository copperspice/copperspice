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

#ifndef QPRINTERINFO_H
#define QPRINTERINFO_H

#include <qprinter.h>
#include <qlist.h>
#include <qpair.h>
#include <qpagesize.h>

#ifndef QT_NO_PRINTER

class QPrinterInfoPrivate;
class QPrinterInfoPrivateDeleter;
class QDebug;

namespace cs_internal {
   struct QPrinterInfoPrivateDeleter {
      void operator()(QPrinterInfoPrivate *d) const;
   };
}

class Q_GUI_EXPORT QPrinterInfo
{
 public:
   QPrinterInfo();
   QPrinterInfo(const QPrinterInfo &other);
   explicit QPrinterInfo(const QPrinter &printer);

   ~QPrinterInfo();

   QPrinterInfo &operator=(const QPrinterInfo &other);

   QString printerName() const;
   QString description() const;
   QString location() const;
   QString makeAndModel() const;
   bool isNull() const;
   bool isDefault() const;
   bool isRemote() const;

   QPrinter::PrinterState state() const;

   QList<QPageSize> supportedPageSizes() const;
   QPageSize defaultPageSize() const;

   bool supportsCustomPageSizes() const;

   QPageSize minimumPhysicalPageSize() const;
   QPageSize maximumPhysicalPageSize() const;


   QList<int> supportedResolutions() const;

   QPrinter::DuplexMode defaultDuplexMode() const;
   QList<QPrinter::DuplexMode> supportedDuplexModes() const;

   static QStringList availablePrinterNames();
   static QList<QPrinterInfo> availablePrinters();

   static QString defaultPrinterName();
   static QPrinterInfo defaultPrinter();

   static QPrinterInfo printerInfo(const QString &printerName);

 private:
   explicit QPrinterInfo(const QString &name);

   friend class QPlatformPrinterSupport;
   friend Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QPrinterInfo &);
   Q_DECLARE_PRIVATE(QPrinterInfo)

   QScopedPointer<QPrinterInfoPrivate, cs_internal::QPrinterInfoPrivateDeleter> d_ptr;
};

#endif // QT_NO_PRINTER


#endif