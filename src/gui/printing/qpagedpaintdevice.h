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

#ifndef QPAGEDPAINTDEVICE_H
#define QPAGEDPAINTDEVICE_H

#include <qpaintdevice.h>
#include <qpagelayout.h>
#include <qpagesize.h>

class Q_GUI_EXPORT QPagedPaintDevice : public QPaintDevice
{
 public:
   QPagedPaintDevice();
   ~QPagedPaintDevice();

   virtual bool newPage() = 0;

   virtual bool setPageLayout(const QPageLayout &pageLayout);
   virtual QPageLayout pageLayout() const;

   virtual bool setPageOrientation(QPageLayout::Orientation orientation);

   virtual bool setPageSize(const QPageSize &size);
   virtual void setPageSize(QPageSize::PageSizeId sizeId);    // not sure about this method
   QPageSize::PageSizeId pageSize() const;

   virtual void setPageSizeMM(const QSizeF &size);
   QSizeF pageSizeMM() const;

   virtual bool setPageMargins(const QMarginsF &margins);
   virtual bool setPageMargins(const QMarginsF &margins, QPageLayout::Unit units);

   virtual void setMargins(const QMarginsF &margins);
   QMarginsF margins() const;

   int fromPage() const;
   int toPage() const;
   bool printSelectionOnly() const;

 protected:
   QPageLayout m_pageLayout;

   // required to keep QPrinter working in QTextDocument::print()
   int  m_fromPage;
   int  m_toPage;
   bool m_pageOrderAscending;
   bool m_printSelectionOnly;
};

#endif
