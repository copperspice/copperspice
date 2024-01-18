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

#ifndef AVFMEDIARECORDERCONTROL_IOS_H
#define AVFMEDIARECORDERCONTROL_IOS_H

#include <avfmediaassetwriter.h>
#include <avfstoragelocation.h>
#include <avfcamerautility.h>
#include <qglobal.h>
#include <qmediarecordercontrol.h>
#include <qurl.h>

#include <qvideooutputorientationhandler_p.h>

#include <AVFoundation/AVFoundation.h>

class AVFCameraService;
class QString;
class QUrl;

class AVFMediaRecorderControlIOS : public QMediaRecorderControl
{
   CS_OBJECT(AVFMediaRecorderControlIOS)

 public:
   AVFMediaRecorderControlIOS(AVFCameraService *service, QObject *parent = nullptr);
   ~AVFMediaRecorderControlIOS();

   QUrl outputLocation() const override;
   bool setOutputLocation(const QUrl &location) override;

   QMediaRecorder::State state() const override;
   QMediaRecorder::Status status() const override;

   qint64 duration() const override;

   bool isMuted() const override;
   qreal volume() const override;

   void applySettings() override;
   void unapplySettings();

   CS_SLOT_1(Public, void setState(QMediaRecorder::State state)override)
   CS_SLOT_2(setState)

   CS_SLOT_1(Public, void setMuted(bool muted)override)
   CS_SLOT_2(setMuted)

   CS_SLOT_1(Public, void setVolume(qreal volume)override)
   CS_SLOT_2(setVolume)

 private:
   void stopWriter();

   AVFCameraService *m_service;
   AVFScopedPointer<AVFMediaAssetWriter> m_writer;

   QUrl m_outputLocation;
   AVFStorageLocation m_storageLocation;

   QMediaRecorder::State m_state;
   QMediaRecorder::Status m_lastStatus;

   NSDictionary *m_audioSettings;
   NSDictionary *m_videoSettings;
   QVideoOutputOrientationHandler m_orientationHandler;

   Q_INVOKABLE void assetWriterStarted();
   Q_INVOKABLE void assetWriterFinished();

   CS_SLOT_1(Private, void captureModeChanged(QCamera::CaptureModes newMode))
   CS_SLOT_2(captureModeChanged)

   CS_SLOT_1(Private, void cameraStatusChanged(QCamera::Status newStatus))
   CS_SLOT_2(cameraStatusChanged)
};

#endif
