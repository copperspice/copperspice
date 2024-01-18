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

#ifndef QMEDIAVIDEOPROBECONTROL_H
#define QMEDIAVIDEOPROBECONTROL_H

#include <qmediacontrol.h>
#include <qvideoframe.h>

class Q_MULTIMEDIA_EXPORT QMediaVideoProbeControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaVideoProbeControl)

 public:
   virtual ~QMediaVideoProbeControl();

   MULTI_CS_SIGNAL_1(Public, void videoFrameProbed(const QVideoFrame &frame))
   MULTI_CS_SIGNAL_2(videoFrameProbed, frame)

   MULTI_CS_SIGNAL_1(Public, void flush())
   MULTI_CS_SIGNAL_2(flush)

 protected:
   explicit QMediaVideoProbeControl(QObject *parent = nullptr);
};

#define QMediaVideoProbeControl_iid "com.copperspice.CS.mediaVideoProbeControl/1.0"
CS_DECLARE_INTERFACE(QMediaVideoProbeControl, QMediaVideoProbeControl_iid)

#endif
