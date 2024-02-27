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

#ifndef DSCAMERACONTROL_H
#define DSCAMERACONTROL_H

#include <qobject.h>
#include <qcameracontrol.h>

class DSCameraService;
class DSCameraSession;

class DSCameraControl : public QCameraControl
{
   CS_OBJECT(DSCameraControl)

 public:
   DSCameraControl(QObject *parent = nullptr);
   ~DSCameraControl();

   QCamera::State state() const override {
      return m_state;
   }

   QCamera::CaptureModes captureMode() const override {
      return m_captureMode;
   }
   void setCaptureMode(QCamera::CaptureModes mode) override;

   void setState(QCamera::State state) override;

   QCamera::Status status() const override;
   bool isCaptureModeSupported(QCamera::CaptureModes mode) const override;

   bool canChangeProperty(PropertyChangeType, QCamera::Status) const override {
      return false;
   }

 private:
   DSCameraSession *m_session;
   DSCameraService *m_service;
   QCamera::State m_state;
   QCamera::CaptureModes m_captureMode;
};

#endif


