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

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#include <qwindowdefs.h>
#include <qrect.h>

class QPaintEngine;
class QPaintDevicePrivate;

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
      PdmPhysicalDpiY,
      PdmDevicePixelRatio,
      PdmDevicePixelRatioScaled
   };

   QPaintDevice(const QPaintDevice &) = delete;
   QPaintDevice &operator=(const QPaintDevice &) = delete;

   virtual ~QPaintDevice();

   virtual int devType() const;
   bool paintingActive() const;
   virtual QPaintEngine *paintEngine() const = 0;

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

   int devicePixelRatio() const {
      return metric(PdmDevicePixelRatio);
   }

   qreal devicePixelRatioF()  const {
      return metric(PdmDevicePixelRatioScaled) / devicePixelRatioFScale();
   }

   int colorCount() const {
      return metric(PdmNumColors);
   }

   int depth() const {
      return metric(PdmDepth);
   }

   static inline qreal devicePixelRatioFScale() {
      return 0x10000;
   }

 protected:
   QPaintDevice();
   virtual int metric(PaintDeviceMetric metric) const;
   virtual void initPainter(QPainter *painter) const;
   virtual QPaintDevice *redirected(QPoint *offset) const;
   virtual QPainter *sharedPainter() const;

   ushort painters;       // refcount

 private:
   QPaintDevicePrivate *reserved;

   friend class QPainter;
   friend class QPainterPrivate;
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

#endif
