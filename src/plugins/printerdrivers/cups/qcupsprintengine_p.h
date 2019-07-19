/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QCUPSPRINTENGINE_P_H
#define QCUPSPRINTENGINE_P_H

#include <qprintengine.h>

#include <qstring.h>
#include <qpaintengine.h>

#include <qpaintengine_p.h>
#include <qprintdevice_p.h>
#include <qprintengine_pdf_p.h>

class QCupsPrintEnginePrivate;

class QCupsPrintEngine : public QPdfPrintEngine
{
   Q_DECLARE_PRIVATE(QCupsPrintEngine)

 public:
   QCupsPrintEngine(QPrinter::PrinterMode m);
   virtual ~QCupsPrintEngine();

   // next two are a reimplementations of QPdfPrintEngine
   void setProperty(PrintEnginePropertyKey key, const QVariant &value) override;
   QVariant property(PrintEnginePropertyKey key) const override;

 private:
   Q_DISABLE_COPY(QCupsPrintEngine)
};

class QCupsPrintEnginePrivate : public QPdfPrintEnginePrivate
{
   Q_DECLARE_PUBLIC(QCupsPrintEngine)

 public:
   QCupsPrintEnginePrivate(QPrinter::PrinterMode m);
   ~QCupsPrintEnginePrivate();

   bool openPrintDevice() override;
   void closePrintDevice() override;

 private:
   Q_DISABLE_COPY(QCupsPrintEnginePrivate)

   void setupDefaultPrinter();
   void changePrinter(const QString &newPrinter);
   void setPageSize(const QPageSize &pageSize);

   QPrintDevice m_printDevice;
   QStringList cupsOptions;
   QString cupsTempFile;
};

#endif
