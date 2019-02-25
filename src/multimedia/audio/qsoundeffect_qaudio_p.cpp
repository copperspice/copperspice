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

#include <qsoundeffect_qaudio_p.h>

#include <qcoreapplication.h>
#include <qiodevice.h>

//#include <QDebug>
//#define QT_QAUDIO_DEBUG 1

Q_GLOBAL_STATIC(QSampleCache, sampleCache)

QSoundEffectPrivate::QSoundEffectPrivate(QObject* parent):
    QObject(parent),
    d(new PrivateSoundSource(this))
{
}

QSoundEffectPrivate::~QSoundEffectPrivate()
{
}

void QSoundEffectPrivate::release()
{
    stop();
    if (d->m_audioOutput) {
        d->m_audioOutput->stop();
        d->m_audioOutput->deleteLater();
        d->m_sample->release();
    }
    delete d;
    this->deleteLater();
}

QStringList QSoundEffectPrivate::supportedMimeTypes()
{
    // Only return supported mime types if we have a audio device available
    const QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    if (devices.size() <= 0)
        return QStringList();

    return QStringList() << QLatin1String("audio/x-wav")
                         << QLatin1String("audio/wav")
                         << QLatin1String("audio/wave")
                         << QLatin1String("audio/x-pn-wav");
}

QUrl QSoundEffectPrivate::source() const
{
    return d->m_url;
}

void QSoundEffectPrivate::setSource(const QUrl &url)
{
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "setSource current=" << d->m_url << ", to=" << url;
#endif
    Q_ASSERT(d->m_url != url);

    stop();

    d->m_url = url;

    d->m_sampleReady = false;

    if (url.isEmpty()) {
        setStatus(QSoundEffect::Null);
        return;
    }

    if (!url.isValid()) {
        setStatus(QSoundEffect::Error);
        return;
    }

    if (d->m_sample) {
        if (!d->m_sampleReady) {
            disconnect(d->m_sample, SIGNAL(error()), d, SLOT(decoderError()));
            disconnect(d->m_sample, SIGNAL(ready()), d, SLOT(sampleReady()));
        }
        d->m_sample->release();
        d->m_sample = 0;
    }

    setStatus(QSoundEffect::Loading);
    d->m_sample = sampleCache()->requestSample(url);
    connect(d->m_sample, SIGNAL(error()), d, SLOT(decoderError()));
    connect(d->m_sample, SIGNAL(ready()), d, SLOT(sampleReady()));

    switch (d->m_sample->state()) {
    case QSample::Ready:
        d->sampleReady();
        break;
    case QSample::Error:
        d->decoderError();
        break;
    default:
        break;
    }
}

int QSoundEffectPrivate::loopCount() const
{
    return d->m_loopCount;
}

int QSoundEffectPrivate::loopsRemaining() const
{
    return d->m_runningCount;
}

void QSoundEffectPrivate::setLoopCount(int loopCount)
{
#ifdef QT_QAUDIO_DEBUG
    qDebug() << "setLoopCount " << loopCount;
#endif
    if (loopCount == 0)
        loopCount = 1;
    d->m_loopCount = loopCount;
    if (d->m_playing)
        setLoopsRemaining(loopCount);
}

qreal QSoundEffectPrivate::volume() const
{
    if (d->m_audioOutput && !d->m_muted)
        return d->m_audioOutput->volume();

    return d->m_volume;
}

void QSoundEffectPrivate::setVolume(qreal volume)
{
    d->m_volume = volume;

    if (d->m_audioOutput && !d->m_muted)
        d->m_audioOutput->setVolume(volume);

    emit volumeChanged();
}

bool QSoundEffectPrivate::isMuted() const
{
    return d->m_muted;
}

void QSoundEffectPrivate::setMuted(bool muted)
{
    if (muted && d->m_audioOutput)
        d->m_audioOutput->setVolume(0);
    else if (!muted && d->m_audioOutput && d->m_muted)
        d->m_audioOutput->setVolume(d->m_volume);

    d->m_muted = muted;
    emit mutedChanged();
}

bool QSoundEffectPrivate::isLoaded() const
{
    return d->m_status == QSoundEffect::Ready;
}


bool QSoundEffectPrivate::isPlaying() const
{
    return d->m_playing;
}

QSoundEffect::Status QSoundEffectPrivate::status() const
{
    return d->m_status;
}

void QSoundEffectPrivate::play()
{
    d->m_offset = 0;
    setLoopsRemaining(d->m_loopCount);
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "play";
#endif
    if (d->m_status == QSoundEffect::Null || d->m_status == QSoundEffect::Error) {
        setStatus(QSoundEffect::Null);
        return;
    }
    setPlaying(true);
    if (d->m_audioOutput && d->m_audioOutput->state() == QAudio::StoppedState && d->m_sampleReady)
        d->m_audioOutput->start(d);
}

void QSoundEffectPrivate::stop()
{
    if (!d->m_playing)
        return;
#ifdef QT_QAUDIO_DEBUG
    qDebug() << "stop()";
#endif
    d->m_offset = 0;

    setPlaying(false);

    if (d->m_audioOutput)
        d->m_audioOutput->stop();
}

void QSoundEffectPrivate::setStatus(QSoundEffect::Status status)
{
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "setStatus" << status;
#endif
    if (d->m_status == status)
        return;
    bool oldLoaded = isLoaded();
    d->m_status = status;
    emit statusChanged();
    if (oldLoaded != isLoaded())
        emit loadedChanged();
}

void QSoundEffectPrivate::setPlaying(bool playing)
{
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "setPlaying(" << playing << ")";
#endif
    if (d->m_playing == playing)
        return;
    d->m_playing = playing;
    emit playingChanged();
}

