/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSOUNDQSS_QWS_H
#define QSOUNDQSS_QWS_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SOUND

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtGui/qwssocket_qws.h>

QT_BEGIN_NAMESPACE

#if defined(QT_NO_NETWORK) || defined(QT_NO_DNS)
#define QT_NO_QWS_SOUNDSERVER
#endif

#ifndef Q_OS_MAC

class QWSSoundServerPrivate;

class Q_GUI_EXPORT QWSSoundServer : public QObject
{
   GUI_CS_OBJECT(QWSSoundServer)

 public:
   explicit QWSSoundServer(QObject *parent = nullptr);
   ~QWSSoundServer();
   void playFile( int id, const QString &filename );
   void stopFile( int id );
   void pauseFile( int id );
   void resumeFile( int id );

   GUI_CS_SIGNAL_1(Public, void soundCompleted(int un_named_arg1))
   GUI_CS_SIGNAL_2(soundCompleted, un_named_arg1)

 private :
   GUI_CS_SLOT_1(Private, void translateSoundCompleted(int un_named_arg1, int un_named_arg2))
   GUI_CS_SLOT_2(translateSoundCompleted)

   QWSSoundServerPrivate *d;
};

#ifndef QT_NO_QWS_SOUNDSERVER
class Q_GUI_EXPORT QWSSoundClient : public QWSSocket
{
   GUI_CS_OBJECT(QWSSoundClient)

 public:

   enum SoundFlags {
      Priority = 0x01,
      Streaming = 0x02  // currently ignored, but but could set up so both Raw and non raw can be done streaming or not.
   };

   enum DeviceErrors {
      ErrOpeningAudioDevice = 0x01,
      ErrOpeningFile = 0x02,
      ErrReadingFile = 0x04
   };
   explicit QWSSoundClient(QObject *parent = nullptr);

   ~QWSSoundClient( );
   void reconnect();
   void play( int id, const QString &filename );
   void play( int id, const QString &filename, int volume, int flags = 0 );
   void playRaw( int id, const QString &, int, int, int, int flags = 0 );

   void pause( int id );
   void stop( int id );
   void resume( int id );
   void setVolume( int id, int left, int right );
   void setMute( int id, bool m );

   // to be used by server only, to protect phone conversation/rings.
   void playPriorityOnly(bool);

   // If silent, tell sound server to release audio device
   // Otherwise, allow sound server to regain audio device
   void setSilent(bool);

   GUI_CS_SIGNAL_1(Public, void soundCompleted(int un_named_arg1))
   GUI_CS_SIGNAL_2(soundCompleted, un_named_arg1)
   GUI_CS_SIGNAL_1(Public, void deviceReady(int id))
   GUI_CS_SIGNAL_2(deviceReady, id)
   GUI_CS_SIGNAL_1(Public, void deviceError(int id, QWSSoundClient::DeviceErrors un_named_arg2))
   GUI_CS_SIGNAL_2(deviceError, id, un_named_arg2)

 private :
   GUI_CS_SLOT_1(Private, void tryReadCommand())
   GUI_CS_SLOT_2(tryReadCommand)
   GUI_CS_SLOT_1(Private, void emitConnectionRefused())
   GUI_CS_SLOT_2(emitConnectionRefused)

   void sendServerMessage(QString msg);
};

class QWSSoundServerSocket : public QWSServerSocket
{
   GUI_CS_OBJECT(QWSSoundServerSocket)

 public:
   explicit QWSSoundServerSocket(QObject *parent = nullptr);

   GUI_CS_SLOT_1(Public, void newConnection())
   GUI_CS_SLOT_2(newConnection)

   GUI_CS_SIGNAL_1(Public, void playFile(int un_named_arg1, int un_named_arg2, const QString &un_named_arg3))
   GUI_CS_SIGNAL_2(playFile, un_named_arg1, un_named_arg2, un_named_arg3)

   GUI_CS_SIGNAL_1(Public, void playFile(int un_named_arg1, int un_named_arg2, const QString &un_named_arg3,
                                         int un_named_arg4, int un_named_arg5))

   GUI_CS_SIGNAL_2(playFile, un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5)

   GUI_CS_SIGNAL_1(Public, void playRawFile(int un_named_arg1, int un_named_arg2, const QString &un_named_arg3,
                   int un_named_arg4, int un_named_arg5, int un_named_arg6, int un_named_arg7))

   GUI_CS_SIGNAL_2(playRawFile, un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5, un_named_arg6, un_named_arg7)

   GUI_CS_SIGNAL_1(Public, void pauseFile(int un_named_arg1, int un_named_arg2))
   GUI_CS_SIGNAL_2(pauseFile, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void stopFile(int un_named_arg1, int un_named_arg2))
   GUI_CS_SIGNAL_2(stopFile, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void resumeFile(int un_named_arg1, int un_named_arg2))
   GUI_CS_SIGNAL_2(resumeFile, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void setVolume(int un_named_arg1, int un_named_arg2, int un_named_arg3, int un_named_arg4))
   GUI_CS_SIGNAL_2(setVolume, un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4)
   GUI_CS_SIGNAL_1(Public, void setMute(int un_named_arg1, int un_named_arg2, bool un_named_arg3))
   GUI_CS_SIGNAL_2(setMute, un_named_arg1, un_named_arg2, un_named_arg3)

   GUI_CS_SIGNAL_1(Public, void stopAll(int un_named_arg1))
   GUI_CS_SIGNAL_2(stopAll, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void playPriorityOnly(bool un_named_arg1))
   GUI_CS_SIGNAL_2(playPriorityOnly, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void setSilent(bool un_named_arg1))
   GUI_CS_SIGNAL_2(setSilent, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void soundFileCompleted(int un_named_arg1, int un_named_arg2))
   GUI_CS_SIGNAL_2(soundFileCompleted, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void deviceReady(int un_named_arg1, int un_named_arg2))
   GUI_CS_SIGNAL_2(deviceReady, un_named_arg1, un_named_arg2)
   GUI_CS_SIGNAL_1(Public, void deviceError(int un_named_arg1, int un_named_arg2, int un_named_arg3))
   GUI_CS_SIGNAL_2(deviceError, un_named_arg1, un_named_arg2, un_named_arg3)
};
#endif

#endif // Q_OS_MAC

QT_END_NAMESPACE

#endif // QT_NO_SOUND

#endif // QSOUNDQSS_QWS_H
