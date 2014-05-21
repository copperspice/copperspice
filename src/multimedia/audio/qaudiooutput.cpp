/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/


#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>
#include <QtMultimedia/qaudiooutput.h>

#include "qaudiodevicefactory_p.h"


QT_BEGIN_NAMESPACE

/*!
    \class QAudioOutput
    \brief The QAudioOutput class provides an interface for sending audio data to an audio output device.

    \inmodule QtMultimedia
    \ingroup  multimedia
    \since 4.6

    You can construct an audio output with the system's
    \l{QAudioDeviceInfo::defaultOutputDevice()}{default audio output
    device}. It is also possible to create QAudioOutput with a
    specific QAudioDeviceInfo. When you create the audio output, you
    should also send in the QAudioFormat to be used for the playback
    (see the QAudioFormat class description for details).

    To play a file:

    Starting to play an audio stream is simply a matter of calling
    start() with a QIODevice. QAudioOutput will then fetch the data it
    needs from the io device. So playing back an audio file is as
    simple as:

    \snippet doc/src/snippets/audio/main.cpp 9
    \dots 4
    \snippet doc/src/snippets/audio/main.cpp 4

    The file will start playing assuming that the audio system and
    output device support it. If you run out of luck, check what's
    up with the error() function.

    After the file has finished playing, we need to stop the device:

    \snippet doc/src/snippets/audio/main.cpp 5

    At any given time, the QAudioOutput will be in one of four states:
    active, suspended, stopped, or idle. These states are described
    by the QAudio::State enum.
    State changes are reported through the stateChanged() signal. You
    can use this signal to, for instance, update the GUI of the
    application; the mundane example here being changing the state of
    a \c { play/pause } button. You request a state change directly
    with suspend(), stop(), reset(), resume(), and start().

    While the stream is playing, you can set a notify interval in
    milliseconds with setNotifyInterval(). This interval specifies the
    time between two emissions of the notify() signal. This is
    relative to the position in the stream, i.e., if the QAudioOutput
    is in the SuspendedState or the IdleState, the notify() signal is
    not emitted. A typical use-case would be to update a
    \l{QSlider}{slider} that allows seeking in the stream.
    If you want the time since playback started regardless of which
    states the audio output has been in, elapsedUSecs() is the function for you.

    If an error occurs, you can fetch the \l{QAudio::Error}{error
    type} with the error() function. Please see the QAudio::Error enum
    for a description of the possible errors that are reported.  When
    an error is encountered, the state changes to QAudio::StoppedState.
    You can check for errors by connecting to the stateChanged()
    signal:

    \snippet doc/src/snippets/audio/main.cpp 8

    \sa QAudioInput, QAudioDeviceInfo
*/

/*!
    Construct a new audio output and attach it to \a parent.
    The default audio output device is used with the output
    \a format parameters.
*/

QAudioOutput::QAudioOutput(const QAudioFormat &format, QObject *parent):
    QObject(parent)
{
    d = QAudioDeviceFactory::createDefaultOutputDevice(format);
    connect(d, SIGNAL(notify()), SIGNAL(notify()));
    connect(d, SIGNAL(stateChanged(QAudio::State)), SIGNAL(stateChanged(QAudio::State)));
}

/*!
    Construct a new audio output and attach it to \a parent.
    The device referenced by \a audioDevice is used with the output
    \a format parameters.
*/

QAudioOutput::QAudioOutput(const QAudioDeviceInfo &audioDevice, const QAudioFormat &format, QObject *parent):
    QObject(parent)
{
    d = QAudioDeviceFactory::createOutputDevice(audioDevice, format);
    connect(d, SIGNAL(notify()), SIGNAL(notify()));
    connect(d, SIGNAL(stateChanged(QAudio::State)), SIGNAL(stateChanged(QAudio::State)));
}

/*!
    Destroys this audio output.
*/

QAudioOutput::~QAudioOutput()
{
    delete d;
}

/*!
    Returns the QAudioFormat being used.

*/

QAudioFormat QAudioOutput::format() const
{
    return d->format();
}

/*!
    Uses the \a device as the QIODevice to transfer data.
    Passing a QIODevice allows the data to be transferred without any extra code.
    All that is required is to open the QIODevice.

    If able to successfully output audio data to the systems audio device the
    state() is set to QAudio::ActiveState, error() is set to QAudio::NoError
    and the stateChanged() signal is emitted.

    If a problem occurs during this process the error() is set to QAudio::OpenError,
    state() is set to QAudio::StoppedState and stateChanged() signal is emitted.

    In either case, the stateChanged() signal may be emitted either synchronously
    during execution of the start() function or asynchronously after start() has
    returned to the caller.

    \sa QIODevice
*/

void QAudioOutput::start(QIODevice* device)
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

QIODevice* QAudioOutput::start()
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
