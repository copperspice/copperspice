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

#include <qaudio.h>
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>
#include <qaudioinput.h>

#include <qaudiodevicefactory_p.h>

QAudioInput::QAudioInput(const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createDefaultInputDevice(format);
   connect(d, SIGNAL(notify()), this, SLOT(notify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
}

/*!
    Construct a new audio input and attach it to \a parent.
    The device referenced by \a audioDevice is used with the input
    \a format parameters.
*/

QAudioInput::QAudioInput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createInputDevice(audioDevice, format);
   connect(d, SIGNAL(notify()), this, SLOT(notify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
}

/*!
    Destroy this audio input.
*/

QAudioInput::~QAudioInput()
{
   delete d;
}

void QAudioInput::start(QIODevice *device)
{
   d->start(device);
}

/*!

    Returns a pointer to a new QIODevice that will be used to handle the data transfer.
    This QIODevice can be used to \l{QIODevice::}{read()} audio data directly.
    You will typically connect to the \l{QIODevice::}{readyRead()} signal, and
    read from the device in the slot you connect to. QAudioInput keeps ownership
    of the device.

    If able to access the systems audio device the state() is set to
    QAudio::IdleState, error() is set to QAudio::NoError
    and the stateChanged() signal is emitted.

    If a problem occurs during this process the error() is set to QAudio::OpenError,
    state() is set to QAudio::StoppedState and stateChanged() signal is emitted.

    \l{QAudioInput#Symbian Platform Security Requirements}

    \sa QIODevice
*/

QIODevice *QAudioInput::start()
{
   return d->start();
}

/*!
    Returns the QAudioFormat being used.
*/

QAudioFormat QAudioInput::format() const
{
   return d->format();
}

/*!
    Stops the audio input, detaching from the system resource.

    Sets error() to QAudio::NoError, state() to QAudio::StoppedState and
    emit stateChanged() signal.
*/

void QAudioInput::stop()
{
   d->stop();
}

/*!
    Drops all audio data in the buffers, resets buffers to zero.
*/

void QAudioInput::reset()
{
   d->reset();
}

/*!
    Stops processing audio data, preserving buffered audio data.

    Sets error() to QAudio::NoError, state() to QAudio::SuspendedState and
    emit stateChanged() signal.
*/

void QAudioInput::suspend()
{
   d->suspend();
}

/*!
    Resumes processing audio data after a suspend().

    Sets error() to QAudio::NoError.
    Sets state() to QAudio::ActiveState if you previously called start(QIODevice*).
    Sets state() to QAudio::IdleState if you previously called start().
    emits stateChanged() signal.
*/

void QAudioInput::resume()
{
   d->resume();
}

/*!
    Sets the audio buffer size to \a value milliseconds.

    Note: This function can be called anytime before start(), calls to this
    are ignored after start(). It should not be assumed that the buffer size
    set is the actual buffer size used, calling bufferSize() anytime after start()
    will return the actual buffer size being used.

*/

void QAudioInput::setBufferSize(int value)
{
   d->setBufferSize(value);
}

/*!
    Returns the audio buffer size in milliseconds.

    If called before start(), returns platform default value.
    If called before start() but setBufferSize() was called prior, returns value set by setBufferSize().
    If called after start(), returns the actual buffer size being used. This may not be what was set previously
    by setBufferSize().

*/

int QAudioInput::bufferSize() const
{
   return d->bufferSize();
}

/*!
    Returns the amount of audio data available to read in bytes.

    NOTE: returned value is only valid while in QAudio::ActiveState or QAudio::IdleState
    state, otherwise returns zero.
*/

int QAudioInput::bytesReady() const
{
   /*
   -If not ActiveState|IdleState, return 0
   -return amount of audio data available to read
   */
   return d->bytesReady();
}

/*!
    Returns the period size in bytes.

    Note: This is the recommended read size in bytes.
*/

int QAudioInput::periodSize() const
{
   return d->periodSize();
}

/*!
    Sets the interval for notify() signal to be emitted.
    This is based on the \a ms of audio data processed
    not on actual real-time.
    The minimum resolution of the timer is platform specific and values
    should be checked with notifyInterval() to confirm actual value
    being used.
*/

void QAudioInput::setNotifyInterval(int ms)
{
   d->setNotifyInterval(ms);
}

/*!
    Returns the notify interval in milliseconds.
*/

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
