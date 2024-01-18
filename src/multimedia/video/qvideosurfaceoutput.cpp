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

#include <qvideosurfaceoutput_p.h>

#include <qabstractvideosurface.h>
#include <qmediaservice.h>
#include <qvideorenderercontrol.h>

// internal
QVideoSurfaceOutput::QVideoSurfaceOutput(QObject *parent)
   :  QObject(parent)
{
}

QVideoSurfaceOutput::~QVideoSurfaceOutput()
{
   if (m_control) {
      m_control.data()->setSurface(nullptr);
      m_service.data()->releaseControl(m_control.data());
   }
}

QMediaObject *QVideoSurfaceOutput::mediaObject() const
{
   return m_object.data();
}

void QVideoSurfaceOutput::setVideoSurface(QAbstractVideoSurface *surface)
{
   m_surface = surface;

   if (m_control) {
      m_control.data()->setSurface(surface);
   }
}

bool QVideoSurfaceOutput::setMediaObject(QMediaObject *object)
{
   if (m_control) {
      m_control.data()->setSurface(nullptr);
      m_service.data()->releaseControl(m_control.data());
   }

   m_control.clear();
   m_service.clear();
   m_object.clear();

   if (object) {
      if (QMediaService *service = object->service()) {

         if (QMediaControl *control = service->requestControl(QVideoRendererControl_iid)) {
            if ((m_control = dynamic_cast<QVideoRendererControl *>(control))) {
               m_service = service;
               m_object = object;
               m_control.data()->setSurface(m_surface.data());

               return true;
            }
            service->releaseControl(control);
         }
      }
   }

   return false;
}
