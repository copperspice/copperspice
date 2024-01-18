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

#ifndef QCAMERACAPTUREDESTINATIONCONTROL_H
#define QCAMERACAPTUREDESTINATIONCONTROL_H

#include <qmediacontrol.h>
#include <qcameraimagecapture.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraCaptureDestinationControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraCaptureDestinationControl)

 public:
   ~QCameraCaptureDestinationControl();

   virtual bool isCaptureDestinationSupported(QCameraImageCapture::CaptureDestinations destination) const = 0;
   virtual QCameraImageCapture::CaptureDestinations captureDestination() const = 0;
   virtual void setCaptureDestination(QCameraImageCapture::CaptureDestinations destination) = 0;

   MULTI_CS_SIGNAL_1(Public, void captureDestinationChanged(QCameraImageCapture::CaptureDestinations destination))
   MULTI_CS_SIGNAL_2(captureDestinationChanged, destination)

 protected:
   explicit QCameraCaptureDestinationControl(QObject *parent = nullptr);
};

#define QCameraCaptureDestinationControl_iid "com.copperspice.CS..cameraCaptureDestinationControl/1.0"
CS_DECLARE_INTERFACE(QCameraCaptureDestinationControl, QCameraCaptureDestinationControl_iid)

#endif

