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

#ifndef QPRINTENGINE_H
#define QPRINTENGINE_H

#include <QtCore/qvariant.h>
#include <QtGui/qprinter.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

class Q_GUI_EXPORT QPrintEngine
{
 public:
   virtual ~QPrintEngine() {}
   enum PrintEnginePropertyKey {
      PPK_CollateCopies,
      PPK_ColorMode,
      PPK_Creator,
      PPK_DocumentName,
      PPK_FullPage,
      PPK_NumberOfCopies,
      PPK_Orientation,
      PPK_OutputFileName,
      PPK_PageOrder,
      PPK_PageRect,
      PPK_PageSize,
      PPK_PaperRect,
      PPK_PaperSource,
      PPK_PrinterName,
      PPK_PrinterProgram,
      PPK_Resolution,
      PPK_SelectionOption,
      PPK_SupportedResolutions,

      PPK_WindowsPageSize,
      PPK_FontEmbedding,
      PPK_SuppressSystemPrintStatus,

      PPK_Duplex,

      PPK_PaperSources,
      PPK_CustomPaperSize,
      PPK_PageMargins,
      PPK_CopyCount,
      PPK_SupportsMultipleCopies,
      PPK_PaperSize = PPK_PageSize,

      PPK_CustomBase = 0xff00
   };

   virtual void setProperty(PrintEnginePropertyKey key, const QVariant &value) = 0;
   virtual QVariant property(PrintEnginePropertyKey key) const = 0;

   virtual bool newPage() = 0;
   virtual bool abort() = 0;

   virtual int metric(QPaintDevice::PaintDeviceMetric) const = 0;

   virtual QPrinter::PrinterState printerState() const = 0;

#ifdef Q_OS_WIN
   virtual HDC getPrinterDC() const {
      return 0;
   }
   virtual void releasePrinterDC(HDC) const { }
#endif

};

#endif // QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QPRINTENGINE_H
