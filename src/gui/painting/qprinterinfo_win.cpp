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
#include <qstringlist.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

extern QPrinter::PaperSize mapDevmodePaperSize(int s);

QList<QPrinterInfo> QPrinterInfo::availablePrinters()
{
   QList<QPrinterInfo> printers;

   DWORD needed = 0;
   DWORD returned = 0;

   if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, 0, 0, &needed, &returned)) {
      LPBYTE buffer = new BYTE[needed];

      if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, buffer, needed, &needed, &returned)) {
         PPRINTER_INFO_4 infoList = reinterpret_cast<PPRINTER_INFO_4>(buffer);
         QPrinterInfo defPrn = defaultPrinter();

         for (uint i = 0; i < returned; ++i) {
            QString printerName(QString::fromStdWString(std::wstring(infoList[i].pPrinterName)));
            QPrinterInfo printerInfo(printerName);

            if (printerInfo.printerName() == defPrn.printerName()) {
               printerInfo.d_ptr->isDefault = true;
            }

            printers.append(printerInfo);
         }
      }

      delete [] buffer;
   }

   return printers;
}

QPrinterInfo QPrinterInfo::defaultPrinter()
{
   std::wstring buffer(256, L'\0');

   GetProfileString(L"windows", L"device", L"no_printers", &buffer[0], 256);
   QString output = QString::fromStdWString(buffer);

   if (output != "no_printers") {
      // Filter out the name of the printer, which should be everything before a comma
      QString printerName = output.split(',').value(0);
      QPrinterInfo printerInfo(printerName);

      printerInfo.d_ptr->isDefault = true;
      return printerInfo;
   }

   return QPrinterInfo();
}

QList<QPrinter::PaperSize> QPrinterInfo::supportedPaperSizes() const
{
   const Q_D(QPrinterInfo);

   QList<QPrinter::PaperSize> paperSizes;

   if (isNull()) {
      return paperSizes;
   }

   DWORD size = DeviceCapabilities(&d->name.toStdWString()[0], NULL, DC_PAPERS, NULL, NULL);

   if ((int)size != -1) {
      wchar_t *papers = new wchar_t[size];
      size = DeviceCapabilities(&d->name.toStdWString()[0], NULL, DC_PAPERS, papers, NULL);

      for (int c = 0; c < (int)size; ++c) {
         paperSizes.append(mapDevmodePaperSize(papers[c]));
      }

      delete [] papers;
   }

   return paperSizes;
}

#endif // QT_NO_PRINTER

QT_END_NAMESPACE
