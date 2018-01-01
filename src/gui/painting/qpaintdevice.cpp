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

#include <qpaintdevice.h>
#include <qlog.h>

extern void qt_painter_removePaintDevice(QPaintDevice *); //qpainter.cpp

QPaintDevice::QPaintDevice()
{
   painters = 0;
}

QPaintDevice::~QPaintDevice()
{
   if (paintingActive()) {
      qWarning("QPaintDevice: Can not destroy paint device which is being painted");
   }

   qt_painter_removePaintDevice(this);
}


#ifndef Q_WS_QPA
int QPaintDevice::metric(PaintDeviceMetric) const
{
   qWarning("QPaintDevice::metrics: Device has no metric information");
   return 0;
}
#endif

Q_GUI_EXPORT int qt_paint_device_metric(const QPaintDevice *device, QPaintDevice::PaintDeviceMetric metric)
{
   return device->metric(metric);
}

