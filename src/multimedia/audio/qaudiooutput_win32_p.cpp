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

#include <qaudiooutput_win32_p.h>
#include <QtEndian>

#ifndef SPEAKER_FRONT_LEFT
#define SPEAKER_FRONT_LEFT            0x00000001
#define SPEAKER_FRONT_RIGHT           0x00000002
#define SPEAKER_FRONT_CENTER          0x00000004
#define SPEAKER_LOW_FREQUENCY         0x00000008
#define SPEAKER_BACK_LEFT             0x00000010
#define SPEAKER_BACK_RIGHT            0x00000020
#define SPEAKER_FRONT_LEFT_OF_CENTER  0x00000040
#define SPEAKER_FRONT_RIGHT_OF_CENTER 0x00000080
#define SPEAKER_BACK_CENTER           0x00000100
#define SPEAKER_SIDE_LEFT             0x00000200
#define SPEAKER_SIDE_RIGHT            0x00000400
#define SPEAKER_TOP_CENTER            0x00000800
#define SPEAKER_TOP_FRONT_LEFT        0x00001000
#define SPEAKER_TOP_FRONT_CENTER      0x00002000
#define SPEAKER_TOP_FRONT_RIGHT       0x00004000
#define SPEAKER_TOP_BACK_LEFT         0x00008000
#define SPEAKER_TOP_BACK_CENTER       0x00010000
#define SPEAKER_TOP_BACK_RIGHT        0x00020000
#define SPEAKER_RESERVED              0x7FFC0000
#define SPEAKER_ALL                   0x80000000
#endif

#ifndef _WAVEFORMATEXTENSIBLE_

