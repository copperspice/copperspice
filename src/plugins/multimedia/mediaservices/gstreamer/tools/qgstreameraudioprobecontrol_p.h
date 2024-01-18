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

#ifndef QGSTREAMERAUDIOPROBECONTROL_H
#define QGSTREAMERAUDIOPROBECONTROL_H

#include <qmediaaudioprobecontrol.h>
#include <qmutex.h>
#include <qaudiobuffer.h>
#include <qshareddata.h>
#include <qgstreamerbufferprobe_p.h>

#include <gst/gst.h>

class QGstreamerAudioProbeControl
   : public QMediaAudioProbeControl, public QGstreamerBufferProbe, public QSharedData
{
   CS_OBJECT_MULTIPLE(QGstreamerAudioProbeControl, QMediaAudioProbeControl)

 public:
   explicit QGstreamerAudioProbeControl(QObject *parent);
   virtual ~QGstreamerAudioProbeControl();

 protected:
   void probeCaps(GstCaps *caps) override;
   bool probeBuffer(GstBuffer *buffer) override;

 private:
   QAudioBuffer m_pendingBuffer;
   QAudioFormat m_format;
   QMutex m_bufferMutex;

   CS_SLOT_1(Private, void bufferProbed())
   CS_SLOT_2(bufferProbed)
};

#endif