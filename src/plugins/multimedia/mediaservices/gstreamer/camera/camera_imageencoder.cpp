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

#include <camera_imageencoder.h>
#include <camera_session.h>
#include <qdebug.h>

CameraBinImageEncoder::CameraBinImageEncoder(CameraBinSession *session)
   : QImageEncoderControl(session), m_session(session)
{
}

CameraBinImageEncoder::~CameraBinImageEncoder()
{
}

QList<QSize> CameraBinImageEncoder::supportedResolutions(const QImageEncoderSettings &, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   return m_session->supportedResolutions(qMakePair<int, int>(0, 0), continuous, QCamera::CaptureStillImage);
}

QStringList CameraBinImageEncoder::supportedImageCodecs() const
{
   return QStringList() << "jpeg";
}

QString CameraBinImageEncoder::imageCodecDescription(const QString &codecName) const
{
   if (codecName == "jpeg") {
      return tr("JPEG image");
   }

   return QString();
}

QImageEncoderSettings CameraBinImageEncoder::imageSettings() const
{
   return m_settings;
}

void CameraBinImageEncoder::setImageSettings(const QImageEncoderSettings &settings)
{
   m_settings = settings;
   emit settingsChanged();
}

