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
#include <qaudiooutput.h>

#include <qaudiodevicefactory_p.h>

QAudioOutput::QAudioOutput(const QAudioFormat &format, QObject *parent)
   : QObject(parent)
{
   m_audioOutput = QAudioDeviceFactory::createDefaultOutputDevice(format);

   connect(m_audioOutput, &QAbstractAudioOutput::notify,       this, &QAudioOutput::notify);
   connect(m_audioOutput, &QAbstractAudioOutput::stateChanged, this, &QAudioOutput::stateChanged);
}

QAudioOutput::QAudioOutput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent)
   : QObject(parent)
{
   m_audioOutput = QAudioDeviceFactory::createOutputDevice(audioDevice, format);
   connect(m_audioOutput, &QAbstractAudioOutput::notify,       this, &QAudioOutput::notify);
   connect(m_audioOutput, &QAbstractAudioOutput::stateChanged, this, &QAudioOutput::stateChanged);
}

QAudioOutput::~QAudioOutput()
{
   delete m_audioOutput;
}

QAudioFormat QAudioOutput::format() const
{
   return m_audioOutput->format();
}

void QAudioOutput::start(QIODevice *device)
{
   m_audioOutput->start(device);
}

QIODevice *QAudioOutput::start()
{
   return m_audioOutput->start();
}

void QAudioOutput::stop()
{
   m_audioOutput->stop();
}

void QAudioOutput::reset()
{
   m_audioOutput->reset();
}

void QAudioOutput::suspend()
{
   m_audioOutput->suspend();
}

void QAudioOutput::resume()
{
   m_audioOutput->resume();
}

int QAudioOutput::bytesFree() const
{
   return m_audioOutput->bytesFree();
}

int QAudioOutput::periodSize() const
{
   return m_audioOutput->periodSize();
}

void QAudioOutput::setBufferSize(int value)
{
   m_audioOutput->setBufferSize(value);
}

int QAudioOutput::bufferSize() const
{
   return m_audioOutput->bufferSize();
}

void QAudioOutput::setNotifyInterval(int ms)
{
   m_audioOutput->setNotifyInterval(ms);
}

int QAudioOutput::notifyInterval() const
{
   return m_audioOutput->notifyInterval();
}

qint64 QAudioOutput::processedUSecs() const
{
   return m_audioOutput->processedUSecs();
}

qint64 QAudioOutput::elapsedUSecs() const
{
   return m_audioOutput->elapsedUSecs();
}

QAudio::Error QAudioOutput::error() const
{
   return m_audioOutput->error();
}

QAudio::State QAudioOutput::state() const
{
   return m_audioOutput->state();
}

void QAudioOutput::setVolume(qreal volume)
{
   qreal v = qBound(qreal(0.0), volume, qreal(1.0));
   m_audioOutput->setVolume(v);
}

qreal QAudioOutput::volume() const
{
   return m_audioOutput->volume();
}

QString QAudioOutput::category() const
{
   return m_audioOutput->category();
}

void QAudioOutput::setCategory(const QString &category)
{
   m_audioOutput->setCategory(category);
}
