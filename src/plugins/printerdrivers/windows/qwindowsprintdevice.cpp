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

#include <qwindowsprintdevice.h>

#include <qdebug.h>
#include <qscopedarraypointer.h>
#include <qstringlist.h>

#ifndef DC_COLLATE
#  define DC_COLLATE 22
#endif

extern qreal qt_pointMultiplier(QPageLayout::Unit unit);

static inline uint qwcsnlen(const wchar_t *str, uint maxlen)
{
   uint length = 0;
   if (str) {
      while (length < maxlen && *str++) {
         length++;
      }
   }

   return length;
}

static QPrint::InputSlot paperBinToInputSlot(int windowsId, const QString &name)
{
   QPrint::InputSlot slot;

   slot.name = name;
   int i;

   for (i = 0; inputSlotMap[i].id != QPrint::CustomInputSlot; ++i) {
      if (inputSlotMap[i].windowsId == windowsId) {
         slot.key = inputSlotMap[i].key;
         slot.id = inputSlotMap[i].id;
         slot.windowsId = inputSlotMap[i].windowsId;
         return slot;
      }
   }

   slot.key = inputSlotMap[i].key;
   slot.id  = inputSlotMap[i].id;
   slot.windowsId = windowsId;

   return slot;
}

static LPDEVMODE getDevmode(HANDLE hPrinter, const QString &printerId)
{
   std::wstring tmp = printerId.toStdWString();
   LPWSTR printerIdUtf16 = const_cast<LPWSTR>(tmp.data());

   // Allocate the required DEVMODE buffer
   LONG dmSize = DocumentProperties(nullptr, hPrinter, printerIdUtf16, nullptr, nullptr, 0);

   if (dmSize < 0) {
      return nullptr;
   }

   LPDEVMODE pDevMode = reinterpret_cast<LPDEVMODE>(malloc(dmSize));

   // Get the default DevMode
   LONG result = DocumentProperties(nullptr, hPrinter, printerIdUtf16, pDevMode, nullptr, DM_OUT_BUFFER);

   if (result != IDOK) {
      free(pDevMode);
      pDevMode = nullptr;
   }

   return pDevMode;
}

QWindowsPrintDevice::QWindowsPrintDevice()
   : QPlatformPrintDevice(), m_hPrinter(nullptr)
{
}

QWindowsPrintDevice::QWindowsPrintDevice(const QString &id)
   : QPlatformPrintDevice(id), m_hPrinter(nullptr)
{
   // First do a fast lookup to see if printer exists, if it does then open it
   if (! id.isEmpty() && QWindowsPrintDevice::availablePrintDeviceIds().contains(id)) {

      std::wstring tmp = m_id.toStdWString();

      if (OpenPrinter(const_cast<LPWSTR>(tmp.data()), &m_hPrinter, nullptr)) {

         DWORD needed = 0;
         GetPrinter(m_hPrinter, 2, nullptr, 0, &needed);
         QScopedArrayPointer<BYTE> buffer(new BYTE[needed]);

         if (GetPrinter(m_hPrinter, 2, buffer.data(), needed, &needed)) {
            PPRINTER_INFO_2 info = reinterpret_cast<PPRINTER_INFO_2>(buffer.data());

            m_name         = QString::fromStdWString(std::wstring(info->pPrinterName));
            m_location     = QString::fromStdWString(std::wstring(info->pLocation));
            m_makeAndModel = QString::fromStdWString(std::wstring(info->pDriverName));       // TODO Check is not available elsewhere

            m_isRemote     = info->Attributes & PRINTER_ATTRIBUTE_NETWORK;
         }

         m_supportsMultipleCopies = (DeviceCapabilities(tmp.data(), nullptr, DC_COPIES, nullptr, nullptr) > 1);
         m_supportsCollateCopies  = DeviceCapabilities(tmp.data(), nullptr, DC_COLLATE, nullptr, nullptr);

         // Min/Max custom size is in tenths of a millimeter
         const qreal multiplier = qt_pointMultiplier(QPageSize::Millimeter);
         DWORD min = DeviceCapabilities(tmp.data(), nullptr, DC_MINEXTENT, nullptr, nullptr);
         m_minimumPhysicalPageSize = QSize((LOWORD(min) / 10.0) * multiplier, (HIWORD(min) / 10.0) * multiplier);

         DWORD max = DeviceCapabilities(tmp.data(), nullptr, DC_MAXEXTENT, nullptr, nullptr);
         m_maximumPhysicalPageSize = QSize((LOWORD(max) / 10.0) * multiplier, (HIWORD(max) / 10.0) * multiplier);
         m_supportsCustomPageSizes = (m_maximumPhysicalPageSize.width() > 0 && m_maximumPhysicalPageSize.height() > 0);
      }
   }
}

