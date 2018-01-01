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

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

#if defined(Q_WS_QWS)
class QWSDisplay;
#endif

class QPaintEngine;

class Q_GUI_EXPORT QPaintDevice                                // device for QPainter
{
 public:
   enum PaintDeviceMetric {
      PdmWidth = 1,
      PdmHeight,
      PdmWidthMM,
      PdmHeightMM,
      PdmNumColors,
      PdmDepth,
      PdmDpiX,
      PdmDpiY,
      PdmPhysicalDpiX,
      PdmPhysicalDpiY
   };

   virtual ~QPaintDevice();

   virtual int devType() const;
   bool paintingActive() const;
   virtual QPaintEngine *paintEngine() const = 0;

#if defined(Q_WS_QWS)
   static QWSDisplay *qwsDisplay();
#endif

#ifdef Q_OS_WIN
   virtual HDC getDC() const;
   virtual void releaseDC(HDC hdc) const;
#endif

   int width() const {
      return metric(PdmWidth);
   }
   int height() const {
      return metric(PdmHeight);
   }
   int widthMM() const {
      return metric(PdmWidthMM);
   }
   int heightMM() const {
      return metric(PdmHeightMM);
   }
   int logicalDpiX() const {
      return metric(PdmDpiX);
   }
   int logicalDpiY() const {
      return metric(PdmDpiY);
   }
   int physicalDpiX() const {
      return metric(PdmPhysicalDpiX);
   }
   int physicalDpiY() const {
      return metric(PdmPhysicalDpiY);
   }

#ifdef QT_DEPRECATED
   QT_DEPRECATED int numColors() const {
      return metric(PdmNumColors);
   }
#endif

   int colorCount() const {
      return metric(PdmNumColors);
   }

   int depth() const {
      return metric(PdmDepth);
   }

 protected:
   QPaintDevice();
   virtual int metric(PaintDeviceMetric metric) const;

   ushort painters;       // refcount

 private:
   Q_DISABLE_COPY(QPaintDevice)

   friend class QPainter;
   friend class QFontEngineMac;
   friend class QX11PaintEngine;
   friend Q_GUI_EXPORT int qt_paint_device_metric(const QPaintDevice *device, PaintDeviceMetric metric);
};

inline int QPaintDevice::devType() const
{
   return QInternal::UnknownDevice;
}

inline bool QPaintDevice::paintingActive() const
{
   return painters != 0;
}

QT_END_NAMESPACE

#endif // QPAINTDEVICE_H
