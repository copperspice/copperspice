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

#ifndef QPDFWRITER_H
#define QPDFWRITER_H

#include <qglobal.h>

#ifndef QT_NO_PDF

#include <qobject.h>
#include <qpagedpaintdevice.h>
#include <qpagelayout.h>

class QIODevice;
class QPdfWriterPrivate;

class Q_GUI_EXPORT QPdfWriter : public QObject, public QPagedPaintDevice
{
    GUI_CS_OBJECT_MULTIPLE(QPdfWriter, QObject)

public:
    explicit QPdfWriter(const QString &filename);
    explicit QPdfWriter(QIODevice *device);
    ~QPdfWriter();

    QString title() const;
    void setTitle(const QString &title);

    QString creator() const;
    void setCreator(const QString &creator);

    bool newPage();

    void setResolution(int resolution);
    int resolution() const;

    using QPagedPaintDevice::setPageSize;

    void setPageSize(PageSize size);
    void setPageSizeMM(const QSizeF &size);

    void setMargins(const Margins &m);

protected:
   QPaintEngine *paintEngine() const;
   int metric(PaintDeviceMetric id) const;

   QScopedPointer<QPdfWriterPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QPdfWriter)
    Q_DECLARE_PRIVATE(QPdfWriter)
};

#endif // QT_NO_PDF

#endif
