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

#ifndef QCAMERAIMAGECAPTURECONTROL_H
#define QCAMERAIMAGECAPTURECONTROL_H

#include <qmediacontrol.h>
#include <qcameraimagecapture.h>
#include <qstring.h>

class QImage;

class Q_MULTIMEDIA_EXPORT QCameraImageCaptureControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraImageCaptureControl)

 public:
   ~QCameraImageCaptureControl();

   virtual bool isReadyForCapture() const = 0;

   virtual QCameraImageCapture::DriveMode driveMode() const = 0;
   virtual void setDriveMode(QCameraImageCapture::DriveMode mode) = 0;

   virtual int capture(const QString &fileName) = 0;
   virtual void cancelCapture() = 0;

   MULTI_CS_SIGNAL_1(Public, void readyForCaptureChanged(bool ready))
   MULTI_CS_SIGNAL_2(readyForCaptureChanged, ready)

   MULTI_CS_SIGNAL_1(Public, void imageExposed(int id))
   MULTI_CS_SIGNAL_2(imageExposed, id)

   MULTI_CS_SIGNAL_1(Public, void imageCaptured(int id, const QImage &preview))
   MULTI_CS_SIGNAL_2(imageCaptured, id, preview)

   MULTI_CS_SIGNAL_1(Public, void imageMetadataAvailable(int id, const QString &key, const QVariant &value))
   MULTI_CS_SIGNAL_2(imageMetadataAvailable, id, key, value)

   MULTI_CS_SIGNAL_1(Public, void imageAvailable(int id, const QVideoFrame &buffer))
   MULTI_CS_SIGNAL_2(imageAvailable, id, buffer)

   MULTI_CS_SIGNAL_1(Public, void imageSaved(int id, const QString &fileName))
   MULTI_CS_SIGNAL_2(imageSaved, id, fileName)

   MULTI_CS_SIGNAL_1(Public, void error(int id, int error, const QString &errorString))
   MULTI_CS_SIGNAL_2(error, id, error, errorString)

 protected:
   explicit QCameraImageCaptureControl(QObject *parent = nullptr);
};

#define QCameraImageCaptureControl_iid "com.copperspice.CS.cameraImageCaptureControl/1.0"
CS_DECLARE_INTERFACE(QCameraImageCaptureControl, QCameraImageCaptureControl_iid)

#endif

