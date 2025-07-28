/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef AVFCAMERASERVICE_H
#define AVFCAMERASERVICE_H

#include <qmediaservice.h>
#include <qobject.h>
#include <qset.h>

class AVFAudioEncoderSettingsControl;
class AVFAudioInputSelectorControl;
class AVFCameraControl;
class AVFCameraDeviceControl;
class AVFCameraExposureControl;
class AVFCameraFlashControl;
class AVFCameraFocusControl;
class AVFCameraInfoControl;
class AVFCameraMetaDataControl;
class AVFCameraRendererControl;
class AVFCameraSession;
class AVFCameraViewfinderSettingsControl2;
class AVFCameraViewfinderSettingsControl;
class AVFCameraZoomControl;
class AVFImageCaptureControl;
class AVFImageEncoderControl;
class AVFMediaContainerControl;
class AVFMediaRecorderControl;
class AVFMediaRecorderControlIOS;
class AVFVideoEncoderSettingsControl;
class AVFVideoWidgetControl;
class AVFVideoWindowControl;
class QCameraControl;
class QMediaRecorderControl;

class AVFCameraService : public QMediaService
{
   CS_OBJECT(AVFCameraService)

 public:
   AVFCameraService(QObject *parent = nullptr);
   ~AVFCameraService();

   QMediaControl *requestControl(const QString &name) override;
   void releaseControl(QMediaControl *control) override;

   AVFCameraSession *session() const {
      return m_session;
   }
   AVFCameraControl *cameraControl() const {
      return m_cameraControl;
   }
   AVFCameraDeviceControl *videoDeviceControl() const {
      return m_videoDeviceControl;
   }
   AVFAudioInputSelectorControl *audioInputSelectorControl() const {
      return m_audioInputSelectorControl;
   }
   AVFCameraMetaDataControl *metaDataControl() const {
      return m_metaDataControl;
   }
   QMediaRecorderControl *recorderControl() const {
      return m_recorderControl;
   }
   AVFImageCaptureControl *imageCaptureControl() const {
      return m_imageCaptureControl;
   }
   AVFCameraFocusControl *cameraFocusControl() const {
      return m_cameraFocusControl;
   }
   AVFCameraExposureControl *cameraExposureControl() const {
      return m_cameraExposureControl;
   }
   AVFCameraZoomControl *cameraZoomControl() const {
      return m_cameraZoomControl;
   }
   AVFCameraRendererControl *videoOutput() const {
      return m_videoOutput;
   }
   AVFCameraViewfinderSettingsControl2 *viewfinderSettingsControl2() const {
      return m_viewfinderSettingsControl2;
   }
   AVFCameraViewfinderSettingsControl *viewfinderSettingsControl() const {
      return m_viewfinderSettingsControl;
   }
   AVFImageEncoderControl *imageEncoderControl() const {
      return m_imageEncoderControl;
   }
   AVFCameraFlashControl *flashControl() const {
      return m_flashControl;
   }
   AVFAudioEncoderSettingsControl *audioEncoderSettingsControl() const {
      return m_audioEncoderSettingsControl;
   }
   AVFVideoEncoderSettingsControl *videoEncoderSettingsControl() const {
      return m_videoEncoderSettingsControl;
   }
   AVFMediaContainerControl *mediaContainerControl() const {
      return m_mediaContainerControl;
   }

 private:
   AVFCameraSession *m_session;
   AVFCameraControl *m_cameraControl;
   AVFCameraInfoControl *m_cameraInfoControl;
   AVFCameraDeviceControl *m_videoDeviceControl;
   AVFAudioInputSelectorControl *m_audioInputSelectorControl;
   AVFCameraRendererControl *m_videoOutput;
   AVFCameraMetaDataControl *m_metaDataControl;
   QMediaRecorderControl *m_recorderControl;
   AVFImageCaptureControl *m_imageCaptureControl;
   AVFCameraFocusControl *m_cameraFocusControl;
   AVFCameraExposureControl *m_cameraExposureControl;
   AVFCameraZoomControl *m_cameraZoomControl;
   AVFCameraViewfinderSettingsControl2 *m_viewfinderSettingsControl2;
   AVFCameraViewfinderSettingsControl *m_viewfinderSettingsControl;
   AVFImageEncoderControl *m_imageEncoderControl;
   AVFCameraFlashControl *m_flashControl;
   AVFAudioEncoderSettingsControl *m_audioEncoderSettingsControl;
   AVFVideoEncoderSettingsControl *m_videoEncoderSettingsControl;
   AVFMediaContainerControl *m_mediaContainerControl;
};

#endif
