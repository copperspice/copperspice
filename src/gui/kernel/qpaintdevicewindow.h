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

#ifndef QPAINTDEVICEWINDOW_H
#define QPAINTDEVICEWINDOW_H

#include <qpaintdevice.h>
#include <qwindow.h>

class QPaintDeviceWindowPrivate;
class QPaintEvent;

class Q_GUI_EXPORT QPaintDeviceWindow : public QWindow, public QPaintDevice
{
   GUI_CS_OBJECT(QPaintDeviceWindow)

 public:
   QPaintDeviceWindow(const QPaintDeviceWindow &) = delete;
   QPaintDeviceWindow &operator=(const QPaintDeviceWindow &) = delete;

   void update(const QRect &rect);
   void update(const QRegion &region);

   using QWindow::width;
   using QWindow::height;
   using QWindow::devicePixelRatio;

   GUI_CS_SLOT_1(Public, void update())
   GUI_CS_SLOT_OVERLOAD(update, ())

 protected:
   virtual void paintEvent(QPaintEvent *event);

   int metric(PaintDeviceMetric metric) const override;
   void exposeEvent(QExposeEvent *event) override;
   bool event(QEvent *event) override;

   QPaintDeviceWindow(QPaintDeviceWindowPrivate &dd, QWindow *parent);

 private:
   Q_DECLARE_PRIVATE(QPaintDeviceWindow)
   QPaintEngine *paintEngine() const override;
};

#endif
