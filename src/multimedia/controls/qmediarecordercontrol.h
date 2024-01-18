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

#ifndef QMEDIARECORDERCONTROL_H
#define QMEDIARECORDERCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qmediarecorder.h>

class QUrl;

class Q_MULTIMEDIA_EXPORT QMediaRecorderControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaRecorderControl)

 public:
   virtual ~QMediaRecorderControl();

   virtual QUrl outputLocation() const = 0;
   virtual bool setOutputLocation(const QUrl &location) = 0;

   virtual QMediaRecorder::State state() const = 0;
   virtual QMediaRecorder::Status status() const = 0;

   virtual qint64 duration() const = 0;

   virtual bool isMuted() const = 0;
   virtual qreal volume() const = 0;

   virtual void applySettings() = 0;

 public:
   MULTI_CS_SIGNAL_1(Public, void stateChanged(QMediaRecorder::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void statusChanged(QMediaRecorder::Status status))
   MULTI_CS_SIGNAL_2(statusChanged, status)

   MULTI_CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   MULTI_CS_SIGNAL_2(durationChanged, duration)

   MULTI_CS_SIGNAL_1(Public, void mutedChanged(bool muted))
   MULTI_CS_SIGNAL_2(mutedChanged, muted)

   MULTI_CS_SIGNAL_1(Public, void volumeChanged(qreal volume))
   MULTI_CS_SIGNAL_2(volumeChanged, volume)

   MULTI_CS_SIGNAL_1(Public, void actualLocationChanged(const QUrl & location))
   MULTI_CS_SIGNAL_2(actualLocationChanged, location)

   MULTI_CS_SIGNAL_1(Public, void error(int error,const QString & errorString))
   MULTI_CS_SIGNAL_2(error,error, errorString)

   MULTI_CS_SLOT_1(Public, virtual void setState(QMediaRecorder::State state)=0)
   MULTI_CS_SLOT_2(setState)

   MULTI_CS_SLOT_1(Public, virtual void setMuted(bool muted)=0)
   MULTI_CS_SLOT_2(setMuted)

   MULTI_CS_SLOT_1(Public, virtual void setVolume(qreal volume)=0)
   MULTI_CS_SLOT_2(setVolume)

 protected:
   explicit QMediaRecorderControl(QObject *parent = nullptr);
};

#define QMediaRecorderControl_iid "com.copperspice.CS.mediaRecorderControl/1.0"
CS_DECLARE_INTERFACE(QMediaRecorderControl, QMediaRecorderControl_iid)

#endif
