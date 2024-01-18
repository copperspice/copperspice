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

#ifndef QCAMERACAPTUREBUFFERFORMATCONTROL_H
#define QCAMERACAPTUREBUFFERFORMATCONTROL_H

#include <qmediacontrol.h>
#include <qcameraimagecapture.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QCameraCaptureBufferFormatControl : public QMediaControl
{
   MULTI_CS_OBJECT(QCameraCaptureBufferFormatControl)

 public:
   ~QCameraCaptureBufferFormatControl();

   virtual QList<QVideoFrame::PixelFormat> supportedBufferFormats() const = 0;
   virtual QVideoFrame::PixelFormat bufferFormat() const = 0;
   virtual void setBufferFormat(QVideoFrame::PixelFormat format) = 0;

   MULTI_CS_SIGNAL_1(Public, void bufferFormatChanged(QVideoFrame::PixelFormat format))
   MULTI_CS_SIGNAL_2(bufferFormatChanged, format)

 protected:
   explicit QCameraCaptureBufferFormatControl(QObject *parent = nullptr);
};

#define QCameraCaptureBufferFormatControl_iid "com.copperspice.CS.cameraCaptureBufferFormatControl/1.0"
CS_DECLARE_INTERFACE(QCameraCaptureBufferFormatControl, QCameraCaptureBufferFormatControl_iid)

#endif

