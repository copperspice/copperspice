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

#include <qgstreamervideoprobecontrol_p.h>

#include <qgstutils_p.h>
#include <qgstvideobuffer_p.h>

QGstreamerVideoProbeControl::QGstreamerVideoProbeControl(QObject *parent)
   : QMediaVideoProbeControl(parent), m_flushing(false), m_frameProbed(false)
{
}

QGstreamerVideoProbeControl::~QGstreamerVideoProbeControl()
{
}

void QGstreamerVideoProbeControl::startFlushing()
{
   m_flushing = true;

   {
      QMutexLocker locker(&m_frameMutex);
      m_pendingFrame = QVideoFrame();
   }

   // only emit flush if at least one frame was probed
   if (m_frameProbed) {
      emit flush();
   }
}

void QGstreamerVideoProbeControl::stopFlushing()
{
   m_flushing = false;
}

void QGstreamerVideoProbeControl::probeCaps(GstCaps *caps)
{
#if GST_CHECK_VERSION(1,0,0)
   GstVideoInfo videoInfo;
   QVideoSurfaceFormat format = QGstUtils::formatForCaps(caps, &videoInfo);

   QMutexLocker locker(&m_frameMutex);
   m_videoInfo = videoInfo;
#else
   int bytesPerLine = 0;
   QVideoSurfaceFormat format = QGstUtils::formatForCaps(caps, &bytesPerLine);

   QMutexLocker locker(&m_frameMutex);
   m_bytesPerLine = bytesPerLine;
#endif

   m_format = format;
}

bool QGstreamerVideoProbeControl::probeBuffer(GstBuffer *buffer)
{
   QMutexLocker locker(&m_frameMutex);

   if (m_flushing || ! m_format.isValid()) {
      return true;
   }

   QVideoFrame frame(

#if GST_CHECK_VERSION(1,0,0)
      new QGstVideoBuffer(buffer, m_videoInfo),
#else
      new QGstVideoBuffer(buffer, m_bytesPerLine),
#endif

      m_format.frameSize(),
      m_format.pixelFormat());

   QGstUtils::setFrameTimeStamps(&frame, buffer);

   m_frameProbed = true;

   if (!m_pendingFrame.isValid()) {
      QMetaObject::invokeMethod(this, "frameProbed", Qt::QueuedConnection);
   }

   m_pendingFrame = frame;

   return true;
}

void QGstreamerVideoProbeControl::frameProbed()
{
   QVideoFrame frame;
   {
      QMutexLocker locker(&m_frameMutex);
      if (!m_pendingFrame.isValid()) {
         return;
      }
      frame = m_pendingFrame;
      m_pendingFrame = QVideoFrame();
   }
   emit videoFrameProbed(frame);
}
