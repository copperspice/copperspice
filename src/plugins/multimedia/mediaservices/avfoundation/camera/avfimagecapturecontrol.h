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

#ifndef AVFIMAGECAPTURECONTROL_H
#define AVFIMAGECAPTURECONTROL_H

#include <qqueue.h>
#include <qsemaphore.h>
#include <qcameraimagecapturecontrol.h>
#include <avfcamerasession.h>
#include <avfstoragelocation.h>

#import <AVFoundation/AVFoundation.h>

class AVFImageCaptureControl : public QCameraImageCaptureControl
{
   CS_OBJECT(AVFImageCaptureControl)

 public:
   struct CaptureRequest {
      int captureId;
      QSemaphore *previewReady;
   };

   AVFImageCaptureControl(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFImageCaptureControl();

   bool isReadyForCapture() const override;

   QCameraImageCapture::DriveMode driveMode() const override {
      return QCameraImageCapture::SingleImageCapture;
   }

   void setDriveMode(QCameraImageCapture::DriveMode) override {
   }

   AVCaptureStillImageOutput *stillImageOutput() const {
      return m_stillImageOutput;
   }

   int capture(const QString &fileName) override;
   void cancelCapture() override;

 private :
   CS_SLOT_1(Private, void updateCaptureConnection())
   CS_SLOT_2(updateCaptureConnection)

   CS_SLOT_1(Private, void updateReadyStatus())
   CS_SLOT_2(updateReadyStatus)

   CS_SLOT_1(Private, void onNewViewfinderFrame(const QVideoFrame &frame))
   CS_SLOT_2(onNewViewfinderFrame)

   void makeCapturePreview(CaptureRequest request, const QVideoFrame &frame, int rotation);

   AVFCameraSession *m_session;
   AVFCameraControl *m_cameraControl;
   bool m_ready;
   int m_lastCaptureId;
   AVCaptureStillImageOutput *m_stillImageOutput;
   AVCaptureConnection *m_videoConnection;
   AVFStorageLocation m_storageLocation;

   QMutex m_requestsMutex;
   QQueue<CaptureRequest> m_captureRequests;
};

#endif