QWindowsPrintDevice::~QWindowsPrintDevice()
{
   ClosePrinter(m_hPrinter);
}

bool QWindowsPrintDevice::isValid() const
{
   return m_hPrinter;
}

bool QWindowsPrintDevice::isDefault() const
{
   return m_id == defaultPrintDeviceId();
}

QPrint::DeviceState QWindowsPrintDevice::state() const
{
   DWORD needed = 0;
   GetPrinter(m_hPrinter, 6, nullptr, 0, &needed);
   QScopedArrayPointer<BYTE> buffer(new BYTE[needed]);

   if (GetPrinter(m_hPrinter, 6, buffer.data(), needed, &needed)) {
      PPRINTER_INFO_6 info = reinterpret_cast<PPRINTER_INFO_6>(buffer.data());

      // TODO Check mapping
      if (info->dwStatus == 0
         || (info->dwStatus & PRINTER_STATUS_WAITING) == PRINTER_STATUS_WAITING
         || (info->dwStatus & PRINTER_STATUS_POWER_SAVE) == PRINTER_STATUS_POWER_SAVE) {
         return QPrint::Idle;

      } else if ((info->dwStatus & PRINTER_STATUS_PRINTING) == PRINTER_STATUS_PRINTING
         || (info->dwStatus & PRINTER_STATUS_BUSY) == PRINTER_STATUS_BUSY
         || (info->dwStatus & PRINTER_STATUS_INITIALIZING) == PRINTER_STATUS_INITIALIZING
         || (info->dwStatus & PRINTER_STATUS_IO_ACTIVE) == PRINTER_STATUS_IO_ACTIVE
         || (info->dwStatus & PRINTER_STATUS_PROCESSING) == PRINTER_STATUS_PROCESSING
         || (info->dwStatus & PRINTER_STATUS_WARMING_UP) == PRINTER_STATUS_WARMING_UP) {
         return QPrint::Active;
      }
   }

   return QPrint::Error;
}

void QWindowsPrintDevice::loadPageSizes() const
{
   std::wstring tmp = m_id.toStdWString();

   // Get the number of paper sizes and check all 3 attributes have same count
   DWORD paperCount = DeviceCapabilities(tmp.data(), nullptr, DC_PAPERNAMES, nullptr, nullptr);

   if ( int(paperCount) > 0
         && DeviceCapabilities(tmp.data(), nullptr, DC_PAPERSIZE, nullptr, nullptr) == int(paperCount)
         && DeviceCapabilities(tmp.data(), nullptr, DC_PAPERS, nullptr, nullptr) == int(paperCount) ) {

      QScopedArrayPointer<wchar_t> paperNames(new wchar_t[paperCount * 64]);
      QScopedArrayPointer<POINT>   winSizes(new POINT[paperCount * sizeof(POINT)]);
      QScopedArrayPointer<wchar_t> papers(new wchar_t[paperCount]);

      // Get the details and match the default paper size
      if (DeviceCapabilities(tmp.data(), nullptr, DC_PAPERNAMES, paperNames.data(), nullptr) == int(paperCount)
         && DeviceCapabilities(tmp.data(), nullptr, DC_PAPERSIZE, (wchar_t *)winSizes.data(), nullptr) == int(paperCount)
         && DeviceCapabilities(tmp.data(), nullptr, DC_PAPERS, papers.data(), nullptr) == int(paperCount) ) {

         // Returned size is in tenths of a millimeter
         const qreal multiplier = qt_pointMultiplier(QPageSize::Millimeter);

         for (int i = 0; i < int(paperCount); ++i) {
            QSize size = QSize(qRound((winSizes[i].x / 10.0) * multiplier), qRound((winSizes[i].y / 10.0) * multiplier));
            wchar_t *paper = paperNames.data() + (i * 64);

            QString name = QString::fromStdWString(std::wstring(paper), qwcsnlen(paper, 64));
            m_pageSizes.append(createPageSize(papers[i], size, name));
         }

      }
   }

   m_havePageSizes = true;
}

