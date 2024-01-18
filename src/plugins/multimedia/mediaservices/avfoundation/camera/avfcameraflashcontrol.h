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

#ifndef AVFCAMERAFLASHCONTROL_H
#define AVFCAMERAFLASHCONTROL_H

#include <qcameraflashcontrol.h>
#include <qcamera.h>
#include <qlist.h>

class AVFCameraService;
class AVFCameraSession;

class AVFCameraFlashControl : public QCameraFlashControl
{
   CS_OBJECT(AVFCameraFlashControl)

 public:
   AVFCameraFlashControl(AVFCameraService *service);

   QCameraExposure::FlashModes flashMode() const override;
   void setFlashMode(QCameraExposure::FlashModes mode) override;
   bool isFlashModeSupported(QCameraExposure::FlashModes mode) const override;
   bool isFlashReady() const override;

 private:
   CS_SLOT_1(Private, void cameraStateChanged(QCamera::State newState))
   CS_SLOT_2(cameraStateChanged)

   bool applyFlashSettings();

   AVFCameraService *m_service;
   AVFCameraSession *m_session;

   // Set of bits:
   QCameraExposure::FlashModes m_supportedModes;
   // Only one bit set actually:
   QCameraExposure::FlashModes m_flashMode;
};

#endif

