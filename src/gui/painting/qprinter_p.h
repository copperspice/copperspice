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

#ifndef QPRINTER_P_H
#define QPRINTER_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_PRINTER

#include <QtGui/qprinter.h>
#include <QtGui/qprintengine.h>
#include <QtCore/qpointer.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

class QPrintEngine;
class QPreviewPaintEngine;
class QPicture;

class QPrinterPrivate
{
   Q_DECLARE_PUBLIC(QPrinter)

 public:
   QPrinterPrivate(QPrinter *printer)
      : printEngine(0)
      , paintEngine(0)
      , q_ptr(printer)
      , printRange(QPrinter::AllPages)
      , minPage(1)
      , maxPage(INT_MAX)
      , fromPage(0)
      , toPage(0)
      , use_default_engine(true)
      , validPrinter(false)
      , hasCustomPageMargins(false)
      , hasUserSetPageSize(false) {
   }

   virtual ~QPrinterPrivate() {

   }

   void createDefaultEngines();

#ifndef QT_NO_PRINTPREVIEWWIDGET
   QList<const QPicture *> previewPages() const;
   void setPreviewMode(bool);
#endif

   void addToManualSetList(QPrintEngine::PrintEnginePropertyKey key);

   QPrinter::PrinterMode printerMode;
   QPrinter::OutputFormat outputFormat;
   QPrintEngine *printEngine;
   QPaintEngine *paintEngine;

   QPrintEngine *realPrintEngine;
   QPaintEngine *realPaintEngine;

#ifndef QT_NO_PRINTPREVIEWWIDGET
   QPreviewPaintEngine *previewEngine;
#endif

   QPrinter *q_ptr;

   QPrinter::PrintRange printRange;
   int minPage, maxPage, fromPage, toPage;

   uint use_default_engine : 1;
   uint had_default_engines : 1;

   uint validPrinter : 1;
   uint hasCustomPageMargins : 1;
   uint hasUserSetPageSize : 1;

   // Used to remember which properties have been manually set by the user.
   QList<QPrintEngine::PrintEnginePropertyKey> manualSetList;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTER_P_H
