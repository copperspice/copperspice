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

#ifndef AVFCAMERASESSION_H
#define AVFCAMERASESSION_H

#include <qmutex.h>
#include <qcamera.h>
#include <qvideoframe.h>

#import <AVFoundation/AVFoundation.h>

@class AVFCameraSessionObserver;

class AVFCameraControl;
class AVFCameraService;
class AVFCameraRendererControl;
class AVFMediaVideoProbeControl;

struct AVFCameraInfo {
   AVFCameraInfo()
      : position(QCamera::UnspecifiedPosition), orientation(0) {
   }

   QString deviceId;
   QString description;
   QCamera::Position position;
   int orientation;
};

class AVFCameraSession : public QObject
{
   CS_OBJECT(AVFCameraSession)

 public:
   AVFCameraSession(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFCameraSession();

   static int defaultCameraIndex();
   static const QList<AVFCameraInfo> &availableCameraDevices();
   static AVFCameraInfo cameraDeviceInfo(const QString &device);

   AVFCameraInfo activeCameraInfo() const {
      return m_activeCameraInfo;
   }

   void setVideoOutput(AVFCameraRendererControl *output);
   AVCaptureSession *captureSession() const {
      return m_captureSession;
   }
   AVCaptureDevice *videoCaptureDevice() const;

   QCamera::State state() const;
   QCamera::State requestedState() const {
      return m_state;
   }
   bool isActive() const {
      return m_active;
   }

   void addProbe(AVFMediaVideoProbeControl *probe);
   void removeProbe(AVFMediaVideoProbeControl *probe);
   FourCharCode defaultCodec();

   AVCaptureDeviceInput *videoInput() const {
      return m_videoInput;
   }

   CS_SLOT_1(Public, void setState(QCamera::State state))
   CS_SLOT_2(setState)

   CS_SLOT_1(Public, void processRuntimeError())
   CS_SLOT_2(processRuntimeError)

   CS_SLOT_1(Public, void processSessionStarted())
   CS_SLOT_2(processSessionStarted)

   CS_SLOT_1(Public, void processSessionStopped())
   CS_SLOT_2(processSessionStopped)

   CS_SLOT_1(Public, void onCaptureModeChanged(QCamera::CaptureModes mode))
   CS_SLOT_2(onCaptureModeChanged)

   CS_SLOT_1(Public, void onCameraFrameFetched(const QVideoFrame &frame))
   CS_SLOT_2(onCameraFrameFetched)

   CS_SIGNAL_1(Public, void readyToConfigureConnections())
   CS_SIGNAL_2(readyToConfigureConnections)

   CS_SIGNAL_1(Public, void stateChanged(QCamera::State newState))
   CS_SIGNAL_2(stateChanged, newState)

   CS_SIGNAL_1(Public, void activeChanged(bool isActive))
   CS_SIGNAL_2(activeChanged, isActive)

   CS_SIGNAL_1(Public, void newViewfinderFrame(const QVideoFrame &frame))
   CS_SIGNAL_2(newViewfinderFrame, frame)

   CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   CS_SIGNAL_2(error, error, errorString)

 private:
   static void updateCameraDevices();
   void attachVideoInputDevice();
   bool applyImageEncoderSettings();
   bool applyViewfinderSettings();

   static int m_defaultCameraIndex;
   static QList<AVFCameraInfo> m_cameraDevices;
   AVFCameraInfo m_activeCameraInfo;

   AVFCameraService *m_service;
   AVFCameraRendererControl *m_videoOutput;

   QCamera::State m_state;
   bool m_active;

   AVCaptureSession *m_captureSession;
   AVCaptureDeviceInput *m_videoInput;
   AVFCameraSessionObserver *m_observer;

   QSet<AVFMediaVideoProbeControl *> m_videoProbes;
   QMutex m_videoProbesMutex;

   FourCharCode m_defaultCodec;
};

#endif
