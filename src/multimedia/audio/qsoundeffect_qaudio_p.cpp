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

#include <qsoundeffect_qaudio_p.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qiodevice.h>

static QSampleCache *sampleCache()
{
   static QSampleCache retval;
   return &retval;
}

QSoundEffectPrivate::QSoundEffectPrivate(QObject *parent)
   : QObject(parent), m_soundSource(new PrivateSoundSource(this))
{
}

QSoundEffectPrivate::~QSoundEffectPrivate()
{
}

void QSoundEffectPrivate::release()
{
   stop();

   if (m_soundSource->m_audioOutput) {
      m_soundSource->m_audioOutput->stop();
      m_soundSource->m_audioOutput->deleteLater();
      m_soundSource->m_sample->release();
   }

   delete m_soundSource;
   this->deleteLater();
}

QStringList QSoundEffectPrivate::supportedMimeTypes()
{
   // Only return supported mime types if we have a audio device available
   const QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

   if (devices.size() <= 0) {
      return QStringList();
   }

   return QStringList( { "audio/x-wav", "audio/wav", "audio/wave", "audio/x-pn-wav"} );
}

QUrl QSoundEffectPrivate::source() const
{
   return m_soundSource->m_url;
}

void QSoundEffectPrivate::setSource(const QUrl &url)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "setSource current=" << m_soundSource->m_url << ", to=" << url;
#endif

   Q_ASSERT(m_soundSource->m_url != url);

   stop();

   m_soundSource->m_url         = url;
   m_soundSource->m_sampleReady = false;

   if (url.isEmpty()) {
      setStatus(QSoundEffect::Null);
      return;
   }

   if (! url.isValid()) {
      setStatus(QSoundEffect::Error);
      return;
   }

   if (m_soundSource->m_sample) {
      if (! m_soundSource->m_sampleReady) {
         disconnect(m_soundSource->m_sample, &QSample::error, m_soundSource, &PrivateSoundSource::decoderError);
         disconnect(m_soundSource->m_sample, &QSample::ready, m_soundSource, &PrivateSoundSource::sampleReady);
      }

      m_soundSource->m_sample->release();
      m_soundSource->m_sample = nullptr;
   }

   setStatus(QSoundEffect::Loading);
   m_soundSource->m_sample = sampleCache()->requestSample(url);

   connect(m_soundSource->m_sample, &QSample::error, m_soundSource, &PrivateSoundSource::decoderError);
   connect(m_soundSource->m_sample, &QSample::ready, m_soundSource, &PrivateSoundSource::sampleReady);

   switch (m_soundSource->m_sample->state()) {
      case QSample::Ready:
         m_soundSource->sampleReady();
         break;

      case QSample::Error:
         m_soundSource->decoderError();
         break;

      default:
         break;
   }
}

int QSoundEffectPrivate::loopCount() const
{
   return m_soundSource->m_loopCount;
}

int QSoundEffectPrivate::loopsRemaining() const
{
   return m_soundSource->m_runningCount;
}

void QSoundEffectPrivate::setLoopCount(int loopCount)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "setLoopCount " << loopCount;
#endif

   if (loopCount == 0) {
      loopCount = 1;
   }

   m_soundSource->m_loopCount = loopCount;

   if (m_soundSource->m_playing) {
      setLoopsRemaining(loopCount);
   }
}

qreal QSoundEffectPrivate::volume() const
{
   if (m_soundSource->m_audioOutput && !m_soundSource->m_muted) {
      return m_soundSource->m_audioOutput->volume();
   }

   return m_soundSource->m_volume;
}

void QSoundEffectPrivate::setVolume(qreal volume)
{
   m_soundSource->m_volume = volume;

   if (m_soundSource->m_audioOutput && !m_soundSource->m_muted) {
      m_soundSource->m_audioOutput->setVolume(volume);
   }

   emit volumeChanged();
}

bool QSoundEffectPrivate::isMuted() const
{
   return m_soundSource->m_muted;
}