QPageSize QWindowsPrintDevice::defaultPageSize() const
{
   if (!m_havePageSizes) {
      loadPageSizes();
   }

   QPageSize pageSize;

   if (LPDEVMODE pDevMode = getDevmode(m_hPrinter, m_id)) {
      // Get the default paper size
      if (pDevMode->dmFields & DM_PAPERSIZE) {
         // Find the supported page size that matches, in theory default should be one of them
         for (const QPageSize &ps : m_pageSizes) {
            if (ps.windowsId() == pDevMode->dmPaperSize) {
               pageSize = ps;
               break;
            }
         }
      }

      // Clean-up
      free(pDevMode);
   }

   return pageSize;
}

QMarginsF QWindowsPrintDevice::printableMargins(const QPageSize &pageSize,
   QPageLayout::Orientation orientation, int resolution) const
{
   // TODO This is slow, need to cache values or find better way!
   // Modify the DevMode to get the DC printable margins in device pixels
   QMarginsF margins = QMarginsF(0, 0, 0, 0);
   DWORD needed = 0;
   GetPrinter(m_hPrinter, 2, nullptr, 0, &needed);
   QScopedArrayPointer<BYTE> buffer(new BYTE[needed]);

   if (GetPrinter(m_hPrinter, 2, buffer.data(), needed, &needed)) {
      PPRINTER_INFO_2 info = reinterpret_cast<PPRINTER_INFO_2>(buffer.data());
      LPDEVMODE devMode = info->pDevMode;
      bool separateDevMode = false;
      if (!devMode) {

         // GetPrinter() didn't include the DEVMODE. Get it a different way.
         devMode = getDevmode(m_hPrinter, m_id);
         if (!devMode) {
            return margins;
         }

         separateDevMode = true;
      }

      std::wstring tmp = m_id.toStdWString();

      HDC pDC = CreateDC(nullptr, tmp.data(), nullptr, devMode);

      if (pageSize.id() == QPageSize::Custom || pageSize.windowsId() <= 0 || pageSize.windowsId() > DMPAPER_LAST) {
         devMode->dmPaperSize   =  0;
         devMode->dmPaperWidth  = pageSize.size(QPageSize::Millimeter).width() * 10.0;
         devMode->dmPaperLength = pageSize.size(QPageSize::Millimeter).height() * 10.0;
      } else {
         devMode->dmPaperSize =  pageSize.windowsId();
      }

      devMode->dmPrintQuality = resolution;
      devMode->dmOrientation = orientation == QPageLayout::Portrait ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;

      ResetDC(pDC, devMode);
      const int dpiWidth = GetDeviceCaps(pDC, LOGPIXELSX);
      const int dpiHeight = GetDeviceCaps(pDC, LOGPIXELSY);
      const qreal wMult = 72.0 / dpiWidth;
      const qreal hMult = 72.0 / dpiHeight;
      const qreal physicalWidth = GetDeviceCaps(pDC, PHYSICALWIDTH) * wMult;
      const qreal physicalHeight = GetDeviceCaps(pDC, PHYSICALHEIGHT) * hMult;
      const qreal printableWidth = GetDeviceCaps(pDC, HORZRES) * wMult;
      const qreal printableHeight = GetDeviceCaps(pDC, VERTRES) * hMult;
      const qreal leftMargin = GetDeviceCaps(pDC, PHYSICALOFFSETX) * wMult;
      const qreal topMargin = GetDeviceCaps(pDC, PHYSICALOFFSETY) * hMult;
      const qreal rightMargin = physicalWidth - leftMargin - printableWidth;
      const qreal bottomMargin = physicalHeight - topMargin - printableHeight;
      margins = QMarginsF(leftMargin, topMargin, rightMargin, bottomMargin);

      if (separateDevMode) {
         free(devMode);
      }
      DeleteDC(pDC);
   }

   return margins;
}

