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

#include <qdebug.h>

#include <dscamera_control.h>
#include <dscamera_service.h>
#include <dscamera_session.h>

DSCameraControl::DSCameraControl(QObject *parent)
   : QCameraControl(parent), m_state(QCamera::UnloadedState), m_captureMode(QCamera::CaptureStillImage)
{
   m_session = dynamic_cast<DSCameraSession *>(parent);
   connect(m_session, SIGNAL(statusChanged(QCamera::Status)), this, SLOT(statusChanged(QCamera::Status)));
}

DSCameraControl::~DSCameraControl()
{
}

void DSCameraControl::setState(QCamera::State state)
{
   if (m_state == state) {
      return;
   }

   bool succeeded = false;

   switch (state) {
      case QCamera::UnloadedState:
         succeeded = m_session->unload();
         break;

      case QCamera::LoadedState:
      case QCamera::ActiveState:
         if (m_state == QCamera::UnloadedState && !m_session->load()) {
            return;
         }

         if (state == QCamera::ActiveState) {
            succeeded = m_session->startPreview();
         } else {
            succeeded = m_session->stopPreview();
         }

         break;
   }

   if (succeeded) {
      m_state = state;
      emit stateChanged(m_state);
   }
}

bool DSCameraControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
   bool bCaptureSupported = false;

   switch (mode) {
      case QCamera::CaptureStillImage:
         bCaptureSupported = true;
         break;

      case QCamera::CaptureVideo:
         bCaptureSupported = false;
         break;
   }

   return bCaptureSupported;
}

void DSCameraControl::setCaptureMode(QCamera::CaptureModes mode)
{
   if (m_captureMode != mode && isCaptureModeSupported(mode)) {
      m_captureMode = mode;
      emit captureModeChanged(mode);
   }
}

QCamera::Status DSCameraControl::status() const
{
   return m_session->status();
}

