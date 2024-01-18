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

#ifndef AVFCAMERACONTROL_H
#define AVFCAMERACONTROL_H

#include <qobject.h>
#include <qcameracontrol.h>

class AVFCameraSession;
class AVFCameraService;

class AVFCameraControl : public QCameraControl
{
   CS_OBJECT(AVFCameraControl)

 public:
   AVFCameraControl(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFCameraControl();

   QCamera::State state() const override;
   void setState(QCamera::State state) override;

   QCamera::Status status() const override;

   QCamera::CaptureModes captureMode() const override;
   void setCaptureMode(QCamera::CaptureModes) override;

   bool isCaptureModeSupported(QCamera::CaptureModes mode) const override;
   bool canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const override;

 private:
   CS_SLOT_1(Private, void updateStatus())
   CS_SLOT_2(updateStatus)

   AVFCameraSession *m_session;

   QCamera::State m_state;
   QCamera::Status m_lastStatus;
   QCamera::CaptureModes m_captureMode;
};

#endif
