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

#include <camera_flash.h>
#include <camera_session.h>
#include <qdebug.h>

#include <gst/interfaces/photography.h>

#if !GST_CHECK_VERSION(1,0,0)
typedef GstFlashMode GstPhotographyFlashMode;
#endif

CameraBinFlash::CameraBinFlash(CameraBinSession *session)
   : QCameraFlashControl(session), m_session(session)
{
}

CameraBinFlash::~CameraBinFlash()
{
}

QCameraExposure::FlashModes CameraBinFlash::flashMode() const
{
   GstPhotographyFlashMode flashMode;
   gst_photography_get_flash_mode(m_session->photography(), &flashMode);

   QCameraExposure::FlashModes modes;
   switch (flashMode) {
      case GST_PHOTOGRAPHY_FLASH_MODE_AUTO:
         modes |= QCameraExposure::FlashAuto;
         break;
      case GST_PHOTOGRAPHY_FLASH_MODE_OFF:
         modes |= QCameraExposure::FlashOff;
         break;
      case GST_PHOTOGRAPHY_FLASH_MODE_ON:
         modes |= QCameraExposure::FlashOn;
         break;
      case GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN:
         modes |= QCameraExposure::FlashFill;
         break;
      case GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE:
         modes |= QCameraExposure::FlashRedEyeReduction;
         break;
      default:
         modes |= QCameraExposure::FlashAuto;
         break;
   }
   return modes;
}

void CameraBinFlash::setFlashMode(QCameraExposure::FlashModes mode)
{
   GstPhotographyFlashMode flashMode;
   gst_photography_get_flash_mode(m_session->photography(), &flashMode);

   if (mode.testFlag(QCameraExposure::FlashAuto)) {
      flashMode = GST_PHOTOGRAPHY_FLASH_MODE_AUTO;
   } else if (mode.testFlag(QCameraExposure::FlashOff)) {
      flashMode = GST_PHOTOGRAPHY_FLASH_MODE_OFF;
   } else if (mode.testFlag(QCameraExposure::FlashOn)) {
      flashMode = GST_PHOTOGRAPHY_FLASH_MODE_ON;
   } else if (mode.testFlag(QCameraExposure::FlashFill)) {
      flashMode = GST_PHOTOGRAPHY_FLASH_MODE_FILL_IN;
   } else if (mode.testFlag(QCameraExposure::FlashRedEyeReduction)) {
      flashMode = GST_PHOTOGRAPHY_FLASH_MODE_RED_EYE;
   }

   gst_photography_set_flash_mode(m_session->photography(), flashMode);
}

bool CameraBinFlash::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{
   return  mode == QCameraExposure::FlashOff ||
           mode == QCameraExposure::FlashOn ||
           mode == QCameraExposure::FlashAuto ||
           mode == QCameraExposure::FlashRedEyeReduction ||
           mode == QCameraExposure::FlashFill;
}

bool CameraBinFlash::isFlashReady() const
{
   return true;
}

