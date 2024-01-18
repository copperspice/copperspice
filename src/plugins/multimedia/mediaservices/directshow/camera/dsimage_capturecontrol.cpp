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
#include <dsimage_capturecontrol.h>

DSImageCaptureControl::DSImageCaptureControl(DSCameraSession *session)
   : QCameraImageCaptureControl(session), m_session(session)
{
   connect(m_session, &DSCameraSession::imageExposed,           this, &DSImageCaptureControl::imageExposed);
   connect(m_session, &DSCameraSession::imageCaptured,          this, &DSImageCaptureControl::imageCaptured);
   connect(m_session, &DSCameraSession::imageSaved,             this, &DSImageCaptureControl::imageSaved);
   connect(m_session, &DSCameraSession::readyForCaptureChanged, this, &DSImageCaptureControl::readyForCaptureChanged);
   connect(m_session, &DSCameraSession::captureError,           this, &DSImageCaptureControl::error);
}

DSImageCaptureControl::~DSImageCaptureControl()
{
}

bool DSImageCaptureControl::isReadyForCapture() const
{
   return m_session->isReadyForCapture();
}

int DSImageCaptureControl::capture(const QString &fileName)
{
   return m_session->captureImage(fileName);
}

QCameraImageCapture::DriveMode DSImageCaptureControl::driveMode() const
{
   return QCameraImageCapture::SingleImageCapture;
}

void DSImageCaptureControl::setDriveMode(QCameraImageCapture::DriveMode mode)
{
   if (mode != QCameraImageCapture::SingleImageCapture) {
      qWarning("Drive mode not supported.");
   }
}


