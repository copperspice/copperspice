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

#ifndef QRASTERWINDOW_H
#define QRASTERWINDOW_H

#include <qpaintdevicewindow.h>

class QRasterWindowPrivate;

class Q_GUI_EXPORT QRasterWindow : public QPaintDeviceWindow
{
   GUI_CS_OBJECT(QRasterWindow)

 public:
   explicit QRasterWindow(QWindow *parent = nullptr);

   QRasterWindow(const QRasterWindow &) = delete;
   QRasterWindow &operator=(const QRasterWindow &) = delete;

 protected:
   int metric(PaintDeviceMetric metric) const override;
   QPaintDevice *redirected(QPoint *) const override;

 private:
   Q_DECLARE_PRIVATE(QRasterWindow)

};

#endif
