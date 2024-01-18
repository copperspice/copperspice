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

#ifndef AVFMEDIARECORDERCONTROL_H
#define AVFMEDIARECORDERCONTROL_H

#include <avfstoragelocation.h>
#include <avfcamerautility.h>
#include <qurl.h>
#include <qmediarecordercontrol.h>

#import <AVFoundation/AVFoundation.h>

@class AVFMediaRecorderDelegate;

class AVFCameraSession;
class AVFCameraControl;
class AVFAudioInputSelectorControl;
class AVFCameraService;

class AVFMediaRecorderControl : public QMediaRecorderControl
{
   CS_OBJECT(AVFMediaRecorderControl)

 public:
   AVFMediaRecorderControl(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFMediaRecorderControl();

   QUrl outputLocation() const override;
   bool setOutputLocation(const QUrl &location) override;

   QMediaRecorder::State state() const override;
   QMediaRecorder::Status status() const override;

   qint64 duration() const override;

   bool isMuted() const override;
   qreal volume() const override;

   void applySettings() override;
   void unapplySettings();

   CS_SLOT_1(Public, void setState(QMediaRecorder::State state) override)
   CS_SLOT_2(setState)

   CS_SLOT_1(Public, void setMuted(bool muted) override)
   CS_SLOT_2(setMuted)

   CS_SLOT_1(Public, void setVolume(qreal volume) override)
   CS_SLOT_2(setVolume)

   CS_SLOT_1(Public, void handleRecordingStarted())
   CS_SLOT_2(handleRecordingStarted)

   CS_SLOT_1(Public, void handleRecordingFinished())
   CS_SLOT_2(handleRecordingFinished)

   CS_SLOT_1(Public, void handleRecordingFailed(const QString &message))
   CS_SLOT_2(handleRecordingFailed)

 private:
   CS_SLOT_1(Private, void setupSessionForCapture())
   CS_SLOT_2(setupSessionForCapture)
   CS_SLOT_1(Private, void updateStatus())
   CS_SLOT_2(updateStatus)

   AVFCameraService *m_service;
   AVFCameraControl *m_cameraControl;
   AVFAudioInputSelectorControl *m_audioInputControl;
   AVFCameraSession *m_session;

   bool m_connected;
   QUrl m_outputLocation;
   QMediaRecorder::State m_state;
   QMediaRecorder::Status m_lastStatus;

   bool m_recordingStarted;
   bool m_recordingFinished;

   bool m_muted;
   qreal m_volume;

   AVCaptureDeviceInput *m_audioInput;
   AVCaptureMovieFileOutput *m_movieOutput;
   AVFMediaRecorderDelegate *m_recorderDelagate;
   AVFStorageLocation m_storageLocation;

   AVFPSRange m_restoreFPS;
};

#endif
