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

#ifndef QPPDPRINTDEVICE_H
#define QPPDPRINTDEVICE_H

#include <qbytearray.h>
#include <qhash.h>
#include <qmargins.h>
#include <qplatform_printdevice.h>

#include <cups/cups.h>
#include <cups/ppd.h>

// emerald
#define QT_NO_MIMETYPE

class QPpdPrintDevice : public QPlatformPrintDevice
{
 public:
   QPpdPrintDevice();
   explicit QPpdPrintDevice(const QString &id);
   virtual ~QPpdPrintDevice();

   bool isValid() const override;
   bool isDefault() const override;

   QPrint::DeviceState state() const override;

   QPageSize defaultPageSize() const override;

   QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation,
      int resolution) const override;

   int defaultResolution() const override;

   QPrint::InputSlot defaultInputSlot() const override;

   QPrint::OutputBin defaultOutputBin() const override;

   QPrint::DuplexMode defaultDuplexMode() const override;

   QPrint::ColorMode defaultColorMode() const override;

 protected:
   void loadPageSizes() const override;
   void loadResolutions() const override;
   void loadInputSlots() const override;
   void loadOutputBins() const override;
   void loadDuplexModes() const override;
   void loadColorModes() const override;

#ifndef QT_NO_MIMETYPE
   void loadMimeTypes() const override;
#endif

 private:
   void loadPrinter();
   QString printerOption(const QString &key) const;
   cups_ptype_e printerTypeFlags() const;

   cups_dest_t *m_cupsDest;
   ppd_file_t *m_ppd;
   QByteArray m_cupsName;
   QByteArray m_cupsInstance;
   QMarginsF m_customMargins;
   mutable QHash<QString, QMarginsF> m_printableMargins;
};

#endif
