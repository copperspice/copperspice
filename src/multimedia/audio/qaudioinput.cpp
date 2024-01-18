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
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>
#include <qaudioinput.h>

#include <qaudiodevicefactory_p.h>

QAudioInput::QAudioInput(const QAudioFormat &format, QObject *parent)
   : QObject(parent)
{
   m_audioInput = QAudioDeviceFactory::createDefaultInputDevice(format);
   connect(m_audioInput, &QAbstractAudioInput::notify,       this, &QAudioInput::notify);
   connect(m_audioInput, &QAbstractAudioInput::stateChanged, this, &QAudioInput::stateChanged);
}

QAudioInput::QAudioInput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent)
   : QObject(parent)
{
   m_audioInput = QAudioDeviceFactory::createInputDevice(audioDevice, format);
   connect(m_audioInput, &QAbstractAudioInput::notify,       this, &QAudioInput::notify);
   connect(m_audioInput, &QAbstractAudioInput::stateChanged, this, &QAudioInput::stateChanged);
}

QAudioInput::~QAudioInput()
{
   delete m_audioInput;
}

void QAudioInput::start(QIODevice *device)
{
   m_audioInput->start(device);
}

QIODevice *QAudioInput::start()
{
   return m_audioInput->start();
}

QAudioFormat QAudioInput::format() const
{
   return m_audioInput->format();
}

void QAudioInput::stop()
{
   m_audioInput->stop();
}

void QAudioInput::reset()
{
   m_audioInput->reset();
}

void QAudioInput::suspend()
{
   m_audioInput->suspend();
}

void QAudioInput::resume()
{
   m_audioInput->resume();
}

void QAudioInput::setBufferSize(int value)
{
   m_audioInput->setBufferSize(value);
}

int QAudioInput::bufferSize() const
{
   return m_audioInput->bufferSize();
}

int QAudioInput::bytesReady() const
{
   /*
   -If not ActiveState|IdleState, return 0
   -return amount of audio data available to read
   */
   return m_audioInput->bytesReady();
}

int QAudioInput::periodSize() const
{
   return m_audioInput->periodSize();
}

void QAudioInput::setNotifyInterval(int ms)
{
   m_audioInput->setNotifyInterval(ms);
}

int QAudioInput::notifyInterval() const
{
   return m_audioInput->notifyInterval();
}

void QAudioInput::setVolume(qreal volume)
{
   qreal v = qBound(qreal(0.0), volume, qreal(1.0));
   m_audioInput->setVolume(v);
}

qreal QAudioInput::volume() const
{
   return m_audioInput->volume();
}

qint64 QAudioInput::processedUSecs() const
{
   return m_audioInput->processedUSecs();
}

qint64 QAudioInput::elapsedUSecs() const
{
   return m_audioInput->elapsedUSecs();
}

QAudio::Error QAudioInput::error() const
{
   return m_audioInput->error();
}

QAudio::State QAudioInput::state() const
{
   return m_audioInput->state();
}