void QSoundEffectPrivate::setMuted(bool muted)
{
   if (muted && m_soundSource->m_audioOutput) {
      m_soundSource->m_audioOutput->setVolume(0);
   } else if (!muted && m_soundSource->m_audioOutput && m_soundSource->m_muted) {
      m_soundSource->m_audioOutput->setVolume(m_soundSource->m_volume);
   }

   m_soundSource->m_muted = muted;
   emit mutedChanged();
}

bool QSoundEffectPrivate::isLoaded() const
{
   return m_soundSource->m_status == QSoundEffect::Ready;
}

bool QSoundEffectPrivate::isPlaying() const
{
   return m_soundSource->m_playing;
}

QSoundEffect::Status QSoundEffectPrivate::status() const
{
   return m_soundSource->m_status;
}

void QSoundEffectPrivate::play()
{
   m_soundSource->m_offset = 0;
   setLoopsRemaining(m_soundSource->m_loopCount);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "play";
#endif

   if (m_soundSource->m_status == QSoundEffect::Null || m_soundSource->m_status == QSoundEffect::Error) {
      setStatus(QSoundEffect::Null);
      return;
   }
   setPlaying(true);
   if (m_soundSource->m_audioOutput && m_soundSource->m_audioOutput->state() == QAudio::StoppedState && m_soundSource->m_sampleReady) {
      m_soundSource->m_audioOutput->start(m_soundSource);
   }
}

void QSoundEffectPrivate::stop()
{
   if (!m_soundSource->m_playing) {
      return;
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "stop()";
#endif

   m_soundSource->m_offset = 0;

   setPlaying(false);

   if (m_soundSource->m_audioOutput) {
      m_soundSource->m_audioOutput->stop();
   }
}

void QSoundEffectPrivate::setStatus(QSoundEffect::Status status)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "setStatus" << status;
#endif

   if (m_soundSource->m_status == status) {
      return;
   }
   bool oldLoaded = isLoaded();
   m_soundSource->m_status = status;

   emit statusChanged();

   if (oldLoaded != isLoaded()) {
      emit loadedChanged();
   }
}

void QSoundEffectPrivate::setPlaying(bool playing)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "setPlaying(" << playing << ")";
#endif

   if (m_soundSource->m_playing == playing) {
      return;
   }

   m_soundSource->m_playing = playing;
   emit playingChanged();
}

