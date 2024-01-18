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

#include <camera_capturebufferformat.h>
#include <camera_session.h>

CameraBinCaptureBufferFormat::CameraBinCaptureBufferFormat(CameraBinSession *session)
   : QCameraCaptureBufferFormatControl(session), m_session(session), m_format(QVideoFrame::Format_Jpeg)
{
}

CameraBinCaptureBufferFormat::~CameraBinCaptureBufferFormat()
{
}

QList<QVideoFrame::PixelFormat> CameraBinCaptureBufferFormat::supportedBufferFormats() const
{
   //the exact YUV format is unknown with camerabin until the first capture is requested
   return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_Jpeg;
}

QVideoFrame::PixelFormat CameraBinCaptureBufferFormat::bufferFormat() const
{
   return m_format;
}

void CameraBinCaptureBufferFormat::setBufferFormat(QVideoFrame::PixelFormat format)
{
   if (m_format != format) {
      m_format = format;
      emit bufferFormatChanged(format);
   }
}

