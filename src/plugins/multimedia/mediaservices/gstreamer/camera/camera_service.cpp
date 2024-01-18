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

#include <camera_service.h>
#include <camera_session.h>
#include <camera_recorder.h>
#include <camera_container.h>
#include <camera_audioencoder.h>
#include <camera_videoencoder.h>
#include <camera_imageencoder.h>
#include <camera_control.h>
#include <camera_metadata.h>
#include <camera_infocontrol.h>

#ifdef HAVE_GST_PHOTOGRAPHY
#include <camera_exposure.h>
#include <camera_flash.h>
#include <camera_focus.h>
#include <camera_locks.h>
#endif

#include <camera_imagecapture.h>
#include <camera_imageprocessing.h>
#include <camera_capturebufferformat.h>
#include <camera_capturedestination.h>
#include <camera_viewfindersettings.h>
#include <camera_viewfindersettings2.h>
#include <camera_zoom.h>
#include <qdebug.h>
#include <qprocess.h>

#include <qgstreamerbushelper_p.h>
#include <qgstutils_p.h>
#include <qgstreameraudioinputselector_p.h>
#include <qgstreamervideoinputdevicecontrol_p.h>
#include <qgstreamervideowidget_p.h>
#include <qgstreamervideowindow_p.h>
#include <qgstreamervideorenderer_p.h>
#include <qmediaserviceprovider_p.h>

CameraBinService::CameraBinService(GstElementFactory *sourceFactory, QObject *parent)
   : QMediaService(parent), m_cameraInfoControl(nullptr), m_viewfinderSettingsControl(nullptr),
     m_viewfinderSettingsControl2(nullptr)
{
   m_captureSession      = nullptr;
   m_metaDataControl     = nullptr;
   m_audioInputSelector  = nullptr;
   m_videoInputDevice    = nullptr;
   m_videoOutput         = nullptr;
   m_videoRenderer       = nullptr;
   m_videoWindow         = nullptr;
   m_videoWidgetControl  = nullptr;
   m_imageCaptureControl = nullptr;

   m_captureSession      = new CameraBinSession(sourceFactory, this);
   m_videoInputDevice    = new QGstreamerVideoInputDeviceControl(sourceFactory, m_captureSession);
   m_imageCaptureControl = new CameraBinImageCapture(m_captureSession);

   connect(m_videoInputDevice, cs_mp_cast<const QString &>(&QVideoDeviceSelectorControl::selectedDeviceChanged),
            m_captureSession, &CameraBinSession::setDevice);

   if (m_videoInputDevice->deviceCount()) {
      m_captureSession->setDevice(m_videoInputDevice->deviceName(m_videoInputDevice->selectedDevice()));
   }

   m_videoRenderer = new QGstreamerVideoRenderer(this);
   m_videoWindow   = new QGstreamerVideoWindow(this);

   // if the GStreamer video sink is not available, don't provide the video window control since it will not work
   if (! m_videoWindow->videoSink()) {
      delete m_videoWindow;
      m_videoWindow = nullptr;
   }

   m_videoWidgetControl = new QGstreamerVideoWidgetControl(this);

   // if the GStreamer video sink is not available, do not provide the video widget control since
   // it will not work. QVideoWidget will fall back to QVideoRendererControl in that case.
   if (! m_videoWidgetControl->videoSink()) {
      delete m_videoWidgetControl;
      m_videoWidgetControl = nullptr;
   }

   m_audioInputSelector = new QGstreamerAudioInputSelector(this);
   connect(m_audioInputSelector, &QGstreamerAudioInputSelector::activeInputChanged,
            m_captureSession, &CameraBinSession::setCaptureDevice);

   if (m_captureSession && m_audioInputSelector->availableInputs().size() > 0) {
      m_captureSession->setCaptureDevice(m_audioInputSelector->defaultInput());
   }

   m_metaDataControl = new CameraBinMetaData(this);
   connect(m_metaDataControl, &CameraBinMetaData::metaDataChanged, m_captureSession, &CameraBinSession::setMetaData);
}

