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

#include <qaudio.h>
#include <qdebug.h>

QDebug operator<<(QDebug dbg, QAudio::Error error)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (error) {
      case QAudio::NoError:
         dbg << "NoError";
         break;
      case QAudio::OpenError:
         dbg << "OpenError";
         break;
      case QAudio::IOError:
         dbg << "IOError";
         break;
      case QAudio::UnderrunError:
         dbg << "UnderrunError";
         break;
      case QAudio::FatalError:
         dbg << "FatalError";
         break;
   }
   return dbg;
}
QDebug operator<<(QDebug dbg, QAudio::State state)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (state) {
      case QAudio::ActiveState:
         dbg << "ActiveState";
         break;
      case QAudio::SuspendedState:
         dbg << "SuspendedState";
         break;
      case QAudio::StoppedState:
         dbg << "StoppedState";
         break;
      case QAudio::IdleState:
         dbg << "IdleState";
         break;
   }
   return dbg;
}
QDebug operator<<(QDebug dbg, QAudio::Mode mode)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (mode) {
      case QAudio::AudioInput:
         dbg << "AudioInput";
         break;
      case QAudio::AudioOutput:
         dbg << "AudioOutput";
         break;
   }
   return dbg;
}
QDebug operator<<(QDebug dbg, QAudio::Role role)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (role) {
      case QAudio::UnknownRole:
         dbg << "UnknownRole";
         break;
      case QAudio::AccessibilityRole:
         dbg << "AccessibilityRole";
         break;
      case QAudio::AlarmRole:
         dbg << "AlarmRole";
         break;
      case QAudio::GameRole:
         dbg << "GameRole";
         break;
      case QAudio::MusicRole:
         dbg << "MusicRole";
         break;
      case QAudio::NotificationRole:
         dbg << "NotificationRole";
         break;
      case QAudio::RingtoneRole:
         dbg << "RingtoneRole";
         break;
      case QAudio::SonificationRole:
         dbg << "SonificationRole";
         break;
      case QAudio::VideoRole:
         dbg << "VideoRole";
         break;
      case QAudio::VoiceCommunicationRole:
         dbg << "VoiceCommunicationRole";
         break;
   }
   return dbg;
}


