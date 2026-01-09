/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef CAMERABINIMAGECAPTURECONTROL_H
#define CAMERABINIMAGECAPTURECONTROL_H

#include <camera_session.h>
#include <qcameraimagecapturecontrol.h>
#include <qvideosurfaceformat.h>

#include <qgstreamerbufferprobe_p.h>

#include <gst/video/video.h>

class CameraBinImageCapture : public QCameraImageCaptureControl, public QGstreamerBusMessageFilter
{
   CS_OBJECT(CameraBinImageCapture)
   CS_INTERFACES(QGstreamerBusMessageFilter)

 public:
   CameraBinImageCapture(CameraBinSession *session);
   virtual ~CameraBinImageCapture();

   QCameraImageCapture::DriveMode driveMode() const override {
      return QCameraImageCapture::SingleImageCapture;
   }

   void setDriveMode(QCameraImageCapture::DriveMode) override {
   }

   bool isReadyForCapture() const override;
   int capture(const QString &fileName) override;
   void cancelCapture() override;

   bool processBusMessage(const QGstreamerMessage &message) override;

 private:
   static GstPadProbeReturn encoderEventProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data);

   class EncoderProbe : public QGstreamerBufferProbe
   {
    public:
      EncoderProbe(CameraBinImageCapture *capture)
         : m_capture(capture)
      { }

      void probeCaps(GstCaps *caps) override;
      bool probeBuffer(GstBuffer *buffer) override;

    private:
      CameraBinImageCapture *const m_capture;
   };

   class MuxerProbe : public QGstreamerBufferProbe
   {
    public:
      MuxerProbe(CameraBinImageCapture *capture)
         : m_capture(capture)
      { }

      void probeCaps(GstCaps *caps) override;
      bool probeBuffer(GstBuffer *buffer) override;

    private:
      CameraBinImageCapture *const m_capture;
   };

   EncoderProbe m_encoderProbe;
   MuxerProbe m_muxerProbe;

   QVideoSurfaceFormat m_bufferFormat;
   QSize m_jpegResolution;
   CameraBinSession *m_session;
   GstElement *m_jpegEncoderElement;
   GstElement *m_metadataMuxerElement;

   GstVideoInfo m_videoInfo;

   int m_requestId;
   bool m_ready;

   CS_SLOT_1(Private, void updateState())
   CS_SLOT_2(updateState)
};

#endif
