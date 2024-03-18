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

#ifndef QAUDIO_H
#define QAUDIO_H

#include <qmultimedia.h>
#include <qglobal.h>
#include <qstring.h>

namespace QAudio {

enum Error {
   NoError,
   OpenError,
   IOError,
   UnderrunError,
   FatalError
};

enum State {
   ActiveState,
   SuspendedState,
   StoppedState,
   IdleState
};

enum Mode  {
   AudioInput,
   AudioOutput
};

enum Role {
   UnknownRole,
   MusicRole,
   VideoRole,
   VoiceCommunicationRole,
   AlarmRole,
   NotificationRole,
   RingtoneRole,
   AccessibilityRole,
   SonificationRole,
   GameRole
};
}

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug dbg, QAudio::Error error);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug dbg, QAudio::State state);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug dbg, QAudio::Mode mode);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug dbg, QAudio::Role role);

#endif
