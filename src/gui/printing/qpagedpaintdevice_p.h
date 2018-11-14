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

#ifndef QPAGEDPAINTDEVICE_P_H
#define QPAGEDPAINTDEVICE_P_H

#include <qpagedpaintdevice.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QPagedPaintDevicePrivate
{
public:
    QPagedPaintDevicePrivate()
        : m_pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0, 0, 0, 0)),
          fromPage(0),
          toPage(0),
          pageOrderAscending(true),
          printSelectionOnly(false)
    {
    }

    virtual ~QPagedPaintDevicePrivate();

    // ### Qt6 Remove these and make public class methods virtual
    virtual bool setPageLayout(const QPageLayout &newPageLayout)
    {
        m_pageLayout = newPageLayout;
        return m_pageLayout.isEquivalentTo(newPageLayout);;
    }

    virtual bool setPageSize(const QPageSize &pageSize)
    {
        m_pageLayout.setPageSize(pageSize);
        return m_pageLayout.pageSize().isEquivalentTo(pageSize);
    }

    virtual bool setPageOrientation(QPageLayout::Orientation orientation)
    {
        m_pageLayout.setOrientation(orientation);
        return m_pageLayout.orientation() == orientation;
    }

    virtual bool setPageMargins(const QMarginsF &margins)
    {
        return setPageMargins(margins, m_pageLayout.units());
    }

    virtual bool setPageMargins(const QMarginsF &margins, QPageLayout::Unit units)
    {
        m_pageLayout.setUnits(units);
        m_pageLayout.setMargins(margins);
        return m_pageLayout.margins() == margins && m_pageLayout.units() == units;
    }

    virtual QPageLayout pageLayout() const
    {
        return m_pageLayout;
    }

    static inline QPagedPaintDevicePrivate *get(QPagedPaintDevice *pd) { return pd->d; }

    QPageLayout m_pageLayout;

    // These are currently required to keep QPrinter functionality working in QTextDocument::print()
    int fromPage;
    int toPage;
    bool pageOrderAscending;
    bool printSelectionOnly;
};

QT_END_NAMESPACE

#endif