#define _WAVEFORMATEXTENSIBLE_
typedef struct {
   WAVEFORMATEX Format;          // Base WAVEFORMATEX data
   union {
      WORD wValidBitsPerSample; // Valid bits in each sample container
      WORD wSamplesPerBlock;    // Samples per block of audio data; valid
      // if wBitsPerSample=0 (but rarely used).
      WORD wReserved;           // Zero if neither case above applies.
   } Samples;
   DWORD dwChannelMask;          // Positions of the audio channels
   GUID SubFormat;               // Format identifier GUID
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE, *LPPWAVEFORMATEXTENSIBLE;
typedef const WAVEFORMATEXTENSIBLE *LPCWAVEFORMATEXTENSIBLE;

#endif

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

//#define DEBUG_AUDIO 1

QT_BEGIN_NAMESPACE

QAudioOutputPrivate::QAudioOutputPrivate(const QByteArray &device, const QAudioFormat &audioFormat):
   settings(audioFormat)
{
   bytesAvailable = 0;
   buffer_size = 0;
   period_size = 0;
   m_device = device;
   totalTimeValue = 0;
   intervalTime = 1000;
   audioBuffer = 0;
   errorState = QAudio::NoError;
   deviceState = QAudio::StoppedState;
   audioSource = 0;
   pullMode = true;
   finished = false;
}

QAudioOutputPrivate::~QAudioOutputPrivate()
{
   mutex.lock();
   finished = true;
   mutex.unlock();

   close();
}

void CALLBACK QAudioOutputPrivate::waveOutProc( HWAVEOUT hWaveOut, UINT uMsg,
      DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
   Q_UNUSED(dwParam1)
   Q_UNUSED(dwParam2)
   Q_UNUSED(hWaveOut)

   QAudioOutputPrivate *qAudio;
   qAudio = (QAudioOutputPrivate *)(dwInstance);
   if (!qAudio) {
      return;
   }

   QMutexLocker(&qAudio->mutex);

   switch (uMsg) {
      case WOM_OPEN:
         qAudio->feedback();
         break;
      case WOM_CLOSE:
         return;
      case WOM_DONE:
         if (qAudio->finished || qAudio->buffer_size == 0 || qAudio->period_size == 0) {
            return;
         }
         qAudio->waveFreeBlockCount++;
         if (qAudio->waveFreeBlockCount >= qAudio->buffer_size / qAudio->period_size) {
            qAudio->waveFreeBlockCount = qAudio->buffer_size / qAudio->period_size;
         }
         qAudio->feedback();
         break;
      default:
         return;
   }
}

WAVEHDR *QAudioOutputPrivate::allocateBlocks(int size, int count)
{
   int i;
   unsigned char *buffer;
   WAVEHDR *blocks;
   DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

   if ((buffer = (unsigned char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                 totalBufferSize)) == 0) {
      qWarning("QAudioOutput: Memory allocation error");
      return 0;
   }
   blocks = (WAVEHDR *)buffer;
   buffer += sizeof(WAVEHDR) * count;
   for (i = 0; i < count; i++) {
      blocks[i].dwBufferLength = size;
      blocks[i].lpData = (LPSTR)buffer;
      buffer += size;
   }
   return blocks;
}

void QAudioOutputPrivate::freeBlocks(WAVEHDR *blockArray)
{
   WAVEHDR *blocks = blockArray;

   int count = buffer_size / period_size;

   for (int i = 0; i < count; i++) {
      waveOutUnprepareHeader(hWaveOut, blocks, sizeof(WAVEHDR));
      blocks++;
   }
   HeapFree(GetProcessHeap(), 0, blockArray);
}

QAudioFormat QAudioOutputPrivate::format() const
{
   return settings;
}

QIODevice *QAudioOutputPrivate::start(QIODevice *device)
{
   if (deviceState != QAudio::StoppedState) {
      close();
   }

   if (!pullMode && audioSource) {
      delete audioSource;
   }

   if (device) {
      //set to pull mode
      pullMode = true;
      audioSource = device;
      deviceState = QAudio::ActiveState;
   } else {
      //set to push mode
      pullMode = false;
      audioSource = new OutputPrivate(this);
      audioSource->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
      deviceState = QAudio::IdleState;
   }

   if ( !open() ) {
      return 0;
   }

   emit stateChanged(deviceState);

   return audioSource;
}

void QAudioOutputPrivate::stop()
{
   if (deviceState == QAudio::StoppedState) {
      return;
   }
   close();
   if (!pullMode && audioSource) {
      delete audioSource;
      audioSource = 0;
   }
   emit stateChanged(deviceState);
}

bool QAudioOutputPrivate::open()
{
#ifdef DEBUG_AUDIO
   QTime now(QTime::currentTime());
   qDebug() << now.second() << "s " << now.msec() << "ms :open()";
#endif

   period_size = 0;

   if (!settings.isValid()) {
      qWarning("QAudioOutput: open error, invalid format.");
   } else if (settings.channels() <= 0) {
      qWarning("QAudioOutput: open error, invalid number of channels (%d).",
               settings.channels());
   } else if (settings.sampleSize() <= 0) {
      qWarning("QAudioOutput: open error, invalid sample size (%d).",
               settings.sampleSize());
   } else if (settings.frequency() < 8000 || settings.frequency() > 96000) {
      qWarning("QAudioOutput: open error, frequency out of range (%d).", settings.frequency());
   } else if (buffer_size == 0) {
      // Default buffer size, 200ms, default period size is 40ms
      buffer_size
         = (settings.frequency()
            * settings.channels()
            * settings.sampleSize()
            + 39) / 40;
      period_size = buffer_size / 5;
   } else {
      period_size = buffer_size / 5;
   }

   if (period_size == 0) {
      errorState = QAudio::OpenError;
      deviceState = QAudio::StoppedState;
      emit stateChanged(deviceState);
      return false;
   }

   waveBlocks = allocateBlocks(period_size, buffer_size / period_size);

   mutex.lock();
   waveFreeBlockCount = buffer_size / period_size;
   mutex.unlock();

   waveCurrentBlock = 0;

   if (audioBuffer == 0) {
      audioBuffer = new char[buffer_size];
   }

   timeStamp.restart();
   elapsedTimeOffset = 0;

   wfx.nSamplesPerSec = settings.frequency();
   wfx.wBitsPerSample = settings.sampleSize();
   wfx.nChannels = settings.channels();
   wfx.cbSize = 0;

   wfx.wFormatTag = WAVE_FORMAT_PCM;
   wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
   wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

   UINT_PTR devId = WAVE_MAPPER;

   WAVEOUTCAPS woc;
   unsigned long iNumDevs, ii;
   iNumDevs = waveOutGetNumDevs();
   for (ii = 0; ii < iNumDevs; ii++) {
      if (waveOutGetDevCaps(ii, &woc, sizeof(WAVEOUTCAPS))
            == MMSYSERR_NOERROR) {
         QString tmp;
         tmp = QString((const QChar *)woc.szPname);
         if (tmp.compare(QLatin1String(m_device)) == 0) {
            devId = ii;
            break;
         }
      }
   }

   if ( settings.channels() <= 2) {
      if (waveOutOpen(&hWaveOut, devId, &wfx,
                      (DWORD_PTR)&waveOutProc,
                      (DWORD_PTR) this,
                      CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
         errorState = QAudio::OpenError;
         deviceState = QAudio::StoppedState;
         emit stateChanged(deviceState);
         qWarning("QAudioOutput: open error");
         return false;
      }
   } else {
      WAVEFORMATEXTENSIBLE wfex;
      wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
      wfex.Format.nChannels = settings.channels();
      wfex.Format.wBitsPerSample = settings.sampleSize();
      wfex.Format.nSamplesPerSec = settings.frequency();
      wfex.Format.nBlockAlign = wfex.Format.nChannels * wfex.Format.wBitsPerSample / 8;
      wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;
      wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
      static const GUID _KSDATAFORMAT_SUBTYPE_PCM = {
         0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
      };
      wfex.SubFormat = _KSDATAFORMAT_SUBTYPE_PCM;
      wfex.Format.cbSize = 22;

      wfex.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
      if (settings.channels() >= 4) {
         wfex.dwChannelMask |= SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
      }
      if (settings.channels() >= 6) {
         wfex.dwChannelMask |= SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY;
      }
      if (settings.channels() == 8) {
         wfex.dwChannelMask |= SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
      }

      if (waveOutOpen(&hWaveOut, devId, &wfex.Format,
                      (DWORD_PTR)&waveOutProc,
                      (DWORD_PTR) this,
                      CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
         errorState = QAudio::OpenError;
         deviceState = QAudio::StoppedState;
         emit stateChanged(deviceState);
         qWarning("QAudioOutput: open error");
         return false;
      }
   }

   totalTimeValue = 0;
   timeStampOpened.restart();
   elapsedTimeOffset = 0;

   errorState = QAudio::NoError;
   if (pullMode) {
      deviceState = QAudio::ActiveState;
      QTimer::singleShot(10, this, SLOT(feedback()));
   } else {
      deviceState = QAudio::IdleState;
   }

   return true;
}

void QAudioOutputPrivate::close()
{
   if (deviceState == QAudio::StoppedState) {
      return;
   }

   deviceState = QAudio::StoppedState;
   errorState = QAudio::NoError;
   int delay = (buffer_size - bytesFree()) * 1000 / (settings.frequency()
               * settings.channels() * (settings.sampleSize() / 8));
   waveOutReset(hWaveOut);
   Sleep(delay + 10);

   freeBlocks(waveBlocks);
   waveOutClose(hWaveOut);
   delete [] audioBuffer;
   audioBuffer = 0;
   buffer_size = 0;
}

int QAudioOutputPrivate::bytesFree() const
{
   int buf;
   buf = waveFreeBlockCount * period_size;

   return buf;
}

int QAudioOutputPrivate::periodSize() const
{
   return period_size;
}

void QAudioOutputPrivate::setBufferSize(int value)
{
   if (deviceState == QAudio::StoppedState) {
      buffer_size = value;
   }
}

int QAudioOutputPrivate::bufferSize() const
{
   return buffer_size;
}

void QAudioOutputPrivate::setNotifyInterval(int ms)
{
   intervalTime = qMax(0, ms);
}

int QAudioOutputPrivate::notifyInterval() const
{
   return intervalTime;
}

qint64 QAudioOutputPrivate::processedUSecs() const
{
   if (deviceState == QAudio::StoppedState) {
      return 0;
   }
   qint64 result = qint64(1000000) * totalTimeValue /
                   (settings.channels() * (settings.sampleSize() / 8)) /
                   settings.frequency();

   return result;
}

qint64 QAudioOutputPrivate::write( const char *data, qint64 len )
{
   // Write out some audio data
   if (deviceState != QAudio::ActiveState && deviceState != QAudio::IdleState) {
      return 0;
   }

   char *p = (char *)data;
   int l = (int)len;

   QByteArray reverse;
   if (settings.byteOrder() == QAudioFormat::BigEndian) {

      switch (settings.sampleSize()) {
         case 8:
            // No need to convert
            break;

         case 16:
            reverse.resize(l);
            for (qint64 i = 0; i < (l >> 1); i++) {
               *((qint16 *)reverse.data() + i) = qFromBigEndian(*((qint16 *)data + i));
            }
            p = reverse.data();
            break;

         case 32:
            reverse.resize(l);
            for (qint64 i = 0; i < (l >> 2); i++) {
               *((qint32 *)reverse.data() + i) = qFromBigEndian(*((qint32 *)data + i));
            }
            p = reverse.data();
            break;
      }
   }

   WAVEHDR *current;
   int remain;
   current = &waveBlocks[waveCurrentBlock];
   while (l > 0) {
      mutex.lock();
      if (waveFreeBlockCount == 0) {
         mutex.unlock();
         break;
      }
      mutex.unlock();

      if (current->dwFlags & WHDR_PREPARED) {
         waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
      }

      if (l < period_size) {
         remain = l;
      } else {
         remain = period_size;
      }
      memcpy(current->lpData, p, remain);

      l -= remain;
      p += remain;
      current->dwBufferLength = remain;
      waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
      waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

      mutex.lock();
      waveFreeBlockCount--;
#ifdef DEBUG_AUDIO
      qDebug("write out l=%d, waveFreeBlockCount=%d",
             current->dwBufferLength, waveFreeBlockCount);
#endif
      mutex.unlock();
      totalTimeValue += current->dwBufferLength;
      waveCurrentBlock++;
      waveCurrentBlock %= buffer_size / period_size;
      current = &waveBlocks[waveCurrentBlock];
      current->dwUser = 0;
      errorState = QAudio::NoError;
      if (deviceState != QAudio::ActiveState) {
         deviceState = QAudio::ActiveState;
         emit stateChanged(deviceState);
      }
   }
   return (len - l);
}

void QAudioOutputPrivate::resume()
{
   if (deviceState == QAudio::SuspendedState) {
      deviceState = QAudio::ActiveState;
      errorState = QAudio::NoError;
      waveOutRestart(hWaveOut);
      QTimer::singleShot(10, this, SLOT(feedback()));
      emit stateChanged(deviceState);
   }
}

void QAudioOutputPrivate::suspend()
{
   if (deviceState == QAudio::ActiveState || deviceState == QAudio::IdleState) {
      int delay = (buffer_size - bytesFree()) * 1000 / (settings.frequency()
                  * settings.channels() * (settings.sampleSize() / 8));
      waveOutPause(hWaveOut);
      Sleep(delay + 10);
      deviceState = QAudio::SuspendedState;
      errorState = QAudio::NoError;
      emit stateChanged(deviceState);
   }
}

void QAudioOutputPrivate::feedback()
{
#ifdef DEBUG_AUDIO
   QTime now(QTime::currentTime());
   qDebug() << now.second() << "s " << now.msec() << "ms :feedback()";
#endif
   bytesAvailable = bytesFree();

   if (!(deviceState == QAudio::StoppedState || deviceState == QAudio::SuspendedState)) {
      if (bytesAvailable >= period_size) {
         QMetaObject::invokeMethod(this, "deviceReady", Qt::QueuedConnection);
      }
   }
}

bool QAudioOutputPrivate::deviceReady()
{
   if (deviceState == QAudio::StoppedState || deviceState == QAudio::SuspendedState) {
      return false;
   }

   if (pullMode) {
      int chunks = bytesAvailable / period_size;
#ifdef DEBUG_AUDIO
      qDebug() << "deviceReady() avail=" << bytesAvailable << " bytes, period size=" << period_size << " bytes";
      qDebug() << "deviceReady() no. of chunks that can fit =" << chunks << ", chunks in bytes =" << chunks *period_size;
#endif
      bool startup = false;
      if (totalTimeValue == 0) {
         startup = true;
      }

      bool full = false;

      mutex.lock();
      if (waveFreeBlockCount == 0) {
         full = true;
      }
      mutex.unlock();

      if (full) {
#ifdef DEBUG_AUDIO
         qDebug() << "Skipping data as unable to write";
#endif
         if (intervalTime && (timeStamp.elapsed() + elapsedTimeOffset) > intervalTime ) {
            emit notify();
            elapsedTimeOffset = timeStamp.elapsed() + elapsedTimeOffset - intervalTime;
            timeStamp.restart();
         }
         return true;
      }

      if (startup) {
         waveOutPause(hWaveOut);
      }
      int input = period_size * chunks;
      int l = audioSource->read(audioBuffer, input);
      if (l > 0) {
         int out = write(audioBuffer, l);
         if (out > 0) {
            if (deviceState != QAudio::ActiveState) {
               deviceState = QAudio::ActiveState;
               emit stateChanged(deviceState);
            }
         }
         if ( out < l) {
            // Didn't write all data
            audioSource->seek(audioSource->pos() - (l - out));
         }
         if (startup) {
            waveOutRestart(hWaveOut);
         }
      } else if (l == 0) {
         bytesAvailable = bytesFree();

         int check = 0;

         mutex.lock();
         check = waveFreeBlockCount;
         mutex.unlock();

         if (check == buffer_size / period_size) {
            if (deviceState != QAudio::IdleState) {
               errorState = QAudio::UnderrunError;
               deviceState = QAudio::IdleState;
               emit stateChanged(deviceState);
            }
         }

      } else if (l < 0) {
         bytesAvailable = bytesFree();
         errorState = QAudio::IOError;
      }
   } else {
      int buffered;

      mutex.lock();
      buffered = waveFreeBlockCount;
      mutex.unlock();

      if (buffered >= buffer_size / period_size && deviceState == QAudio::ActiveState) {
         if (deviceState != QAudio::IdleState) {
            errorState = QAudio::UnderrunError;
            deviceState = QAudio::IdleState;
            emit stateChanged(deviceState);
         }
      }
   }
   if (deviceState != QAudio::ActiveState && deviceState != QAudio::IdleState) {
      return true;
   }

   if (intervalTime && (timeStamp.elapsed() + elapsedTimeOffset) > intervalTime) {
      emit notify();
      elapsedTimeOffset = timeStamp.elapsed() + elapsedTimeOffset - intervalTime;
      timeStamp.restart();
   }

   return true;
}

qint64 QAudioOutputPrivate::elapsedUSecs() const
{
   if (deviceState == QAudio::StoppedState) {
      return 0;
   }

   return timeStampOpened.elapsed() * 1000;
}

QAudio::Error QAudioOutputPrivate::error() const
{
   return errorState;
}

QAudio::State QAudioOutputPrivate::state() const
{
   return deviceState;
}

void QAudioOutputPrivate::reset()
{
   close();
}

OutputPrivate::OutputPrivate(QAudioOutputPrivate *audio)
{
   audioDevice = qobject_cast<QAudioOutputPrivate *>(audio);
}

OutputPrivate::~OutputPrivate() {}

qint64 OutputPrivate::readData( char *data, qint64 len)
{
   Q_UNUSED(data)
   Q_UNUSED(len)

   return 0;
}

qint64 OutputPrivate::writeData(const char *data, qint64 len)
{
   int retry = 0;
   qint64 written = 0;

   if ((audioDevice->deviceState == QAudio::ActiveState)
         || (audioDevice->deviceState == QAudio::IdleState)) {
      qint64 l = len;
      while (written < l) {
         int chunk = audioDevice->write(data + written, (l - written));
         if (chunk <= 0) {
            retry++;
         } else {
            written += chunk;
         }

         if (retry > 10) {
            return written;
         }
      }
      audioDevice->deviceState = QAudio::ActiveState;
   }
   return written;
}

QT_END_NAMESPACE
