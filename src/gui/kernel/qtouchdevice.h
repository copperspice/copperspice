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

#ifndef QTOUCHDEVICE_H
#define QTOUCHDEVICE_H

#include <qobject.h>

class QDebug;
class QTouchDevicePrivate;

class Q_GUI_EXPORT QTouchDevice
{
   GUI_CS_GADGET(QTouchDevice)

   GUI_CS_ENUM(DeviceType)
   GUI_CS_ENUM(CapabilityFlag)

 public:
   enum DeviceType {
      TouchScreen,
      TouchPad
   };

   enum CapabilityFlag {
      Position = 0x0001,
      Area = 0x0002,
      Pressure = 0x0004,
      Velocity = 0x0008,
      RawPositions = 0x0010,
      NormalizedPosition = 0x0020,
      MouseEmulation = 0x0040
   };
   using Capabilities = QFlags<CapabilityFlag>;

   QTouchDevice();
   ~QTouchDevice();

   static QList<const QTouchDevice *> devices();

   QString name() const;
   DeviceType type() const;
   Capabilities capabilities() const;
   int maximumTouchPoints() const;

   void setName(const QString &name);
   void setType(DeviceType devType);
   void setCapabilities(Capabilities caps);
   void setMaximumTouchPoints(int max);

 private:
   QTouchDevicePrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTouchDevice::Capabilities)

Q_GUI_EXPORT QDebug operator<<(QDebug, const QTouchDevice *);

#endif
