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

#include <qpagedpaintdevice.h>

QPagedPaintDevice::QPagedPaintDevice()
   : m_pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0, 0, 0, 0)),
     m_fromPage(0), m_toPage(0), m_pageOrderAscending(true), m_printSelectionOnly(false)
{
}

QPagedPaintDevice::~QPagedPaintDevice()
{
}

bool QPagedPaintDevice::setPageLayout(const QPageLayout &newPageLayout)
{
   m_pageLayout = newPageLayout;
   return m_pageLayout.isEquivalentTo(newPageLayout);
}

QPageLayout QPagedPaintDevice::pageLayout() const
{
   return m_pageLayout;
}

bool QPagedPaintDevice::setPageOrientation(QPageLayout::Orientation orientation)
{
   m_pageLayout.setOrientation(orientation);
   return m_pageLayout.orientation() == orientation;
}

bool QPagedPaintDevice::setPageSize(const QPageSize &size)
{
   m_pageLayout.setPageSize(size);
   return m_pageLayout.pageSize().isEquivalentTo(size);
}

void QPagedPaintDevice::setPageSize(QPageSize::PageSizeId sizeId)
{
   m_pageLayout.setPageSize(QPageSize(sizeId));
}

QPageSize::PageSizeId QPagedPaintDevice::pageSize() const
{
   // returns an emum
   return m_pageLayout.pageSize().id();
}

void QPagedPaintDevice::setPageSizeMM(const QSizeF &size)
{
   m_pageLayout.setPageSize(QPageSize(size, QPageSize::Millimeter));
}

QSizeF QPagedPaintDevice::pageSizeMM() const
{
   return m_pageLayout.pageSize().size(QPageSize::Millimeter);
}

bool QPagedPaintDevice::setPageMargins(const QMarginsF &margins) {
   return setPageMargins(margins, m_pageLayout.units());
}

bool QPagedPaintDevice::setPageMargins(const QMarginsF &margins, QPageLayout::Unit units) {
   m_pageLayout.setUnits(units);
   m_pageLayout.setMargins(margins);

   return m_pageLayout.margins() == margins && m_pageLayout.units() == units;
}

void QPagedPaintDevice::setMargins(const QMarginsF &margins)
{
   m_pageLayout.setUnits(QPageSize::Unit::Millimeter);
   m_pageLayout.setMargins(margins);
}

QMarginsF QPagedPaintDevice::margins() const
{
   return m_pageLayout.margins(QPageSize::Unit::Millimeter);
}

int QPagedPaintDevice::fromPage() const
{
   return m_fromPage;
}

int QPagedPaintDevice::toPage() const
{
   return m_toPage;
}

bool QPagedPaintDevice::printSelectionOnly() const
{
   return m_printSelectionOnly;
}