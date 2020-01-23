/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qpaintdevice.h>
#include <qdebug.h>

QPaintDevice::QPaintDevice()
{
   reserved = 0;
   painters = 0;
}

QPaintDevice::~QPaintDevice()
{
   if (paintingActive()) {
      qWarning("QPaintDevice: Can not destroy paint device which is being painted");
   }
}
void QPaintDevice::initPainter(QPainter *) const
{
}

/*!
    \internal
*/
QPaintDevice *QPaintDevice::redirected(QPoint *) const
{
   return 0;
}

/*!
    \internal
*/
QPainter *QPaintDevice::sharedPainter() const
{
   return 0;
}

Q_GUI_EXPORT int qt_paint_device_metric(const QPaintDevice *device, QPaintDevice::PaintDeviceMetric metric)
{
   return device->metric(metric);
}

int QPaintDevice::metric(PaintDeviceMetric m) const
{
   // Fallback: A subclass has not implemented PdmDevicePixelRatioScaled but might
   // have implemented PdmDevicePixelRatio.
   if (m == PdmDevicePixelRatioScaled) {
      return this->metric(PdmDevicePixelRatio) * devicePixelRatioFScale();
   }

   qWarning("QPaintDevice::metrics: Device has no metric information");

   if (m == PdmDpiX) {
      return 72;
   } else if (m == PdmDpiY) {
      return 72;
   } else if (m == PdmNumColors) {
      // FIXME: does this need to be a real value?
      return 256;
   } else if (m == PdmDevicePixelRatio) {
      return 1;
   } else {
      qDebug("Unrecognised metric %d!", m);
      return 0;
   }
}
