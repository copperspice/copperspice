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

#ifndef QMEDIASTREAMSCONTROL_H
#define QMEDIASTREAMSCONTROL_H

#include <qmediacontrol.h>
#include <qmultimedia.h>
#include <qstring.h>

class Q_MULTIMEDIA_EXPORT QMediaStreamsControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaStreamsControl)
   MULTI_CS_ENUM(StreamType)

 public:
   enum StreamType {
      UnknownStream,
      VideoStream,
      AudioStream,
      SubPictureStream,
      DataStream
   };

   virtual ~QMediaStreamsControl();

   virtual int streamCount() = 0;
   virtual StreamType streamType(int streamID) = 0;

   virtual QVariant metaData(int streamID, const QString &key) = 0;

   virtual bool isActive(int streamID) = 0;
   virtual void setActive(int streamID, bool state) = 0;

   MULTI_CS_SIGNAL_1(Public, void streamsChanged())
   MULTI_CS_SIGNAL_2(streamsChanged)

   MULTI_CS_SIGNAL_1(Public, void activeStreamsChanged())
   MULTI_CS_SIGNAL_2(activeStreamsChanged)

 protected:
   explicit QMediaStreamsControl(QObject *parent = nullptr);
};

#define QMediaStreamsControl_iid "com.copperspice.CS.mediaStreamsControl/1.0"
CS_DECLARE_INTERFACE(QMediaStreamsControl, QMediaStreamsControl_iid)

#endif
