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

#ifndef QPRINTDEVICE_H
#define QPRINTDEVICE_H

#include <qprint_p.h>

#include <qsharedpointer.h>
#include <qpagelayout.h>

#ifndef QT_NO_PRINTER

class QPlatformPrintDevice;
class QMarginsF;
class QMimeType;
class QDebug;

class Q_GUI_EXPORT QPrintDevice
{
public:
    QPrintDevice();
    QPrintDevice(const QString & id);
    QPrintDevice(const QPrintDevice &other);
    ~QPrintDevice();

    QPrintDevice &operator=(const QPrintDevice &other);
    QPrintDevice &operator=(QPrintDevice &&other) {
      swap(other); return *this;
    }

    bool operator==(const QPrintDevice &other) const;

    QString id() const;
    QString name() const;
    QString location() const;
    QString makeAndModel() const;

    bool isValid() const;
    bool isDefault() const;
    bool isRemote() const;

    QPrint::DeviceState state() const;

    bool isValidPageLayout(const QPageLayout &layout, int resolution) const;

    void swap(QPrintDevice &other) {
      d.swap(other.d);
    }

    bool supportsMultipleCopies() const;
    bool supportsCollateCopies() const;

    QPageSize defaultPageSize() const;
    QList<QPageSize> supportedPageSizes() const;

    QPageSize supportedPageSize(const QPageSize &pageSize) const;
    QPageSize supportedPageSize(QPageSize::PageSizeId pageSizeId) const;
    QPageSize supportedPageSize(const QString &pageName) const;
    QPageSize supportedPageSize(const QSize &pointSize) const;
    QPageSize supportedPageSize(const QSizeF &size, QPageSize::Unit units = QPageSize::Unit::Point) const;

    bool supportsCustomPageSizes() const;

    QSize minimumPhysicalPageSize() const;
    QSize maximumPhysicalPageSize() const;

    QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation, int resolution) const;

    int defaultResolution() const;
    QList<int> supportedResolutions() const;

    QPrint::InputSlot defaultInputSlot() const;
    QList<QPrint::InputSlot> supportedInputSlots() const;

    QPrint::OutputBin defaultOutputBin() const;
    QList<QPrint::OutputBin> supportedOutputBins() const;

    QPrint::DuplexMode defaultDuplexMode() const;
    QList<QPrint::DuplexMode> supportedDuplexModes() const;

    QPrint::ColorMode defaultColorMode() const;
    QList<QPrint::ColorMode> supportedColorModes() const;

#ifndef QT_NO_MIMETYPE
/* emerald - mimedatabase
    QList<QMimeType> supportedMimeTypes() const;
*/
#endif

   void format(QDebug debug) const;

private:
    QPrintDevice(QPlatformPrintDevice *dd);
    QSharedPointer<QPlatformPrintDevice> d;

    friend class QPlatformPrinterSupport;
    friend class QPlatformPrintDevice;
};

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QPrintDevice &);

#endif // QT_NO_PRINTER

#endif
