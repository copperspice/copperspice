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

#ifndef QAUDIOROLECONTROL_H
#define QAUDIOROLECONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qaudio.h>

class Q_MULTIMEDIA_EXPORT QAudioRoleControl : public QMediaControl
{
   MULTI_CS_OBJECT(QAudioRoleControl)

 public:
   virtual ~QAudioRoleControl();

   virtual QAudio::Role audioRole() const = 0;
   virtual void setAudioRole(QAudio::Role role) = 0;

   virtual QList<QAudio::Role> supportedAudioRoles() const = 0;

   MULTI_CS_SIGNAL_1(Public, void audioRoleChanged(QAudio::Role role))
   MULTI_CS_SIGNAL_2(audioRoleChanged, role)

 protected:
   explicit QAudioRoleControl(QObject *parent = nullptr);
};

#define QAudioRoleControl_iid "com.copperspice.CS.audioRoleControl/1.0"
CS_DECLARE_INTERFACE(QAudioRoleControl, QAudioRoleControl_iid)

#endif
