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

#ifndef CAMERABINCAPTUREBUFFERFORMAT_H
#define CAMERABINCAPTUREBUFFERFORMAT_H

#include <qcamera.h>
#include <qcameracapturebufferformatcontrol.h>

#include <gst/gst.h>
#include <glib.h>

class CameraBinSession;

class Q_MULTIMEDIA_EXPORT CameraBinCaptureBufferFormat : public QCameraCaptureBufferFormatControl
{
   CS_OBJECT(CameraBinCaptureBufferFormat)

 public:
   CameraBinCaptureBufferFormat(CameraBinSession *session);
   virtual ~CameraBinCaptureBufferFormat();

   QList<QVideoFrame::PixelFormat> supportedBufferFormats() const override;

   QVideoFrame::PixelFormat bufferFormat() const override;
   void setBufferFormat(QVideoFrame::PixelFormat format) override;

 private:
   CameraBinSession *m_session;
   QVideoFrame::PixelFormat m_format;
};

#endif
