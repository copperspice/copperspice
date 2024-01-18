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

#ifndef QGSTREAMERVIDEOPROBECONTROL_H
#define QGSTREAMERVIDEOPROBECONTROL_H

#include <qmediavideoprobecontrol.h>
#include <qmutex.h>
#include <qvideoframe.h>
#include <qvideosurfaceformat.h>

#include <qgstreamerbufferprobe_p.h>

#include <gst/gst.h>
#include <gst/video/video.h>

class QGstreamerVideoProbeControl
   : public QMediaVideoProbeControl, public QGstreamerBufferProbe, public QSharedData
{
   CS_OBJECT(QGstreamerVideoProbeControl)

 public:
   explicit QGstreamerVideoProbeControl(QObject *parent);
   virtual ~QGstreamerVideoProbeControl();

   void probeCaps(GstCaps *caps) override;
   bool probeBuffer(GstBuffer *buffer) override;

   void startFlushing();
   void stopFlushing();

 private:
   CS_SLOT_1(Private, void frameProbed())
   CS_SLOT_2(frameProbed)

   QVideoSurfaceFormat m_format;
   QVideoFrame m_pendingFrame;
   QMutex m_frameMutex;

#if GST_CHECK_VERSION(1,0,0)
   GstVideoInfo m_videoInfo;
#else
   int m_bytesPerLine;
#endif

   bool m_flushing;
   bool m_frameProbed; // true if at least one frame was probed
};

#endif