void QSoundEffectPrivate::setLoopsRemaining(int loopsRemaining)
{
    if (d->m_runningCount == loopsRemaining)
        return;
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "setLoopsRemaining " << loopsRemaining;
#endif
    d->m_runningCount = loopsRemaining;
    emit loopsRemainingChanged();
}

/* Categories are ignored */
QString QSoundEffectPrivate::category() const
{
    return d->m_category;
}

void QSoundEffectPrivate::setCategory(const QString &category)
{
    if (d->m_category != category && !d->m_playing) {
        d->m_category = category;
        emit categoryChanged();
    }
}

PrivateSoundSource::PrivateSoundSource(QSoundEffectPrivate* s):
    QIODevice(s),
    m_loopCount(1),
    m_runningCount(0),
    m_playing(false),
    m_status(QSoundEffect::Null),
    m_audioOutput(0),
    m_sample(0),
    m_muted(false),
    m_volume(1.0),
    m_sampleReady(false),
    m_offset(0)
{
    soundeffect = s;
    m_category = QLatin1String("game");
    open(QIODevice::ReadOnly);
}

void PrivateSoundSource::sampleReady()
{
    if (m_status == QSoundEffect::Error)
        return;

#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "sampleReady "<<m_playing;
#endif
    disconnect(m_sample, SIGNAL(error()), this, SLOT(decoderError()));
    disconnect(m_sample, SIGNAL(ready()), this, SLOT(sampleReady()));
    if (!m_audioOutput) {
        m_audioOutput = new QAudioOutput(m_sample->format());
        connect(m_audioOutput,SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
        if (!m_muted)
            m_audioOutput->setVolume(m_volume);
        else
            m_audioOutput->setVolume(0);
    }
    m_sampleReady = true;
    soundeffect->setStatus(QSoundEffect::Ready);

    if (m_playing)
        m_audioOutput->start(this);
}

void PrivateSoundSource::decoderError()
{
    qWarning("QSoundEffect(qaudio): Error decoding source");
    disconnect(m_sample, SIGNAL(ready()), this, SLOT(sampleReady()));
    disconnect(m_sample, SIGNAL(error()), this, SLOT(decoderError()));
    m_playing = false;
    soundeffect->setStatus(QSoundEffect::Error);
}

void PrivateSoundSource::stateChanged(QAudio::State state)
{
#ifdef QT_QAUDIO_DEBUG
    qDebug() << this << "stateChanged " << state;
#endif
    if ((state == QAudio::IdleState && m_runningCount == 0)
         || (state == QAudio::StoppedState && m_audioOutput->error() != QAudio::NoError))
        emit soundeffect->stop();
}

qint64 PrivateSoundSource::readData( char* data, qint64 len)
{
    if ((m_runningCount > 0  || m_runningCount == QSoundEffect::Infinite) && m_playing) {

        if (m_sample->state() != QSample::Ready)
            return 0;

        qint64 bytesWritten = 0;

        const int   periodSize = m_audioOutput->periodSize();
        const int   sampleSize = m_sample->data().size();
        const char* sampleData = m_sample->data().constData();

        // Some systems can have large buffers we only need a max of three
        int    periodsFree = qMin(3, (int)(m_audioOutput->bytesFree()/periodSize));
        int    dataOffset = 0;

#ifdef QT_QAUDIO_DEBUG
        qDebug() << "bytesFree=" << m_audioOutput->bytesFree() << ", can fit " << periodsFree << " periodSize() chunks";
#endif

        while ((periodsFree > 0) && (bytesWritten + periodSize <= len)) {

            if (sampleSize - m_offset >= periodSize) {
                // We can fit a whole period of data
                memcpy(data + dataOffset, sampleData + m_offset, periodSize);
                m_offset += periodSize;
                dataOffset += periodSize;
                bytesWritten += periodSize;
#ifdef QT_QAUDIO_DEBUG
                qDebug() << "WHOLE PERIOD: bytesWritten=" << bytesWritten << ", offset=" << m_offset
                         << ", filesize=" << sampleSize;
#endif
            } else {
                // We are at end of sound, first write what is left of current sound
                memcpy(data + dataOffset, sampleData + m_offset, sampleSize - m_offset);
                bytesWritten += sampleSize - m_offset;
                int wrapLen = periodSize - (sampleSize - m_offset);
                if (wrapLen > sampleSize)
                    wrapLen = sampleSize;
#ifdef QT_QAUDIO_DEBUG
                qDebug() << "END OF SOUND: bytesWritten=" << bytesWritten << ", offset=" << m_offset
                         << ", part1=" << (sampleSize-m_offset);
#endif
                dataOffset += (sampleSize - m_offset);
                m_offset = 0;

                if (m_runningCount > 0 && m_runningCount != QSoundEffect::Infinite)
                    soundeffect->setLoopsRemaining(m_runningCount-1);

                if (m_runningCount > 0 || m_runningCount == QSoundEffect::Infinite) {
                    // There are still more loops of this sound to play, append the start of sound to make up full period
                    memcpy(data + dataOffset, sampleData + m_offset, wrapLen);
                    m_offset += wrapLen;
                    dataOffset += wrapLen;
                    bytesWritten += wrapLen;
#ifdef QT_QAUDIO_DEBUG
                    qDebug() << "APPEND START FOR FULL PERIOD: bytesWritten=" << bytesWritten << ", offset=" << m_offset
                             << ", part2=" << wrapLen;
                    qDebug() << "part1 + part2 should be a period " << periodSize;
#endif
                }
            }
            if (m_runningCount == 0)
                break;

            periodsFree--;
        }
        return bytesWritten;
    }

    return 0;
}

qint64 PrivateSoundSource::writeData(const char* data, qint64 len)
{
    return 0;
}
