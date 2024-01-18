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

#ifndef QPDFWRITER_H
#define QPDFWRITER_H

#include <qglobal.h>

#ifndef QT_NO_PDF

#include <qobject.h>
#include <qpagedpaintdevice.h>
#include <qpagelayout.h>

class QIODevice;
class QPdfEngine;

class Q_GUI_EXPORT QPdfWriter : public QObject, public QPagedPaintDevice
{
    GUI_CS_OBJECT_MULTIPLE(QPdfWriter, QObject)

public:
   explicit QPdfWriter(const QString &filename);
   explicit QPdfWriter(QIODevice *device);

   QPdfWriter(const QPdfWriter &) = delete;
   QPdfWriter &operator=(const QPdfWriter &) = delete;

   ~QPdfWriter();

   QString creator() const;
   void setCreator(const QString &creator);

   QString title() const;
   void setTitle(const QString &title);

   bool newPage() override;

   void setResolution(int resolution);
   int resolution() const;

   QPageLayout pageLayout() const override;
   bool setPageLayout(const QPageLayout &newPageLayout) override;

   bool setPageOrientation(QPageLayout::Orientation orientation) override;

   void setMargins(const QMarginsF &margins) override;

   bool setPageMargins(const QMarginsF &margins) override;
   bool setPageMargins(const QMarginsF &margins, QPageLayout::Unit units) override;

   void setPageSizeMM(const QSizeF &size) override;

   bool setPageSize(const QPageSize &size) override;
   void setPageSize(QPageSize::PageSizeId sizeId)  override;   // not sure about this method

 protected:
   QPaintEngine *paintEngine() const override;
   int metric(PaintDeviceMetric id) const override;

 private:
    std::unique_ptr<QPdfEngine> m_engine;
};

#endif // QT_NO_PDF

#endif
