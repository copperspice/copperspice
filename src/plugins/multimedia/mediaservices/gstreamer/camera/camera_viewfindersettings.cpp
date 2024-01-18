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

#include <camera_viewfindersettings.h>
#include <camera_session.h>

CameraBinViewfinderSettings::CameraBinViewfinderSettings(CameraBinSession *session)
   : QCameraViewfinderSettingsControl(session), m_session(session)
{
}

CameraBinViewfinderSettings::~CameraBinViewfinderSettings()
{
}

bool CameraBinViewfinderSettings::isViewfinderParameterSupported(ViewfinderParameter parameter) const
{
   switch (parameter) {
      case Resolution:
      case PixelAspectRatio:
      case MinimumFrameRate:
      case MaximumFrameRate:
      case PixelFormat:
         return true;

      case UserParameter:
         return false;
   }
   return false;
}

QVariant CameraBinViewfinderSettings::viewfinderParameter(ViewfinderParameter parameter) const
{
   switch (parameter) {
      case Resolution:
         return m_session->viewfinderSettings().resolution();
      case PixelAspectRatio:
         return m_session->viewfinderSettings().pixelAspectRatio();
      case MinimumFrameRate:
         return m_session->viewfinderSettings().minimumFrameRate();
      case MaximumFrameRate:
         return m_session->viewfinderSettings().maximumFrameRate();
      case PixelFormat:
         return m_session->viewfinderSettings().pixelFormat();
      case UserParameter:
         return QVariant();
   }
   return false;
}

void CameraBinViewfinderSettings::setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value)
{
   QCameraViewfinderSettings settings = m_session->viewfinderSettings();

   switch (parameter) {
      case Resolution:
         settings.setResolution(value.toSize());
         break;

      case PixelAspectRatio:
         settings.setPixelAspectRatio(value.toSize());
         break;

      case MinimumFrameRate:
         settings.setMinimumFrameRate(value.toReal());
         break;

      case MaximumFrameRate:
         settings.setMaximumFrameRate(value.toReal());
         break;

      case PixelFormat:
         settings.setPixelFormat(value.value<QVideoFrame::PixelFormat>());

      case UserParameter:
         break;
   }

   m_session->setViewfinderSettings(settings);
}

