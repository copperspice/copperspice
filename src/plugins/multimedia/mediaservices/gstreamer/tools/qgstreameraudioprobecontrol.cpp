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

#include <qgstreameraudioprobecontrol_p.h>
#include <qgstutils_p.h>

QGstreamerAudioProbeControl::QGstreamerAudioProbeControl(QObject *parent)
   : QMediaAudioProbeControl(parent)
{
}

QGstreamerAudioProbeControl::~QGstreamerAudioProbeControl()
{
}

void QGstreamerAudioProbeControl::probeCaps(GstCaps *caps)
{
   QAudioFormat format = QGstUtils::audioFormatForCaps(caps);

   QMutexLocker locker(&m_bufferMutex);
   m_format = format;
}

bool QGstreamerAudioProbeControl::probeBuffer(GstBuffer *buffer)
{
   qint64 position = GST_BUFFER_TIMESTAMP(buffer);
   position = position >= 0
      ? position / G_GINT64_CONSTANT(1000) // microseconds
      : -1;

   QByteArray data;
#if GST_CHECK_VERSION(1,0,0)
   GstMapInfo info;
   if (gst_buffer_map(buffer, &info, GST_MAP_READ)) {
      data = QByteArray(reinterpret_cast<const char *>(info.data), info.size);
      gst_buffer_unmap(buffer, &info);
   } else {
      return true;
   }
#else
   data = QByteArray(reinterpret_cast<const char *>(buffer->data), buffer->size);
#endif

   QMutexLocker locker(&m_bufferMutex);
   if (m_format.isValid()) {
      if (!m_pendingBuffer.isValid()) {
         QMetaObject::invokeMethod(this, "bufferProbed", Qt::QueuedConnection);
      }
      m_pendingBuffer = QAudioBuffer(data, m_format, position);
   }

   return true;
}

void QGstreamerAudioProbeControl::bufferProbed()
{
   QAudioBuffer audioBuffer;
   {
      QMutexLocker locker(&m_bufferMutex);
      if (!m_pendingBuffer.isValid()) {
         return;
      }
      audioBuffer = m_pendingBuffer;
      m_pendingBuffer = QAudioBuffer();
   }
   emit audioBufferProbed(audioBuffer);
}