void QWindowsPrintDevice::loadResolutions() const
{
   std::wstring tmp = m_id.toStdWString();

   DWORD resCount = DeviceCapabilities(tmp.data(), nullptr, DC_ENUMRESOLUTIONS, nullptr, nullptr);

   if (int(resCount) > 0) {
      QScopedArrayPointer<LONG> resolutions(new LONG[resCount * 2]);
      // Get the details and match the default paper size

      if (DeviceCapabilities(tmp.data(), nullptr, DC_ENUMRESOLUTIONS, (LPWSTR)resolutions.data(), nullptr) == static_cast<int>(resCount)) {
         for (int i = 0; i < int(resCount * 2); i += 2) {
            m_resolutions.append(resolutions[i + 1]);
         }
      }
   }

   m_haveResolutions = true;
}

int QWindowsPrintDevice::defaultResolution() const
{
   int resolution = 72;  // TODO Set a sensible default?

   if (LPDEVMODE pDevMode = getDevmode(m_hPrinter, m_id)) {
      // Get the default resolution
      if (pDevMode->dmFields & DM_YRESOLUTION) {
         if (pDevMode->dmPrintQuality > 0) {
            resolution = pDevMode->dmPrintQuality;
         } else {
            resolution = pDevMode->dmYResolution;
         }
      }

      // Clean-up
      free(pDevMode);
   }

   return resolution;
}

void QWindowsPrintDevice::loadInputSlots() const
{
   std::wstring tmp = m_id.toStdWString();

   DWORD binCount = DeviceCapabilities(tmp.data(), nullptr, DC_BINS, nullptr, nullptr);

   if (static_cast<int>(binCount) > 0
      && DeviceCapabilities(tmp.data(), nullptr, DC_BINNAMES, nullptr, nullptr) == static_cast<int>(binCount)) {

      QScopedArrayPointer<WORD> bins(new WORD[binCount * sizeof(WORD)]);
      QScopedArrayPointer<wchar_t> binNames(new wchar_t[binCount * 24]);

      // Get the details and match the default paper size
      if (DeviceCapabilities(tmp.data(), nullptr, DC_BINS, (LPWSTR)bins.data(), nullptr) == static_cast<int>(binCount)
         && DeviceCapabilities(tmp.data(), nullptr, DC_BINNAMES, binNames.data(), nullptr) == static_cast<int>(binCount)) {

         for (int i = 0; i < int(binCount); ++i) {
            wchar_t *binName = binNames.data() + (i * 24);
            QString name = QString::fromStdWString(std::wstring(binName), qwcsnlen(binName, 24));
            m_inputSlots.append(paperBinToInputSlot(bins[i], name));
         }

      }
   }

   m_haveInputSlots = true;
}

QPrint::InputSlot QWindowsPrintDevice::defaultInputSlot() const
{
   QPrint::InputSlot inputSlot = QPlatformPrintDevice::defaultInputSlot();;

   if (LPDEVMODE pDevMode = getDevmode(m_hPrinter, m_id)) {
      // Get the default input slot
      if (pDevMode->dmFields & DM_DEFAULTSOURCE) {
         QPrint::InputSlot tempSlot = paperBinToInputSlot(pDevMode->dmDefaultSource, QString());

         for (const QPrint::InputSlot &slot : supportedInputSlots()) {
            if (slot.key == tempSlot.key) {
               inputSlot = slot;
               break;
            }
         }
      }

      // Clean-up
      free(pDevMode);
   }
   return inputSlot;
}

