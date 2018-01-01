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
#include <qpainter.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qapplication.h>

QT_BEGIN_NAMESPACE

extern void qt_painter_removePaintDevice(QPaintDevice *); //qpainter.cpp

int QPaintDevice::metric(PaintDeviceMetric m) const
{
   qWarning("QPaintDevice::metrics: Device has no metric information");
   if (m == PdmDpiX) {
      return 72;
   } else if (m == PdmDpiY) {
      return 72;
   } else if (m == PdmNumColors) {
      // FIXME: does this need to be a real value?
      return 256;
   } else {
      qDebug("Unrecognised metric %d!", m);
      return 0;
   }
}

QT_END_NAMESPACE
