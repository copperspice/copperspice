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

#include <qpaintdevice.h>
#include <qdebug.h>

QPaintDevice::QPaintDevice()
{
   reserved = nullptr;
   painters = 0;
}

QPaintDevice::~QPaintDevice()
{
   if (paintingActive()) {
      qWarning("QPaintDevice::~QPaintDevice() Unable to destroy a paint device while it is being painted");
   }
}

void QPaintDevice::initPainter(QPainter *) const
{
}

// internal
QPaintDevice *QPaintDevice::redirected(QPoint *) const
{
   return nullptr;
}

// internal
QPainter *QPaintDevice::sharedPainter() const
{
   return nullptr;
}

Q_GUI_EXPORT int qt_paint_device_metric(const QPaintDevice *device, QPaintDevice::PaintDeviceMetric metric)
{
   return device->metric(metric);
}

int QPaintDevice::metric(PaintDeviceMetric m) const
{
   // Fallback: subclass has not implemented
   // PdmDevicePixelRatioScaled but might have implemented PdmDevicePixelRatio

   if (m == PdmDevicePixelRatioScaled) {
      return this->metric(PdmDevicePixelRatio) * devicePixelRatioFScale();
   }

   qWarning("QPaintDevice::metric() Device has no metric information");

   if (m == PdmDpiX) {
      return 72;

   } else if (m == PdmDpiY) {
      return 72;

   } else if (m == PdmNumColors) {
      // does this need to be a real value?
      return 256;

   } else if (m == PdmDevicePixelRatio) {
      return 1;

   } else {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("Unrecognised metric %d", m);
#endif
      return 0;
   }
}