CameraBinService::~CameraBinService()
{
}

QMediaControl *CameraBinService::requestControl(const QString &name)
{
   if (! m_captureSession) {
      return nullptr;
   }

   if (! m_videoOutput) {
      if (name == QVideoRendererControl_iid) {
         m_videoOutput = m_videoRenderer;

      } else if (name == QVideoWindowControl_iid) {
         m_videoOutput = m_videoWindow;

      } else if (name == QVideoWidgetControl_iid) {
         m_videoOutput = m_videoWidgetControl;
      }

      if (m_videoOutput) {
         m_captureSession->setViewfinder(m_videoOutput);
         return m_videoOutput;
      }
   }

   if (name == QAudioInputSelectorControl_iid) {
      return m_audioInputSelector;
   }

   if (name == QVideoDeviceSelectorControl_iid) {
      return m_videoInputDevice;
   }

   if (name == QMediaRecorderControl_iid) {
      return m_captureSession->recorderControl();
   }

   if (name == QAudioEncoderSettingsControl_iid) {
      return m_captureSession->audioEncodeControl();
   }

   if (name == QVideoEncoderSettingsControl_iid) {
      return m_captureSession->videoEncodeControl();
   }

   if (name == QImageEncoderControl_iid) {
      return m_captureSession->imageEncodeControl();
   }

   if (name == QMediaContainerControl_iid) {
      return m_captureSession->mediaContainerControl();
   }

   if (name == QCameraControl_iid) {
      return m_captureSession->cameraControl();
   }

   if (name == QMetaDataWriterControl_iid) {
      return m_metaDataControl;
   }

   if (name == QCameraImageCaptureControl_iid) {
      return m_imageCaptureControl;
   }

#ifdef HAVE_GST_PHOTOGRAPHY
   if (name == QCameraExposureControl_iid) {
      return m_captureSession->cameraExposureControl();
   }

   if (name == QCameraFlashControl_iid) {
      return m_captureSession->cameraFlashControl();
   }

   if (name ==  QCameraFocusControl_iid) {
      return m_captureSession->cameraFocusControl();
   }

   if (name == QCameraLocksControl_iid) {
      return m_captureSession->cameraLocksControl();
   }
#endif

   if (name == QCameraZoomControl_iid) {
      return m_captureSession->cameraZoomControl();
   }

   if (name == QCameraImageProcessingControl_iid) {
      return m_captureSession->imageProcessingControl();
   }

   if (name == QCameraCaptureDestinationControl_iid) {
      return m_captureSession->captureDestinationControl();
   }

   if (name == QCameraCaptureBufferFormatControl_iid) {
      return m_captureSession->captureBufferFormatControl();
   }

   if (name == QCameraViewfinderSettingsControl_iid) {
      if (! m_viewfinderSettingsControl) {
         m_viewfinderSettingsControl = new CameraBinViewfinderSettings(m_captureSession);
      }
      return m_viewfinderSettingsControl;
   }

   if (name == QCameraViewfinderSettingsControl2_iid) {
      if (! m_viewfinderSettingsControl2) {
         m_viewfinderSettingsControl2 = new CameraBinViewfinderSettings2(m_captureSession);
      }
      return m_viewfinderSettingsControl2;
   }

   if (name == QCameraInfoControl_iid){
      if (!m_cameraInfoControl) {
         m_cameraInfoControl = new CameraBinInfoControl(m_captureSession->sourceFactory(), this);
      }
      return m_cameraInfoControl;
   }

   return nullptr;
}

void CameraBinService::releaseControl(QMediaControl *control)
{
   if (control && control == m_videoOutput) {
      m_videoOutput = nullptr;
      m_captureSession->setViewfinder(nullptr);
   }
}

bool CameraBinService::isCameraBinAvailable()
{
   GstElementFactory *factory = gst_element_factory_find(QT_GSTREAMER_CAMERABIN_ELEMENT_NAME);
   if (factory) {
      gst_object_unref(GST_OBJECT(factory));
      return true;
   }

   return false;
}

