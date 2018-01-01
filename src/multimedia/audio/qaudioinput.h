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

#ifndef QAUDIOINPUT_H
#define QAUDIOINPUT_H

#include <QtCore/qiodevice.h>
#include <QtCore/qglobal.h>
#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudiodeviceinfo.h>

QT_BEGIN_NAMESPACE

class QAbstractAudioInput;

class Q_MULTIMEDIA_EXPORT QAudioInput : public QObject
{
   MULTI_CS_OBJECT(QAudioInput)

 public:
   explicit QAudioInput(const QAudioFormat &format = QAudioFormat(), QObject *parent = nullptr);
   explicit QAudioInput(const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format = QAudioFormat(),
                        QObject *parent = nullptr);
   ~QAudioInput();

   QAudioFormat format() const;

   void start(QIODevice *device);
   QIODevice *start();

   void stop();
   void reset();
   void suspend();
   void resume();

   void setBufferSize(int bytes);
   int bufferSize() const;

   int bytesReady() const;
   int periodSize() const;

   void setNotifyInterval(int milliSeconds);
   int notifyInterval() const;

   qint64 processedUSecs() const;
   qint64 elapsedUSecs() const;

   QAudio::Error error() const;
   QAudio::State state() const;

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QAudio::State un_named_arg1))
   MULTI_CS_SIGNAL_2(stateChanged, un_named_arg1)
   MULTI_CS_SIGNAL_1(Public, void notify())
   MULTI_CS_SIGNAL_2(notify)
   //
   MULTI_CS_SLOT_1(Public, void emitStateChanged(QAudio::State un_named_arg1) {emit stateChanged(un_named_arg1);})
   MULTI_CS_SLOT_2(emitStateChanged)
   MULTI_CS_SLOT_1(Public, void emitNotify() {emit notify();})
   MULTI_CS_SLOT_2(emitNotify)
   //

 private:
   Q_DISABLE_COPY(QAudioInput)

   QAbstractAudioInput *d;
};

QT_END_NAMESPACE

#endif // QAUDIOINPUT_H
