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
#include <QtMultimedia/qaudioinput.h>

#include <qaudiodevicefactory_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAudioInput
    \brief The QAudioInput class provides an interface for receiving audio data from an audio input device.

    \inmodule QtMultimedia
    \ingroup  multimedia
    \since 4.6

    You can construct an audio input with the system's
    \l{QAudioDeviceInfo::defaultInputDevice()}{default audio input
    device}. It is also possible to create QAudioInput with a
    specific QAudioDeviceInfo. When you create the audio input, you
    should also send in the QAudioFormat to be used for the recording
    (see the QAudioFormat class description for details).

    To record to a file:

    QAudioInput lets you record audio with an audio input device. The
    default constructor of this class will use the systems default
    audio device, but you can also specify a QAudioDeviceInfo for a
    specific device. You also need to pass in the QAudioFormat in
    which you wish to record.

    Starting up the QAudioInput is simply a matter of calling start()
    with a QIODevice opened for writing. For instance, to record to a
    file, you can:

    \snippet doc/src/snippets/audio/main.cpp 3
    \dots 4
    \snippet doc/src/snippets/audio/main.cpp 0

    This will start recording if the format specified is supported by
    the input device (you can check this with
    QAudioDeviceInfo::isFormatSupported(). In case there are any
    snags, use the error() function to check what went wrong. We stop
    recording in the \c stopRecording() slot.

    \snippet doc/src/snippets/audio/main.cpp 1

    At any point in time, QAudioInput will be in one of four states:
    active, suspended, stopped, or idle. These states are specified by
    the QAudio::State enum. You can request a state change directly through
    suspend(), resume(), stop(), reset(), and start(). The current
    state is reported by state(). QAudioOutput will also signal you
    when the state changes (stateChanged()).

    QAudioInput provides several ways of measuring the time that has
    passed since the start() of the recording. The \c processedUSecs()
    function returns the length of the stream in microseconds written,
    i.e., it leaves out the times the audio input was suspended or idle.
    The elapsedUSecs() function returns the time elapsed since start() was called regardless of
    which states the QAudioInput has been in.

    If an error should occur, you can fetch its reason with error().
    The possible error reasons are described by the QAudio::Error
    enum. The QAudioInput will enter the \l{QAudio::}{StoppedState} when
    an error is encountered.  Connect to the stateChanged() signal to
    handle the error:

    \snippet doc/src/snippets/audio/main.cpp 2

    \sa QAudioOutput, QAudioDeviceInfo

    \section1 Symbian Platform Security Requirements

    On Symbian, processes which use this class must have the
    \c UserEnvironment platform security capability.  If the client
    process lacks this capability, calls to either overload of start()
    will fail.
    This failure is indicated by the QAudioInput object setting
    its error() value to \l{QAudio::OpenError} and then emitting a
    \l{stateChanged()}{stateChanged}(\l{QAudio::StoppedState}) signal.

    Platform security capabilities are added via the
    \l{qmake-variable-reference.html#target-capability}{TARGET.CAPABILITY}
    qmake variable.
*/

/*!
    Construct a new audio input and attach it to \a parent.
    The default audio input device is used with the output
    \a format parameters.
*/

QAudioInput::QAudioInput(const QAudioFormat &format, QObject *parent):
   QObject(parent)
{
   d = QAudioDeviceFactory::createDefaultInputDevice(format);
   connect(d, SIGNAL(notify()), this, SLOT(emitNotify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(emitStateChanged(QAudio::State)));
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
   connect(d, SIGNAL(notify()), this, SLOT(emitNotify()));
   connect(d, SIGNAL(stateChanged(QAudio::State)), this, SLOT(emitStateChanged(QAudio::State)));
}

/*!
    Destroy this audio input.
*/

QAudioInput::~QAudioInput()
{
   delete d;
}

/*!
     Uses the \a device as the QIODevice to transfer data.
     Passing a QIODevice allows the data to be transferred without any extra code.
     All that is required is to open the QIODevice. QAudioInput does not take
     ownership of \a device.

     The QAudioInput will write to the device when new data is available. You can
     subclass QIODevice and reimplement \l{QIODevice::}{writeData()} if you wish to
     access the data. If you simply want to save data to a file, you can pass a
     QFile to this function.

     If able to successfully get audio data from the systems audio device the
     state() is set to either QAudio::ActiveState or QAudio::IdleState,
     error() is set to QAudio::NoError and the stateChanged() signal is emitted.

     If a problem occurs during this process the error() is set to QAudio::OpenError,
     state() is set to QAudio::StoppedState and stateChanged() signal is emitted.

     \l{QAudioInput#Symbian Platform Security Requirements}

     \sa QIODevice
*/

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
   return d->start(0);
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

/*!
    Returns the amount of audio data processed since start()
    was called in microseconds.
*/

qint64 QAudioInput::processedUSecs() const
{
   return d->processedUSecs();
}

/*!
    Returns the microseconds since start() was called, including time in Idle and
    Suspend states.
*/

qint64 QAudioInput::elapsedUSecs() const
{
   return d->elapsedUSecs();
}

/*!
    Returns the error state.
*/

QAudio::Error QAudioInput::error() const
{
   return d->error();
}

/*!
    Returns the state of audio processing.
*/

QAudio::State QAudioInput::state() const
{
   return d->state();
}

/*!
    \fn QAudioInput::stateChanged(QAudio::State state)
    This signal is emitted when the device \a state has changed.
*/

/*!
    \fn QAudioInput::notify()
    This signal is emitted when x ms of audio data has been processed
    the interval set by setNotifyInterval(x).
*/

QT_END_NAMESPACE

