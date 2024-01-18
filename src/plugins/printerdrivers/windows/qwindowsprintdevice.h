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

#ifndef QWINDOWSPRINTDEVICE_H
#define QWINDOWSPRINTDEVICE_H

#include <qplatform_printdevice.h>
#include <qt_windows.h>

class QWindowsPrintDevice : public QPlatformPrintDevice
{
 public:
   QWindowsPrintDevice();
   explicit QWindowsPrintDevice(const QString &id);
   virtual ~QWindowsPrintDevice();

   bool isValid() const override;
   bool isDefault() const override;

   QPrint::DeviceState state() const override;

   QPageSize defaultPageSize() const override;

   QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation,
      int resolution) const override;

   int defaultResolution() const override;

   QPrint::InputSlot defaultInputSlot() const override;

   QPrint::DuplexMode defaultDuplexMode() const override;

   QPrint::ColorMode defaultColorMode() const override;

   static QStringList availablePrintDeviceIds();
   static QString defaultPrintDeviceId();

 protected:
   void loadPageSizes() const override;
   void loadResolutions() const override;
   void loadInputSlots() const override;
   void loadOutputBins() const override;
   void loadDuplexModes() const override;
   void loadColorModes() const override;

 private:
   HANDLE m_hPrinter;
};

#endif
