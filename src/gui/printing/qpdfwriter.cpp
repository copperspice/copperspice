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

#include <qpdfwriter.h>

#ifndef QT_NO_PDF

#include <qfile.h>
#include <qpagedpaintdevice.h>

#include <qpdf_p.h>

QPdfWriter::QPdfWriter(const QString &filename)
    : QObject(), m_engine(new QPdfEngine())
{
   m_engine->setOutputFilename(filename);

   // set QPagedPaintDevice layout to match the current paint engine layout
   QPagedPaintDevice::setPageLayout(m_engine->pageLayout());
}

QPdfWriter::QPdfWriter(QIODevice *device)
    : QObject(), m_engine(new QPdfEngine())
{
   m_engine->d_func()->outDevice = device;

   // set QPagedPaintDevice layout to match the current paint engine layout
   QPagedPaintDevice::setPageLayout(m_engine->pageLayout());
}

QPdfWriter::~QPdfWriter()
{
}

QString QPdfWriter::creator() const
{
   return m_engine->d_func()->creator;
}

void QPdfWriter::setCreator(const QString &creator)
{
   m_engine->d_func()->creator = creator;
}

QString QPdfWriter::title() const
{
   return m_engine->d_func()->title;
}

void QPdfWriter::setTitle(const QString &title)
{
   m_engine->d_func()->title = title;
}

QPaintEngine *QPdfWriter::paintEngine() const
{
   return m_engine.get();
}

int QPdfWriter::resolution() const
{
    return m_engine->resolution();
}

void QPdfWriter::setResolution(int resolution)
{
   if (resolution > 0) {
      m_engine->setResolution(resolution);
   }
}

int QPdfWriter::metric(PaintDeviceMetric id) const
{
   return m_engine->metric(id);
}

bool QPdfWriter::newPage()
{
   return m_engine->newPage();
}

QPageLayout QPdfWriter::pageLayout() const
{
  return m_engine->pageLayout();
}

bool QPdfWriter::setPageLayout(const QPageLayout &newPageLayout)
{
  m_engine->setPageLayout(newPageLayout);

  m_pageLayout = m_engine->pageLayout();
  return m_pageLayout.isEquivalentTo(newPageLayout);
}

bool QPdfWriter::setPageOrientation(QPageLayout::Orientation orientation)
{
  m_engine->setPageOrientation(orientation);

  m_pageLayout = m_engine->pageLayout();
  return m_pageLayout.orientation() == orientation;
}

void QPdfWriter::setMargins(const QMarginsF &margins)
{
   // forward
   setPageMargins(margins, QPageSize::Unit::Millimeter);
}

bool QPdfWriter::setPageMargins(const QMarginsF &margins)
{
  // forward
  return setPageMargins(margins, pageLayout().units());
}

bool QPdfWriter::setPageMargins(const QMarginsF &margins, QPageLayout::Unit units)
{
  m_engine->setPageMargins(margins, units);

  m_pageLayout = m_engine->pageLayout();
  return m_pageLayout.margins() == margins && m_pageLayout.units() == units;
}

void QPdfWriter::setPageSize(QPageSize::PageSizeId sizeId)
{
   // forward
   setPageSize(QPageSize(sizeId));
}

void QPdfWriter::setPageSizeMM(const QSizeF &size)
{
   // forward
   setPageSize(QPageSize(size, QPageSize::Millimeter));
}

bool QPdfWriter::setPageSize(const QPageSize &pageSize)
{
  m_engine->setPageSize(pageSize);

  m_pageLayout = m_engine->pageLayout();
  return m_pageLayout.pageSize().isEquivalentTo(pageSize);
}

#endif // QT_NO_PDF
