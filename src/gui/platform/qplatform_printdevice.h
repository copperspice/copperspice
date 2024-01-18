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

#ifndef QPLATFORMPRINTDEVICE_H
#define QPLATFORMPRINTDEVICE_H

#include <qprint_p.h>

#include <qvector.h>
#include <qpagelayout.h>

// emerald   #include <qmimetype.h>

#ifndef QT_NO_PRINTER

class Q_GUI_EXPORT QPlatformPrintDevice
{
 public:
    QPlatformPrintDevice();
    explicit QPlatformPrintDevice(const QString &id);

    QPlatformPrintDevice(const QPlatformPrintDevice &) = delete;
    QPlatformPrintDevice &operator=(const QPlatformPrintDevice &) = delete;

    virtual ~QPlatformPrintDevice();

    virtual QString id() const;
    virtual QString name() const;
    virtual QString location() const;
    virtual QString makeAndModel() const;

    virtual bool isValid() const;
    virtual bool isDefault() const;
    virtual bool isRemote() const;

    virtual QPrint::DeviceState state() const;

    virtual bool isValidPageLayout(const QPageLayout &layout, int resolution) const;

    virtual bool supportsMultipleCopies() const;
    virtual bool supportsCollateCopies() const;

    virtual QPageSize defaultPageSize() const;
    virtual QList<QPageSize> supportedPageSizes() const;

    virtual QPageSize supportedPageSize(const QPageSize &pageSize) const;
    virtual QPageSize supportedPageSize(QPageSize::PageSizeId pageSizeId) const;
    virtual QPageSize supportedPageSize(const QString &pageName) const;
    virtual QPageSize supportedPageSize(const QSize &pointSize) const;
    virtual QPageSize supportedPageSize(const QSizeF &size, QPageSize::Unit units) const;

    virtual bool supportsCustomPageSizes() const;

    virtual QSize minimumPhysicalPageSize() const;
    virtual QSize maximumPhysicalPageSize() const;

    virtual QMarginsF printableMargins(const QPageSize &pageSize, QPageLayout::Orientation orientation,
                                       int resolution) const;

    virtual int defaultResolution() const;
    virtual QList<int> supportedResolutions() const;

    virtual QPrint::InputSlot defaultInputSlot() const;
    virtual QList<QPrint::InputSlot> supportedInputSlots() const;

    virtual QPrint::OutputBin defaultOutputBin() const;
    virtual QList<QPrint::OutputBin> supportedOutputBins() const;

    virtual QPrint::DuplexMode defaultDuplexMode() const;
    virtual QList<QPrint::DuplexMode> supportedDuplexModes() const;

    virtual QPrint::ColorMode defaultColorMode() const;
    virtual QList<QPrint::ColorMode> supportedColorModes() const;

#ifndef QT_NO_MIMETYPE
   // emerald   virtual QList<QMimeType> supportedMimeTypes() const;
#endif

    static QPageSize createPageSize(const QString &key, const QSize &size, const QString &localizedName);
    static QPageSize createPageSize(int windowsId, const QSize &size, const QString &localizedName);

 protected:
    virtual void loadPageSizes() const;
    virtual void loadResolutions() const;
    virtual void loadInputSlots() const;
    virtual void loadOutputBins() const;
    virtual void loadDuplexModes() const;
    virtual void loadColorModes() const;

#ifndef QT_NO_MIMETYPE
    // emerald   virtual void loadMimeTypes() const;
#endif

    QPageSize supportedPageSizeMatch(const QPageSize &pageSize) const;

    QString m_id;
    QString m_name;
    QString m_location;
    QString m_makeAndModel;

    bool m_isRemote;

    bool m_supportsMultipleCopies;
    bool m_supportsCollateCopies;

    mutable bool m_havePageSizes;
    mutable QVector<QPageSize> m_pageSizes;

    bool m_supportsCustomPageSizes;

    QSize m_minimumPhysicalPageSize;
    QSize m_maximumPhysicalPageSize;

    mutable bool m_haveResolutions;
    mutable QVector<int> m_resolutions;

    mutable bool m_haveInputSlots;
    mutable QVector<QPrint::InputSlot> m_inputSlots;

    mutable bool m_haveOutputBins;
    mutable QVector<QPrint::OutputBin> m_outputBins;

    mutable bool m_haveDuplexModes;
    mutable QVector<QPrint::DuplexMode> m_duplexModes;

    mutable bool m_haveColorModes;
    mutable QVector<QPrint::ColorMode> m_colorModes;

#ifndef QT_NO_MIMETYPE
   // emerald    mutable bool m_haveMimeTypes;
   // emerald    mutable QVector<QMimeType> m_mimeTypes;
#endif
};

#endif // QT_NO_PRINTER

#endif