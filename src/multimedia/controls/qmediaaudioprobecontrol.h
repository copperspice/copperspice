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

#ifndef QMEDIAAUDIOPROBECONTROL_H
#define QMEDIAAUDIOPROBECONTROL_H

#include <qmediacontrol.h>
#include <qaudiobuffer.h>

class Q_MULTIMEDIA_EXPORT QMediaAudioProbeControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaAudioProbeControl)

 public:
   virtual ~QMediaAudioProbeControl();

   MULTI_CS_SIGNAL_1(Public, void audioBufferProbed(const QAudioBuffer &buffer))
   MULTI_CS_SIGNAL_2(audioBufferProbed, buffer)

   MULTI_CS_SIGNAL_1(Public, void flush())
   MULTI_CS_SIGNAL_2(flush)

 protected:
   explicit QMediaAudioProbeControl(QObject *parent = nullptr);
};

#define QMediaAudioProbeControl_iid "com.copperspice.CS.mediaAudioProbeControl/1.0"
CS_DECLARE_INTERFACE(QMediaAudioProbeControl, QMediaAudioProbeControl_iid)

#endif
