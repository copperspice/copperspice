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

#ifndef QCOCOAPRINTDEVICE_H
#define QCOCOAPRINTDEVICE_H

#include <qplatform_printdevice.h>

#ifndef QT_NO_PRINTER

#include <qt_mac_p.h>

#include <cups/ppd.h>

class QCocoaPrintDevice : public QPlatformPrintDevice
{
 public:
   QCocoaPrintDevice();
   explicit QCocoaPrintDevice(const QString &id);
   virtual ~QCocoaPrintDevice();

   bool isValid() const override;
   bool isDefault() const override;

   QPrint::DeviceState state() const override;

   QPageSize defaultPageSize() const override;

   QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation,
      int resolution) const override;

   int defaultResolution() const override;

   QPrint::ColorMode defaultColorMode() const override;
   QPrint::DuplexMode defaultDuplexMode() const override;
   QPrint::InputSlot defaultInputSlot() const override;
   QPrint::OutputBin defaultOutputBin() const override;

   PMPrinter macPrinter() const;
   PMPaper macPaper(const QPageSize &pageSize) const;

 protected:
   void loadPageSizes() const override;
   void loadResolutions() const override;
   void loadInputSlots() const override;
   void loadOutputBins() const override;
   void loadDuplexModes() const override;
   void loadColorModes() const override;

#if ! defined(QT_NO_MIMETYPE)
   void loadMimeTypes() const;
#endif

 private:
   QPageSize createPageSize(const PMPaper &paper) const;
   bool openPpdFile();

   // Mac Core Printing
   PMPrinter m_printer;
   PMPrintSession m_session;
   mutable QHash<QString, PMPaper> m_macPapers;

   // PPD File
   ppd_file_t *m_ppd;

   QMarginsF m_customMargins;
   mutable QHash<QString, QMarginsF> m_printableMargins;
};

#endif // QT_NO_PRINTER
#endif