void QSoundEffectPrivate::setLoopsRemaining(int loopsRemaining)
{
   if (m_soundSource->m_runningCount == loopsRemaining) {
      return;
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "setLoopsRemaining " << loopsRemaining;
#endif

   m_soundSource->m_runningCount = loopsRemaining;
   emit loopsRemainingChanged();
}

/* Categories are ignored */
QString QSoundEffectPrivate::category() const
{
   return m_soundSource->m_category;
}

void QSoundEffectPrivate::setCategory(const QString &category)
{
   if (m_soundSource->m_category != category && ! m_soundSource->m_playing) {
      m_soundSource->m_category = category;
      emit categoryChanged();
   }
}

PrivateSoundSource::PrivateSoundSource(QSoundEffectPrivate *s)
   : QIODevice(s), m_loopCount(1), m_runningCount(0), m_playing(false), m_status(QSoundEffect::Null),
     m_audioOutput(nullptr), m_sample(nullptr), m_muted(false), m_volume(1.0), m_sampleReady(false), m_offset(0)
{
   soundeffect = s;
   m_category  = QString("game");
   open(QIODevice::ReadOnly);
}

void PrivateSoundSource::sampleReady()
{
   if (m_status == QSoundEffect::Error) {
      return;
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "sampleReady " << m_playing;
#endif

   disconnect(m_sample, &QSample::error, this, &PrivateSoundSource::decoderError);
   disconnect(m_sample, &QSample::ready, this, &PrivateSoundSource::sampleReady);

   if (! m_audioOutput) {
      m_audioOutput = new QAudioOutput(m_sample->format());

      connect(m_audioOutput, &QAudioOutput::stateChanged, this, &PrivateSoundSource::stateChanged);

      if (! m_muted) {
         m_audioOutput->setVolume(m_volume);
      } else {
         m_audioOutput->setVolume(0);
      }
   }

   m_sampleReady = true;
   soundeffect->setStatus(QSoundEffect::Ready);

   if (m_playing) {
      m_audioOutput->start(this);
   }
}

void PrivateSoundSource::decoderError()
{
   qWarning("QSoundEffect(qaudio): Error decoding source");

   disconnect(m_sample, &QSample::error, this, &PrivateSoundSource::decoderError);
   disconnect(m_sample, &QSample::ready, this, &PrivateSoundSource::sampleReady);

   m_playing = false;
   soundeffect->setStatus(QSoundEffect::Error);
}

void PrivateSoundSource::stateChanged(QAudio::State state)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << this << "stateChanged " << state;
#endif

   if ((state == QAudio::IdleState && m_runningCount == 0)
         || (state == QAudio::StoppedState && m_audioOutput->error() != QAudio::NoError)) {
      emit soundeffect->stop();
   }
}

qint64 PrivateSoundSource::readData( char *data, qint64 len)
{
   if ((m_runningCount > 0  || m_runningCount == QSoundEffect::Infinite) && m_playing) {

      if (m_sample->state() != QSample::Ready) {
         return 0;
      }

      qint64 bytesWritten = 0;

      const int   periodSize = m_audioOutput->periodSize();
      const int   sampleSize = m_sample->data().size();
      const char *sampleData = m_sample->data().constData();

      // Some systems can have large buffers we only need a max of three
      int    periodsFree = qMin(3, (int)(m_audioOutput->bytesFree() / periodSize));
      int    dataOffset = 0;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
      qDebug() << "bytesFree=" << m_audioOutput->bytesFree() << ", can fit " << periodsFree << " periodSize() chunks";
#endif

      while ((periodsFree > 0) && (bytesWritten + periodSize <= len)) {

         if (sampleSize - m_offset >= periodSize) {
            // We can fit a whole period of data
            memcpy(data + dataOffset, sampleData + m_offset, periodSize);
            m_offset += periodSize;
            dataOffset += periodSize;
            bytesWritten += periodSize;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << "WHOLE PERIOD: bytesWritten=" << bytesWritten << ", offset=" << m_offset
               << ", filesize=" << sampleSize;
#endif

         } else {
            // We are at end of sound, first write what is left of current sound
            memcpy(data + dataOffset, sampleData + m_offset, sampleSize - m_offset);
            bytesWritten += sampleSize - m_offset;

            int wrapLen = periodSize - (sampleSize - m_offset);

            if (wrapLen > sampleSize) {
               wrapLen = sampleSize;
            }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << "END OF SOUND: bytesWritten=" << bytesWritten << ", offset=" << m_offset
               << ", part1=" << (sampleSize - m_offset);
#endif

            dataOffset += (sampleSize - m_offset);
            m_offset = 0;

            if (m_runningCount > 0 && m_runningCount != QSoundEffect::Infinite) {
               soundeffect->setLoopsRemaining(m_runningCount - 1);
            }

            if (m_runningCount > 0 || m_runningCount == QSoundEffect::Infinite) {
               // There are still more loops of this sound to play, append the start of sound to make up full period
               memcpy(data + dataOffset, sampleData + m_offset, wrapLen);
               m_offset += wrapLen;
               dataOffset += wrapLen;
               bytesWritten += wrapLen;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
               qDebug() << "APPEND START FOR FULL PERIOD: bytesWritten=" << bytesWritten << ", offset=" << m_offset
                  << ", part2=" << wrapLen;

               qDebug() << "part1 + part2 should be a period " << periodSize;
#endif
            }
         }

         if (m_runningCount == 0) {
            break;
         }

         periodsFree--;
      }

      return bytesWritten;
   }

   return 0;
}

qint64 PrivateSoundSource::writeData(const char *data, qint64 len)
{
   (void) data;
   (void) len;

   return 0;
}
