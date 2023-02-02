/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef CAMERABINRECORDERCONTROL_H
#define CAMERABINRECORDERCONTROL_H

#include <qmediarecordercontrol.h>
#include <camera_session.h>

#ifdef HAVE_GST_ENCODING_PROFILES
#include <gst/pbutils/encoding-profile.h>
#endif

class CameraBinRecorder : public QMediaRecorderControl
{
   CS_OBJECT(CameraBinRecorder)

 public:
   CameraBinRecorder(CameraBinSession *session);
   virtual ~CameraBinRecorder();

   QUrl outputLocation() const;
   bool setOutputLocation(const QUrl &sink);

   QMediaRecorder::State state() const;
   QMediaRecorder::Status status() const;

   qint64 duration() const;

   bool isMuted() const;
   qreal volume() const;

   void applySettings();

#ifdef HAVE_GST_ENCODING_PROFILES
   GstEncodingContainerProfile *videoProfile();
#endif

   CS_SLOT_1(Public, void setState(QMediaRecorder::State state))
   CS_SLOT_2(setState)

   CS_SLOT_1(Public, void setMuted(bool un_named_arg1))
   CS_SLOT_2(setMuted)

   CS_SLOT_1(Public, void setVolume(qreal volume))
   CS_SLOT_2(setVolume)

   CS_SLOT_1(Public, void updateStatus())
   CS_SLOT_2(updateStatus)

 private:
   CameraBinSession *m_session;
   QMediaRecorder::State m_state;
   QMediaRecorder::Status m_status;
};

#endif