void QWindowsPrintDevice::loadOutputBins() const
{
   m_outputBins.append(QPlatformPrintDevice::defaultOutputBin());
   m_haveOutputBins = true;
}

void QWindowsPrintDevice::loadDuplexModes() const
{
   std::wstring tmp = m_id.toStdWString();

   m_duplexModes.append(QPrint::DuplexNone);
   DWORD duplex = DeviceCapabilities(tmp.data(), nullptr, DC_DUPLEX, nullptr, nullptr);

   if (int(duplex) == 1) {
      // TODO Assume if duplex flag supports both modes
      m_duplexModes.append(QPrint::DuplexAuto);
      m_duplexModes.append(QPrint::DuplexLongSide);
      m_duplexModes.append(QPrint::DuplexShortSide);
   }

   m_haveDuplexModes = true;
}

QPrint::DuplexMode QWindowsPrintDevice::defaultDuplexMode() const
{
   QPrint::DuplexMode duplexMode = QPrint::DuplexNone;

   if (LPDEVMODE pDevMode = getDevmode(m_hPrinter, m_id)) {
      // Get the default duplex mode
      if (pDevMode->dmFields & DM_DUPLEX) {
         if (pDevMode->dmDuplex == DMDUP_VERTICAL) {
            duplexMode = QPrint::DuplexLongSide;
         } else if (pDevMode->dmDuplex == DMDUP_HORIZONTAL) {
            duplexMode = QPrint::DuplexShortSide;
         }
      }
      // Clean-up
      free(pDevMode);
   }
   return duplexMode;
}

void QWindowsPrintDevice::loadColorModes() const
{
   std::wstring tmp = m_id.toStdWString();

   m_colorModes.append(QPrint::GrayScale);
   DWORD color = DeviceCapabilities(tmp.data(), nullptr, DC_COLORDEVICE, nullptr, nullptr);

   if (int(color) == 1) {
      m_colorModes.append(QPrint::Color);
   }

   m_haveColorModes = true;
}

QPrint::ColorMode QWindowsPrintDevice::defaultColorMode() const
{
   if (!m_haveColorModes) {
      loadColorModes();
   }

   if (!m_colorModes.contains(QPrint::Color)) {
      return QPrint::GrayScale;
   }

   QPrint::ColorMode colorMode = QPrint::GrayScale;

   if (LPDEVMODE pDevMode = getDevmode(m_hPrinter, m_id)) {
      // Get the default color mode
      if (pDevMode->dmFields & DM_COLOR && pDevMode->dmColor == DMCOLOR_COLOR) {
         colorMode = QPrint::Color;
      }
      // Clean-up
      free(pDevMode);
   }
   return colorMode;
}

QStringList QWindowsPrintDevice::availablePrintDeviceIds()
{
   QStringList list;
   DWORD needed = 0;
   DWORD returned = 0;

   if ((! EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, nullptr, 0, &needed, &returned) &&
         GetLastError() != ERROR_INSUFFICIENT_BUFFER) || ! needed) {
      return list;
   }

   QScopedArrayPointer<BYTE> buffer(new BYTE[needed]);

   if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, buffer.data(), needed, &needed, &returned)) {
      return list;
   }

   PPRINTER_INFO_4 infoList = reinterpret_cast<PPRINTER_INFO_4>(buffer.data());

   for (uint i = 0; i < returned; ++i) {
      list.append(QString::fromStdWString(std::wstring(infoList[i].pPrinterName)));
   }

   return list;
}

QString QWindowsPrintDevice::defaultPrintDeviceId()
{
   DWORD size = 0;
   GetDefaultPrinter(nullptr, &size);
   QScopedArrayPointer<wchar_t> name(new wchar_t[size]);
   GetDefaultPrinter(name.data(), &size);

   return QString::fromStdWString(std::wstring(name.data()));
}


