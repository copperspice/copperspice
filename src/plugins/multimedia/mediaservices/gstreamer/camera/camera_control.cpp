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

#include <camera_control.h>

#include <qdebug.h>
#include <qfile.h>
#include <qmetaobject.h>

#include <camera_container.h>
#include <camera_audioencoder.h>
#include <camera_videoencoder.h>
#include <camera_imageencoder.h>
#include <camera_resourcepolicy.h>

// #define CAMEABIN_DEBUG 1
#define ENUM_NAME(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(e)).valueToKey((v)))

CameraBinControl::CameraBinControl(CameraBinSession *session)
   : QCameraControl(session), m_session(session), m_state(QCamera::UnloadedState), m_reloadPending(false)
{
   connect(m_session, &CameraBinSession::statusChanged,     this, &CameraBinControl::statusChanged);
   connect(m_session, &CameraBinSession::viewfinderChanged, this, &CameraBinControl::reloadLater);
   connect(m_session, &CameraBinSession::readyChanged,      this, &CameraBinControl::reloadLater);
   connect(m_session, &CameraBinSession::error,             this, &CameraBinControl::handleCameraError);
   connect(m_session, &CameraBinSession::busyChanged,       this, &CameraBinControl::handleBusyChanged);

   m_resourcePolicy = new CamerabinResourcePolicy(this);

   connect(m_resourcePolicy, &CamerabinResourcePolicy::resourcesGranted, this, &CameraBinControl::handleResourcesGranted);
   connect(m_resourcePolicy, &CamerabinResourcePolicy::resourcesDenied,  this, &CameraBinControl::handleResourcesLost);
   connect(m_resourcePolicy, &CamerabinResourcePolicy::resourcesLost,    this, &CameraBinControl::handleResourcesLost);
}

CameraBinControl::~CameraBinControl()
{
}

QCamera::CaptureModes CameraBinControl::captureMode() const
{
   return m_session->captureMode();
}

void CameraBinControl::setCaptureMode(QCamera::CaptureModes mode)
{
   if (m_session->captureMode() != mode) {
      m_session->setCaptureMode(mode);

      if (m_state == QCamera::ActiveState) {
         m_resourcePolicy->setResourceSet(
            captureMode() == QCamera::CaptureStillImage ?
            CamerabinResourcePolicy::ImageCaptureResources :
            CamerabinResourcePolicy::VideoCaptureResources);
      }
      emit captureModeChanged(mode);
   }
}

bool CameraBinControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
   return mode == QCamera::CaptureStillImage || mode == QCamera::CaptureVideo;
}

void CameraBinControl::setState(QCamera::State state)
{
#ifdef CAMEABIN_DEBUG
   qDebug() << Q_FUNC_INFO << ENUM_NAME(QCamera, "State", state);
#endif
   if (m_state != state) {
      m_state = state;

      //special case for stopping the camera while it's busy,
      //it should be delayed until the camera is idle
      if (state == QCamera::LoadedState &&
            m_session->status() == QCamera::ActiveStatus &&
            m_session->isBusy()) {
#ifdef CAMEABIN_DEBUG
         qDebug() << Q_FUNC_INFO << "Camera is busy, QCamera::stop() is delayed";
#endif
         emit stateChanged(m_state);
         return;
      }

      CamerabinResourcePolicy::ResourceSet resourceSet = CamerabinResourcePolicy::NoResources;
      switch (state) {
         case QCamera::UnloadedState:
            resourceSet = CamerabinResourcePolicy::NoResources;
            break;

         case QCamera::LoadedState:
            resourceSet = CamerabinResourcePolicy::LoadedResources;
            break;

         case QCamera::ActiveState:
            resourceSet = captureMode() == QCamera::CaptureStillImage ?
                          CamerabinResourcePolicy::ImageCaptureResources :
                          CamerabinResourcePolicy::VideoCaptureResources;
            break;
      }

      m_resourcePolicy->setResourceSet(resourceSet);

      if (m_resourcePolicy->isResourcesGranted()) {
         //postpone changing to Active if the session is nor ready yet
         if (state == QCamera::ActiveState) {
            if (m_session->isReady()) {
               m_session->setState(state);
            } else {
#ifdef CAMEABIN_DEBUG
               qDebug() << "Camera session is not ready yet, postpone activating";
#endif
            }
         } else {
            m_session->setState(state);
         }
      }

      emit stateChanged(m_state);
   }
}

