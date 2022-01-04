/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#ifndef DSVIDEODEVICECONTROL_H
#define DSVIDEODEVICECONTROL_H

#include <qvideodeviceselectorcontrol.h>
#include <qstringlist.h>

class DSCameraSession;

using DSVideoDeviceInfo = QPair<QString, QString>;

class DSVideoDeviceControl : public QVideoDeviceSelectorControl
{
   CS_OBJECT(DSVideoDeviceControl)

 public:
   DSVideoDeviceControl(QObject *parent = nullptr);

   int deviceCount() const;
   QString deviceName(int index) const;
   QString deviceDescription(int index) const;
   int defaultDevice() const;
   int selectedDevice() const;

   static const QList<DSVideoDeviceInfo> &availableDevices();

   CS_SLOT_1(Public, void setSelectedDevice(int index))
   CS_SLOT_2(setSelectedDevice)

 private:
   static void updateDevices();

   DSCameraSession *m_session;
   int selected;
};

#endif
