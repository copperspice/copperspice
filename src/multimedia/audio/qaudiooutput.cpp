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

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>
#include <QtMultimedia/qaudiooutput.h>

#include <qaudiodevicefactory_p.h>

QT_BEGIN_NAMESPACE

QAudioOutput::QAudioOutput(const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createDefaultOutputDevice(format);
   connect(d, SIGNAL(notify()), this, SLOT(emitNotify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(emitStateChanged(QAudio::State)));
}

QAudioOutput::QAudioOutput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createOutputDevice(audioDevice, format);
   connect(d, SIGNAL(notify()), this, SLOT(emitNotify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(emitStateChanged(QAudio::State)));
}

QAudioOutput::~QAudioOutput()
{
   delete d;
}

QAudioFormat QAudioOutput::format() const
{
   return d->format();
}


void QAudioOutput::start(QIODevice *device)
{
   d->start(device);
}

/*!
    Returns a pointer to the QIODevice being used to handle the data
    transfer. This QIODevice can be used to write() audio data directly.

    If able to access the systems audio device the state() is set to
    QAudio::IdleState, error() is set to QAudio::NoError
    and the stateChanged() signal is emitted.

    If a problem occurs during this process the error() is set to QAudio::OpenError,
    state() is set to QAudio::StoppedState and stateChanged() signal is emitted.

    In either case, the stateChanged() signal may be emitted either synchronously
    during execution of the start() function or asynchronously after start() has
    returned to the caller.

    \sa QIODevice
*/

QIODevice *QAudioOutput::start()
{
   return d->start(0);
}

/*!
    Stops the audio output, detaching from the system resource.

    Sets error() to QAudio::NoError, state() to QAudio::StoppedState and
    emit stateChanged() signal.
*/

void QAudioOutput::stop()
{
   d->stop();
}

/*!
    Drops all audio data in the buffers, resets buffers to zero.
*/

void QAudioOutput::reset()
{
   d->reset();
}

/*!
    Stops processing audio data, preserving buffered audio data.

    Sets error() to QAudio::NoError, state() to QAudio::SuspendedState and
    emit stateChanged() signal.
*/

void QAudioOutput::suspend()
{
   d->suspend();
}

/*!
    Resumes processing audio data after a suspend().

    Sets error() to QAudio::NoError.
    Sets state() to QAudio::ActiveState if you previously called start(QIODevice*).
    Sets state() to QAudio::IdleState if you previously called start().
    emits stateChanged() signal.

    Note: signal will always be emitted during execution of the resume() function.
*/

void QAudioOutput::resume()
{
   d->resume();
}

/*!
    Returns the free space available in bytes in the audio buffer.

    NOTE: returned value is only valid while in QAudio::ActiveState or QAudio::IdleState
    state, otherwise returns zero.
*/

int QAudioOutput::bytesFree() const
{
   return d->bytesFree();
}

/*!
    Returns the period size in bytes.

    Note: This is the recommended write size in bytes.
*/

int QAudioOutput::periodSize() const
{
   return d->periodSize();
}

/*!
    Sets the audio buffer size to \a value in bytes.

    Note: This function can be called anytime before start(), calls to this
    are ignored after start(). It should not be assumed that the buffer size
    set is the actual buffer size used, calling bufferSize() anytime after start()
    will return the actual buffer size being used.
*/

void QAudioOutput::setBufferSize(int value)
{
   d->setBufferSize(value);
}

/*!
    Returns the audio buffer size in bytes.

    If called before start(), returns platform default value.
    If called before start() but setBufferSize() was called prior, returns value set by setBufferSize().
    If called after start(), returns the actual buffer size being used. This may not be what was set previously
    by setBufferSize().

*/

int QAudioOutput::bufferSize() const
{
   return d->bufferSize();
}

/*!
    Sets the interval for notify() signal to be emitted.
    This is based on the \a ms of audio data processed
    not on actual real-time.
    The minimum resolution of the timer is platform specific and values
    should be checked with notifyInterval() to confirm actual value
    being used.
*/

void QAudioOutput::setNotifyInterval(int ms)
{
   d->setNotifyInterval(ms);
}

/*!
    Returns the notify interval in milliseconds.
*/

int QAudioOutput::notifyInterval() const
{
   return d->notifyInterval();
}

/*!
    Returns the amount of audio data processed by the class since start()
    was called in microseconds.

    Note: The amount of audio data played can be determined by subtracting
    the microseconds of audio data still in the systems audio buffer.

    \code
    qint64 bytesInBuffer = bufferSize() - bytesFree();
    qint64 usInBuffer = (qint64)(1000000) * bytesInBuffer / ( channels() * sampleSize() / 8 ) / frequency();
    qint64 usPlayed = processedUSecs() - usInBuffer;
    \endcode
*/

qint64 QAudioOutput::processedUSecs() const
{
   return d->processedUSecs();
}

/*!
    Returns the microseconds since start() was called, including time in Idle and
    Suspend states.
*/

qint64 QAudioOutput::elapsedUSecs() const
{
   return d->elapsedUSecs();
}

/*!
    Returns the error state.
*/

QAudio::Error QAudioOutput::error() const
{
   return d->error();
}

/*!
    Returns the state of audio processing.
*/

QAudio::State QAudioOutput::state() const
{
   return d->state();
}

/*!
    \fn QAudioOutput::stateChanged(QAudio::State state)
    This signal is emitted when the device \a state has changed.
    This is the current state of the audio output.
*/

/*!
    \fn QAudioOutput::notify()
    This signal is emitted when x ms of audio data has been processed
    the interval set by setNotifyInterval(x).
*/

QT_END_NAMESPACE