QCamera::State CameraBinControl::state() const
{
   return m_state;
}

QCamera::Status CameraBinControl::status() const
{
   return m_session->status();
}

void CameraBinControl::reloadLater()
{
#ifdef CAMEABIN_DEBUG
   qDebug() << "CameraBinControl: reload pipeline requested" << ENUM_NAME(QCamera, "State", m_state);
#endif
   if (!m_reloadPending && m_state == QCamera::ActiveState) {
      m_reloadPending = true;

      if (!m_session->isBusy()) {
         m_session->setState(QCamera::LoadedState);
         QMetaObject::invokeMethod(this, "delayedReload", Qt::QueuedConnection);
      }
   }
}

void CameraBinControl::handleResourcesLost()
{
#ifdef CAMEABIN_DEBUG
   qDebug() << Q_FUNC_INFO << ENUM_NAME(QCamera, "State", m_state);
#endif
   m_session->setState(QCamera::UnloadedState);
}

void CameraBinControl::handleResourcesGranted()
{
#ifdef CAMEABIN_DEBUG
   qDebug() << Q_FUNC_INFO << ENUM_NAME(QCamera, "State", m_state);
#endif

   //camera will be started soon by delayedReload()
   if (m_reloadPending && m_state == QCamera::ActiveState) {
      return;
   }

   if (m_state == QCamera::ActiveState && m_session->isReady()) {
      m_session->setState(QCamera::ActiveState);
   } else if (m_state == QCamera::LoadedState) {
      m_session->setState(QCamera::LoadedState);
   }
}

void CameraBinControl::handleBusyChanged(bool busy)
{
   if (!busy && m_session->status() == QCamera::ActiveStatus) {
      if (m_state == QCamera::LoadedState) {
         //handle delayed stop() because of busy camera
         m_resourcePolicy->setResourceSet(CamerabinResourcePolicy::LoadedResources);
         m_session->setState(QCamera::LoadedState);
      } else if (m_state == QCamera::ActiveState && m_reloadPending) {
         //handle delayed reload because of busy camera
         m_session->setState(QCamera::LoadedState);
         QMetaObject::invokeMethod(this, "delayedReload", Qt::QueuedConnection);
      }
   }
}

void CameraBinControl::handleCameraError(int errorCode, const QString &errorString)
{
   emit error(errorCode, errorString);
   setState(QCamera::UnloadedState);
}

void CameraBinControl::delayedReload()
{
#ifdef CAMEABIN_DEBUG
   qDebug() << "CameraBinControl: reload pipeline";
#endif
   if (m_reloadPending) {
      m_reloadPending = false;
      if (m_state == QCamera::ActiveState &&
            m_session->isReady() &&
            m_resourcePolicy->isResourcesGranted()) {
         m_session->setState(QCamera::ActiveState);
      }
   }
}

bool CameraBinControl::canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const
{
   (void) status;

   switch (changeType) {
      case QCameraControl::Viewfinder:
         return true;
      case QCameraControl::CaptureMode:
      case QCameraControl::ImageEncodingSettings:
      case QCameraControl::VideoEncodingSettings:
      case QCameraControl::ViewfinderSettings:
      default:
         return status != QCamera::ActiveStatus;
   }
}

#define VIEWFINDER_COLORSPACE_CONVERSION 0x00000004

bool CameraBinControl::viewfinderColorSpaceConversion() const
{
   gint flags = 0;
   g_object_get(G_OBJECT(m_session->cameraBin()), "flags", &flags, NULL);

   return flags & VIEWFINDER_COLORSPACE_CONVERSION;
}

void CameraBinControl::setViewfinderColorSpaceConversion(bool enabled)
{
   gint flags = 0;
   g_object_get(G_OBJECT(m_session->cameraBin()), "flags", &flags, NULL);

   if (enabled) {
      flags |= VIEWFINDER_COLORSPACE_CONVERSION;
   } else {
      flags &= ~VIEWFINDER_COLORSPACE_CONVERSION;
   }

   g_object_set(G_OBJECT(m_session->cameraBin()), "flags", flags, NULL);
}

