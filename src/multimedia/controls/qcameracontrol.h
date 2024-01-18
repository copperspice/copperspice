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

#ifndef QCAMERACONTROL_H
#define QCAMERACONTROL_H

#include <qcamera.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraControl)

 public:
   enum PropertyChangeType {
      CaptureMode           = 1,
      ImageEncodingSettings = 2,
      VideoEncodingSettings = 3,
      Viewfinder            = 4,
      ViewfinderSettings    = 5
   };

   ~QCameraControl();

   virtual QCamera::State state() const = 0;
   virtual void setState(QCamera::State state) = 0;

   virtual QCamera::Status status() const = 0;

   virtual QCamera::CaptureModes captureMode() const = 0;
   virtual void setCaptureMode(QCamera::CaptureModes mode) = 0;
   virtual bool isCaptureModeSupported(QCamera::CaptureModes mode) const = 0;

   virtual bool canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const = 0;

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QCamera::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void statusChanged(QCamera::Status status))
   MULTI_CS_SIGNAL_2(statusChanged, status)

   MULTI_CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   MULTI_CS_SIGNAL_2(error, error, errorString)

   MULTI_CS_SIGNAL_1(Public, void captureModeChanged(QCamera::CaptureModes mode))
   MULTI_CS_SIGNAL_2(captureModeChanged, mode)

 protected:
   explicit QCameraControl(QObject *parent = nullptr);
};

#define QCameraControl_iid "com.copperspice.CS.cameraControl/1.0"
CS_DECLARE_INTERFACE(QCameraControl, QCameraControl_iid)

#endif

