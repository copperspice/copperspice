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

#ifndef AVFCAMERADEVICECONTROL_H
#define AVFCAMERADEVICECONTROL_H

#include <qvideodeviceselectorcontrol.h>
#include <qstringlist.h>

#import <AVFoundation/AVFoundation.h>

class AVFCameraSession;
class AVFCameraService;

class AVFCameraDeviceControl : public QVideoDeviceSelectorControl
{
   CS_OBJECT(AVFCameraDeviceControl)

 public:
   AVFCameraDeviceControl(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFCameraDeviceControl();

   int deviceCount() const;

   QString deviceName(int index) const;
   QString deviceDescription(int index) const;

   int defaultDevice() const;
   int selectedDevice() const;

   CS_SLOT_1(Public, void setSelectedDevice(int index))
   CS_SLOT_2(setSelectedDevice)

   //device changed since the last createCaptureDevice()
   bool isDirty() const {
      return m_dirty;
   }
   AVCaptureDevice *createCaptureDevice();

 private:
   AVFCameraService *m_service;

   int m_selectedDevice;
   bool m_dirty;
};

#endif
