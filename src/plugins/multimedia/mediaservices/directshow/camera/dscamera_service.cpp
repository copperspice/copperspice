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

#include <qvariant.h>
#include <qdebug.h>

#include <dscamera_service.h>
#include <dscamera_control.h>
#include <dscamera_session.h>
#include <dscamera_viewfindersettingscontrol.h>
#include <dscamera_imageprocessingcontrol.h>
#include <dsimage_capturecontrol.h>
#include <dsvideo_renderer.h>
#include <dsvideo_devicecontrol.h>

DSCameraService::DSCameraService(QObject *parent)
   : QMediaService(parent), m_videoRenderer(nullptr)
{
   m_session      = new DSCameraSession(this);
   m_control      = new DSCameraControl(m_session);
   m_videoDevice  = new DSVideoDeviceControl(m_session);
   m_imageCapture = new DSImageCaptureControl(m_session);
   m_viewfinderSettings     = new DSCameraViewfinderSettingsControl(m_session);
   m_imageProcessingControl = new DSCameraImageProcessingControl(m_session);
}

DSCameraService::~DSCameraService()
{
   delete m_control;
   delete m_viewfinderSettings;
   delete m_imageProcessingControl;
   delete m_videoDevice;
   delete m_videoRenderer;
   delete m_imageCapture;
   delete m_session;
}

QMediaControl *DSCameraService::requestControl(const QString &name)
{
   if (name == QCameraControl_iid) {
      return m_control;
   }

   if (name == QCameraImageCaptureControl_iid) {
      return m_imageCapture;
   }

   if (name == QVideoRendererControl_iid) {
      if (! m_videoRenderer) {
         m_videoRenderer = new DSVideoRendererControl(m_session, this);
         return m_videoRenderer;
      }
   }

   if (name == QVideoDeviceSelectorControl_iid) {
      return m_videoDevice;
   }

   if (name ==  QCameraViewfinderSettingsControl2_iid) {
      return m_viewfinderSettings;
   }

   if (name == QCameraImageProcessingControl_iid) {
      return m_imageProcessingControl;
   }

   return nullptr;
}

void DSCameraService::releaseControl(QMediaControl *control)
{
   if (control == m_videoRenderer) {
      delete m_videoRenderer;
      m_videoRenderer = nullptr;
      return;
   }
}
