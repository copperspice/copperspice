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

#include <qprinterinfo.h>
#include <qprinterinfo_p.h>
#include <qprintdevice_p.h>

#ifndef QT_NO_PRINTER

#include <qdebug.h>

#include <qplatform_printplugin.h>
#include <qplatform_printersupport.h>

static QPrinterInfoPrivate *shared_null()
{
   static QPrinterInfoPrivate retval;
   return &retval;
}

namespace cs_internal {
   void QPrinterInfoPrivateDeleter::operator()(QPrinterInfoPrivate *d) const {
      if (d != shared_null()) {
         delete d;
      }
   }
}

QPrinterInfoPrivate::QPrinterInfoPrivate(const QString &id)
{
   if (!id.isEmpty()) {
      QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
      if (ps) {
         m_printDevice = ps->createPrintDevice(id);
      }
   }
}

QPrinterInfoPrivate::~QPrinterInfoPrivate()
{
}

QPrinterInfo::QPrinterInfo()
   : d_ptr(shared_null())
{
}

QPrinterInfo::QPrinterInfo(const QPrinterInfo &other)
   : d_ptr((other.d_ptr.data() == shared_null()) ? shared_null() : new QPrinterInfoPrivate(*other.d_ptr))
{
}

QPrinterInfo::QPrinterInfo(const QPrinter &printer)
   : d_ptr(shared_null())
{
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
   if (ps) {
      QPrinterInfo pi(printer.printerName());
      if (pi.d_ptr.data() == shared_null()) {
         d_ptr.reset(shared_null());
      } else {
         d_ptr.reset(new QPrinterInfoPrivate(*pi.d_ptr));
      }
   }
}

QPrinterInfo::QPrinterInfo(const QString &name)
   : d_ptr(new QPrinterInfoPrivate(name))
{
}

QPrinterInfo::~QPrinterInfo()
{
}

QPrinterInfo &QPrinterInfo::operator=(const QPrinterInfo &other)
{
   Q_ASSERT(d_ptr);
   if (other.d_ptr.data() == shared_null()) {
      d_ptr.reset(shared_null());
   } else {
      d_ptr.reset(new QPrinterInfoPrivate(*other.d_ptr));
   }
   return *this;
}

QString QPrinterInfo::printerName() const
{
   const Q_D(QPrinterInfo);
   return d->m_printDevice.id();
}

QString QPrinterInfo::description() const
{
   const Q_D(QPrinterInfo);
   return d->m_printDevice.name();
}

QString QPrinterInfo::location() const
{
   const Q_D(QPrinterInfo);
   return d->m_printDevice.location();
}

QString QPrinterInfo::makeAndModel() const
{
   const Q_D(QPrinterInfo);
   return d->m_printDevice.makeAndModel();
}

bool QPrinterInfo::isNull() const
{
   Q_D(const QPrinterInfo);
   return d == shared_null() || !d->m_printDevice.isValid();
}

bool QPrinterInfo::isDefault() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.isDefault();
}

bool QPrinterInfo::isRemote() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.isRemote();
}

QPrinter::PrinterState QPrinterInfo::state() const
{
   Q_D(const QPrinterInfo);
   return QPrinter::PrinterState(d->m_printDevice.state());
}

QList<QPageSize> QPrinterInfo::supportedPageSizes() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.supportedPageSizes();
}

QPageSize QPrinterInfo::defaultPageSize() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.defaultPageSize();
}

bool QPrinterInfo::supportsCustomPageSizes() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.supportsCustomPageSizes();
}

QPageSize QPrinterInfo::minimumPhysicalPageSize() const
{
   Q_D(const QPrinterInfo);
   return QPageSize(d->m_printDevice.minimumPhysicalPageSize(), QString(), QPageSize::ExactMatch);
}

QPageSize QPrinterInfo::maximumPhysicalPageSize() const
{
   Q_D(const QPrinterInfo);
   return QPageSize(d->m_printDevice.maximumPhysicalPageSize(), QString(), QPageSize::ExactMatch);
}

QList<int> QPrinterInfo::supportedResolutions() const
{
   Q_D(const QPrinterInfo);
   return d->m_printDevice.supportedResolutions();
}

QPrinter::DuplexMode QPrinterInfo::defaultDuplexMode() const
{
   Q_D(const QPrinterInfo);
   return QPrinter::DuplexMode(d->m_printDevice.defaultDuplexMode());
}

QList<QPrinter::DuplexMode> QPrinterInfo::supportedDuplexModes() const
{
   Q_D(const QPrinterInfo);
   QList<QPrinter::DuplexMode> list;
   const QList<QPrint::DuplexMode> supportedDuplexModes = d->m_printDevice.supportedDuplexModes();

   for (QPrint::DuplexMode mode : supportedDuplexModes) {
      list << QPrinter::DuplexMode(mode);
   }
   return list;
}

QStringList QPrinterInfo::availablePrinterNames()
{
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
   if (ps) {
      return ps->availablePrintDeviceIds();
   }
   return QStringList();
}

QList<QPrinterInfo> QPrinterInfo::availablePrinters()
{
   QList<QPrinterInfo> list;
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();

   if (ps) {
      const QStringList availablePrintDeviceIds = ps->availablePrintDeviceIds();

      for (const QString &id : availablePrintDeviceIds) {
         list.append(QPrinterInfo(id));
      }
   }
   return list;
}

QString QPrinterInfo::defaultPrinterName()
{
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
   if (ps) {
      return ps->defaultPrintDeviceId();
   }
   return QString();
}

QPrinterInfo QPrinterInfo::defaultPrinter()
{
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
   if (ps) {
      return QPrinterInfo(ps->defaultPrintDeviceId());
   }
   return QPrinterInfo();
}

QPrinterInfo QPrinterInfo::printerInfo(const QString &printerName)
{
   return QPrinterInfo(printerName);
}

QDebug operator<<(QDebug debug, const QPrinterInfo &p)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug << "QPrinterInfo(";

   if (p.isNull()) {
      debug << "null";
   } else {
      p.d_ptr->m_printDevice.format(debug);
   }

   debug << ')';

   return debug;
}

#endif
