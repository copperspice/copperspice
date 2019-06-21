/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>
#include <qaudioinput.h>

#include <qaudiodevicefactory_p.h>

QAudioInput::QAudioInput(const QAudioFormat &format, QObject *parent)
   : QObject(parent)
{
   d = QAudioDeviceFactory::createDefaultInputDevice(format);
   connect(d, SIGNAL(notify()),                    this, SLOT(notify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
}

QAudioInput::QAudioInput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createInputDevice(audioDevice, format);
   connect(d, SIGNAL(notify()),                    this, SLOT(notify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
}

QAudioInput::~QAudioInput()
{
   delete d;
}

void QAudioInput::start(QIODevice *device)
{
   d->start(device);
}

QIODevice *QAudioInput::start()
{
   return d->start();
}

QAudioFormat QAudioInput::format() const
{
   return d->format();
}

void QAudioInput::stop()
{
   d->stop();
}

void QAudioInput::reset()
{
   d->reset();
}

void QAudioInput::suspend()
{
   d->suspend();
}

void QAudioInput::resume()
{
   d->resume();
}


void QAudioInput::setBufferSize(int value)
{
   d->setBufferSize(value);
}

int QAudioInput::bufferSize() const
{
   return d->bufferSize();
}

int QAudioInput::bytesReady() const
{
   /*
   -If not ActiveState|IdleState, return 0
   -return amount of audio data available to read
   */
   return d->bytesReady();
}

int QAudioInput::periodSize() const
{
   return d->periodSize();
}

void QAudioInput::setNotifyInterval(int ms)
{
   d->setNotifyInterval(ms);
}

int QAudioInput::notifyInterval() const
{
   return d->notifyInterval();
}
void QAudioInput::setVolume(qreal volume)
{
   qreal v = qBound(qreal(0.0), volume, qreal(1.0));
   d->setVolume(v);
}
qreal QAudioInput::volume() const
{
   return d->volume();
}

qint64 QAudioInput::processedUSecs() const
{
   return d->processedUSecs();
}

qint64 QAudioInput::elapsedUSecs() const
{
   return d->elapsedUSecs();
}

QAudio::Error QAudioInput::error() const
{
   return d->error();
}

QAudio::State QAudioInput::state() const
{
   return d->state();
}
