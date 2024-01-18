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

#ifndef QVIDEODEVICESELECTORCONTROL_H
#define QVIDEODEVICESELECTORCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>

class Q_MULTIMEDIA_EXPORT QVideoDeviceSelectorControl : public QMediaControl
{
   MULTI_CS_OBJECT(QVideoDeviceSelectorControl)

 public:
   virtual ~QVideoDeviceSelectorControl();

   virtual int deviceCount() const = 0;

   virtual QString deviceName(int index) const = 0;
   virtual QString deviceDescription(int index) const = 0;

   virtual int defaultDevice() const = 0;
   virtual int selectedDevice() const = 0;

   MULTI_CS_SLOT_1(Public, virtual void setSelectedDevice(int index) = 0)
   MULTI_CS_SLOT_2(setSelectedDevice)

   MULTI_CS_SIGNAL_1(Public, void selectedDeviceChanged(int index))
   MULTI_CS_SIGNAL_OVERLOAD(selectedDeviceChanged, (int), index)

   MULTI_CS_SIGNAL_1(Public, void selectedDeviceChanged(const QString &deviceName))
   MULTI_CS_SIGNAL_OVERLOAD(selectedDeviceChanged, (const QString &), deviceName)

   MULTI_CS_SIGNAL_1(Public, void devicesChanged())
   MULTI_CS_SIGNAL_2(devicesChanged)

 protected:
   explicit QVideoDeviceSelectorControl(QObject *parent = nullptr);
};

#define QVideoDeviceSelectorControl_iid "com.copperspice.CS.videoDeviceSelectorControl/1.0"
CS_DECLARE_INTERFACE(QVideoDeviceSelectorControl, QVideoDeviceSelectorControl_iid)

#endif
