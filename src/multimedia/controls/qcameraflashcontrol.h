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

#ifndef QCAMERAFLASHCONTROL_H
#define QCAMERAFLASHCONTROL_H

#include <qcameraexposure.h>
#include <qcamera.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraFlashControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraFlashControl)

 public:
   ~QCameraFlashControl();

   virtual QCameraExposure::FlashModes flashMode() const = 0;
   virtual void setFlashMode(QCameraExposure::FlashModes mode) = 0;
   virtual bool isFlashModeSupported(QCameraExposure::FlashModes mode) const = 0;

   virtual bool isFlashReady() const = 0;

   MULTI_CS_SIGNAL_1(Public, void flashReady(bool ready))
   MULTI_CS_SIGNAL_2(flashReady, ready)

 protected:
   explicit QCameraFlashControl(QObject *parent = nullptr);
};

#define QCameraFlashControl_iid "com.copperspice.CS.cameraFlashControl/1.0"
CS_DECLARE_INTERFACE(QCameraFlashControl, QCameraFlashControl_iid)

#endif

