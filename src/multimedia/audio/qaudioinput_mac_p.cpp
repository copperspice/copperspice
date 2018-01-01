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

#include <QtCore/qendian.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include <QtMultimedia/qaudioinput.h>

#include <qaudio_mac_p.h>
#include <qaudioinput_mac_p.h>
#include <qaudiodeviceinfo_mac_p.h>

QT_BEGIN_NAMESPACE

namespace QtMultimediaInternal {

static const int default_buffer_size = 4 * 1024;

class QAudioBufferList
{
 public:
   QAudioBufferList(AudioStreamBasicDescription const &streamFormat):
      owner(false),
      sf(streamFormat) {
      const bool isInterleaved = (sf.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
      const int numberOfBuffers = isInterleaved ? 1 : sf.mChannelsPerFrame;

      dataSize = 0;

      bfs = reinterpret_cast<AudioBufferList *>(qMalloc(sizeof(AudioBufferList) +
            (sizeof(AudioBuffer) * numberOfBuffers)));

      bfs->mNumberBuffers = numberOfBuffers;
      for (int i = 0; i < numberOfBuffers; ++i) {
         bfs->mBuffers[i].mNumberChannels = isInterleaved ? numberOfBuffers : 1;
         bfs->mBuffers[i].mDataByteSize = 0;
         bfs->mBuffers[i].mData = 0;
      }
   }

   QAudioBufferList(AudioStreamBasicDescription const &streamFormat, char *buffer, int bufferSize):
      owner(false),
      sf(streamFormat),
      bfs(0) {
      dataSize = bufferSize;

      bfs = reinterpret_cast<AudioBufferList *>(qMalloc(sizeof(AudioBufferList) + sizeof(AudioBuffer)));

      bfs->mNumberBuffers = 1;
      bfs->mBuffers[0].mNumberChannels = 1;
      bfs->mBuffers[0].mDataByteSize = dataSize;
      bfs->mBuffers[0].mData = buffer;
   }

   QAudioBufferList(AudioStreamBasicDescription const &streamFormat, int framesToBuffer):
      owner(true),
      sf(streamFormat),
      bfs(0) {
      const bool isInterleaved = (sf.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
      const int numberOfBuffers = isInterleaved ? 1 : sf.mChannelsPerFrame;

      dataSize = framesToBuffer * sf.mBytesPerFrame;

      bfs = reinterpret_cast<AudioBufferList *>(qMalloc(sizeof(AudioBufferList) +
            (sizeof(AudioBuffer) * numberOfBuffers)));
      bfs->mNumberBuffers = numberOfBuffers;
      for (int i = 0; i < numberOfBuffers; ++i) {
         bfs->mBuffers[i].mNumberChannels = isInterleaved ? numberOfBuffers : 1;
         bfs->mBuffers[i].mDataByteSize = dataSize;
         bfs->mBuffers[i].mData = qMalloc(dataSize);
      }
   }

   ~QAudioBufferList() {
      if (owner) {
         for (UInt32 i = 0; i < bfs->mNumberBuffers; ++i) {
            qFree(bfs->mBuffers[i].mData);
         }
      }

      qFree(bfs);
   }

   AudioBufferList *audioBufferList() const {
      return bfs;
   }

   char *data(int buffer = 0) const {
      return static_cast<char *>(bfs->mBuffers[buffer].mData);
   }

   qint64 bufferSize(int buffer = 0) const {
      return bfs->mBuffers[buffer].mDataByteSize;
   }

   int frameCount(int buffer = 0) const {
      return bfs->mBuffers[buffer].mDataByteSize / sf.mBytesPerFrame;
   }

   int packetCount(int buffer = 0) const {
      return bfs->mBuffers[buffer].mDataByteSize / sf.mBytesPerPacket;
   }

   int packetSize() const {
      return sf.mBytesPerPacket;
   }

   void reset() {
      for (UInt32 i = 0; i < bfs->mNumberBuffers; ++i) {
         bfs->mBuffers[i].mDataByteSize = dataSize;
         bfs->mBuffers[i].mData = 0;
      }
   }

 private:
   bool    owner;
   int     dataSize;
   AudioStreamBasicDescription sf;
   AudioBufferList *bfs;
};

class QAudioPacketFeeder
{
 public:
   QAudioPacketFeeder(QAudioBufferList *abl):
      audioBufferList(abl) {
      totalPackets = audioBufferList->packetCount();
      position = 0;
   }

   bool feed(AudioBufferList &dst, UInt32 &packetCount) {
      if (position == totalPackets) {
         dst.mBuffers[0].mDataByteSize = 0;
         packetCount = 0;
         return false;
      }

      if (totalPackets - position < packetCount) {
         packetCount = totalPackets - position;
      }

      dst.mBuffers[0].mDataByteSize = packetCount * audioBufferList->packetSize();
      dst.mBuffers[0].mData = audioBufferList->data() + (position * audioBufferList->packetSize());

      position += packetCount;

      return true;
   }

   bool empty() const {
      return position == totalPackets;
   }

 private:
   UInt32 totalPackets;
   UInt32 position;
   QAudioBufferList   *audioBufferList;
};

class QAudioInputBuffer : public QObject
{
   MULTI_CS_OBJECT(QAudioInputBuffer)

 public:
   QAudioInputBuffer(int bufferSize,
                     int maxPeriodSize,
                     AudioStreamBasicDescription const &inputFormat,
                     AudioStreamBasicDescription const &outputFormat,
                     QObject *parent):
      QObject(parent),
      m_deviceError(false),
      m_audioConverter(0),
      m_inputFormat(inputFormat),
      m_outputFormat(outputFormat) {
      m_maxPeriodSize = maxPeriodSize;
      m_periodTime = m_maxPeriodSize / m_outputFormat.mBytesPerFrame * 1000 / m_outputFormat.mSampleRate;
      m_buffer = new QAudioRingBuffer(bufferSize + (bufferSize % maxPeriodSize == 0 ? 0 : maxPeriodSize -
                                      (bufferSize % maxPeriodSize)));
      m_inputBufferList = new QAudioBufferList(m_inputFormat);

      m_flushTimer = new QTimer(this);
      connect(m_flushTimer, SIGNAL(timeout()), this, SLOT(flushBuffer()));

      if (toQAudioFormat(inputFormat) != toQAudioFormat(outputFormat)) {
         if (AudioConverterNew(&m_inputFormat, &m_outputFormat, &m_audioConverter) != noErr) {
            qWarning() << "QAudioInput: Unable to create an Audio Converter";
            m_audioConverter = 0;
         }
      }
   }

   ~QAudioInputBuffer() {
      delete m_buffer;
   }

   qint64 renderFromDevice(AudioUnit audioUnit,
                           AudioUnitRenderActionFlags *ioActionFlags,
                           const AudioTimeStamp *inTimeStamp,
                           UInt32 inBusNumber,
                           UInt32 inNumberFrames) {
      const bool  pullMode = m_device == 0;

      OSStatus    err;
      qint64      framesRendered = 0;

      m_inputBufferList->reset();
      err = AudioUnitRender(audioUnit,
                            ioActionFlags,
                            inTimeStamp,
                            inBusNumber,
                            inNumberFrames,
                            m_inputBufferList->audioBufferList());

      if (m_audioConverter != 0) {
         QAudioPacketFeeder  feeder(m_inputBufferList);

         int     copied = 0;
         const int available = m_buffer->free();

         while (err == noErr && !feeder.empty()) {
            QAudioRingBuffer::Region region = m_buffer->acquireWriteRegion(available);

            if (region.second == 0) {
               break;
            }

            AudioBufferList     output;
            output.mNumberBuffers = 1;
            output.mBuffers[0].mNumberChannels = 1;
            output.mBuffers[0].mDataByteSize = region.second;
            output.mBuffers[0].mData = region.first;

            UInt32  packetSize = region.second / m_outputFormat.mBytesPerPacket;
            err = AudioConverterFillComplexBuffer(m_audioConverter,
                                                  converterCallback,
                                                  &feeder,
                                                  &packetSize,
                                                  &output,
                                                  0);
            region.second = output.mBuffers[0].mDataByteSize;
            copied += region.second;

            m_buffer->releaseWriteRegion(region);
         }

         framesRendered += copied / m_outputFormat.mBytesPerFrame;
      } else {
         const int available = m_inputBufferList->bufferSize();
         bool    wecan = true;
         int     copied = 0;

         while (wecan && copied < available) {
            QAudioRingBuffer::Region region = m_buffer->acquireWriteRegion(available - copied);

            if (region.second > 0) {
               memcpy(region.first, m_inputBufferList->data() + copied, region.second);
               copied += region.second;
            } else {
               wecan = false;
            }

            m_buffer->releaseWriteRegion(region);
         }

         framesRendered = copied / m_outputFormat.mBytesPerFrame;
      }

      if (pullMode && framesRendered > 0) {
         emit readyRead();
      }

      return framesRendered;
   }

   qint64 readBytes(char *data, qint64 len) {
      bool    wecan = true;
      qint64  bytesCopied = 0;

      len -= len % m_maxPeriodSize;
      while (wecan && bytesCopied < len) {
         QAudioRingBuffer::Region region = m_buffer->acquireReadRegion(len - bytesCopied);

         if (region.second > 0) {
            memcpy(data + bytesCopied, region.first, region.second);
            bytesCopied += region.second;
         } else {
            wecan = false;
         }

         m_buffer->releaseReadRegion(region);
      }

      return bytesCopied;
   }

   void setFlushDevice(QIODevice *device) {
      if (m_device != device) {
         m_device = device;
      }
   }

   void startFlushTimer() {
      if (m_device != 0) {
         m_flushTimer->start((m_buffer->size() - (m_maxPeriodSize * 2)) / m_maxPeriodSize * m_periodTime);
      }
   }

   void stopFlushTimer() {
      m_flushTimer->stop();
   }

   void flush(bool all = false) {
      if (m_device == 0) {
         return;
      }

      const int used = m_buffer->used();
      const int readSize = all ? used : used - (used % m_maxPeriodSize);

      if (readSize > 0) {
         bool    wecan = true;
         int     flushed = 0;

         while (!m_deviceError && wecan && flushed < readSize) {
            QAudioRingBuffer::Region region = m_buffer->acquireReadRegion(readSize - flushed);

            if (region.second > 0) {
               int bytesWritten = m_device->write(region.first, region.second);
               if (bytesWritten < 0) {
                  stopFlushTimer();
                  m_deviceError = true;
               } else {
                  region.second = bytesWritten;
                  flushed += bytesWritten;
                  wecan = bytesWritten != 0;
               }
            } else {
               wecan = false;
            }

            m_buffer->releaseReadRegion(region);
         }
      }
   }

   void reset() {
      m_buffer->reset();
      m_deviceError = false;
   }

   int available() const {
      return m_buffer->free();
   }

   int used() const {
      return m_buffer->used();
   }

   MULTI_CS_SIGNAL_1(Public, void readyRead())
   MULTI_CS_SIGNAL_2(readyRead)

 private:
   MULTI_CS_SLOT_1(Private, void flushBuffer() {
      flush();
       })
   MULTI_CS_SLOT_2(flushBuffer)

   bool        m_deviceError;
   int         m_maxPeriodSize;
   int         m_periodTime;
   QIODevice  *m_device;
   QTimer     *m_flushTimer;
   QAudioRingBuffer   *m_buffer;
   QAudioBufferList   *m_inputBufferList;
   AudioConverterRef   m_audioConverter;
   AudioStreamBasicDescription m_inputFormat;
   AudioStreamBasicDescription m_outputFormat;

   const static OSStatus as_empty = 'qtem';

   // Converter callback
   static OSStatus converterCallback(AudioConverterRef inAudioConverter,
                                     UInt32 *ioNumberDataPackets,
                                     AudioBufferList *ioData,
                                     AudioStreamPacketDescription **outDataPacketDescription,
                                     void *inUserData) {
      Q_UNUSED(inAudioConverter);
      Q_UNUSED(outDataPacketDescription);

      QAudioPacketFeeder *feeder = static_cast<QAudioPacketFeeder *>(inUserData);

      if (!feeder->feed(*ioData, *ioNumberDataPackets)) {
         return as_empty;
      }

      return noErr;
   }
};


class MacInputDevice : public QIODevice
{
   MULTI_CS_OBJECT(MacInputDevice)

 public:
   MacInputDevice(QAudioInputBuffer *audioBuffer, QObject *parent):
      QIODevice(parent),
      m_audioBuffer(audioBuffer) {
      open(QIODevice::ReadOnly | QIODevice::Unbuffered);
      connect(m_audioBuffer, SIGNAL(readyRead()), this, SLOT(readyRead()));
   }

   qint64 readData(char *data, qint64 len) override {
      return m_audioBuffer->readBytes(data, len);
   }

   qint64 writeData(const char *data, qint64 len) override {
      Q_UNUSED(data);
      Q_UNUSED(len);

      return 0;
   }

   bool isSequential() const override {
      return true;
   }

 private:
   QAudioInputBuffer   *m_audioBuffer;
};

}


QAudioInputPrivate::QAudioInputPrivate(const QByteArray &device, QAudioFormat const &format):
   audioFormat(format)
{
   QDataStream ds(device);
   quint32 did, mode;

   ds >> did >> mode;

   if (QAudio::Mode(mode) == QAudio::AudioOutput) {
      errorCode = QAudio::OpenError;
   } else {
      audioDeviceInfo = new QAudioDeviceInfoInternal(device, QAudio::AudioInput);
      isOpen = false;
      audioDeviceId = AudioDeviceID(did);
      audioUnit = 0;
      startTime = 0;
      totalFrames = 0;
      audioBuffer = 0;
      internalBufferSize = QtMultimediaInternal::default_buffer_size;
      clockFrequency = AudioGetHostClockFrequency() / 1000;
      errorCode = QAudio::NoError;
      stateCode = QAudio::StoppedState;

      intervalTimer = new QTimer(this);
      intervalTimer->setInterval(1000);
      connect(intervalTimer, SIGNAL(timeout()), this, SLOT(notify()));
   }
}

QAudioInputPrivate::~QAudioInputPrivate()
{
   close();
   delete audioDeviceInfo;
}

bool QAudioInputPrivate::open()
{
   UInt32  size = 0;

   if (isOpen) {
      return true;
   }

   ComponentDescription    cd;
   cd.componentType = kAudioUnitType_Output;
   cd.componentSubType = kAudioUnitSubType_HALOutput;
   cd.componentManufacturer = kAudioUnitManufacturer_Apple;
   cd.componentFlags = 0;
   cd.componentFlagsMask = 0;

   // Open
   Component cp = FindNextComponent(NULL, &cd);
   if (cp == 0) {
      qWarning() << "QAudioInput: Failed to find HAL Output component";
      return false;
   }

   if (OpenAComponent(cp, &audioUnit) != noErr) {
      qWarning() << "QAudioInput: Unable to Open Output Component";
      return false;
   }

   // Set mode
   // switch to input mode
   UInt32 enable = 1;
   if (AudioUnitSetProperty(audioUnit,
                            kAudioOutputUnitProperty_EnableIO,
                            kAudioUnitScope_Input,
                            1,
                            &enable,
                            sizeof(enable)) != noErr) {
      qWarning() << "QAudioInput: Unable to switch to input mode (Enable Input)";
      return false;
   }

   enable = 0;
   if (AudioUnitSetProperty(audioUnit,
                            kAudioOutputUnitProperty_EnableIO,
                            kAudioUnitScope_Output,
                            0,
                            &enable,
                            sizeof(enable)) != noErr) {
      qWarning() << "QAudioInput: Unable to switch to input mode (Disable output)";
      return false;
   }

   // register callback
   AURenderCallbackStruct cb;
   cb.inputProc = inputCallback;
   cb.inputProcRefCon = this;

   if (AudioUnitSetProperty(audioUnit,
                            kAudioOutputUnitProperty_SetInputCallback,
                            kAudioUnitScope_Global,
                            0,
                            &cb,
                            sizeof(cb)) != noErr) {
      qWarning() << "QAudioInput: Failed to set AudioUnit callback";
      return false;
   }

   // Set Audio Device
   if (AudioUnitSetProperty(audioUnit,
                            kAudioOutputUnitProperty_CurrentDevice,
                            kAudioUnitScope_Global,
                            0,
                            &audioDeviceId,
                            sizeof(audioDeviceId)) != noErr) {
      qWarning() << "QAudioInput: Unable to use configured device";
      return false;
   }

   // Set format
   // Wanted
   streamFormat = toAudioStreamBasicDescription(audioFormat);

   // Required on unit
   if (audioFormat == audioDeviceInfo->preferredFormat()) {
      deviceFormat = streamFormat;
      AudioUnitSetProperty(audioUnit,
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           1,
                           &deviceFormat,
                           sizeof(deviceFormat));
   } else {
      size = sizeof(deviceFormat);
      if (AudioUnitGetProperty(audioUnit,
                               kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Input,
                               1,
                               &deviceFormat,
                               &size) != noErr) {
         qWarning() << "QAudioInput: Unable to retrieve device format";
         return false;
      }

      if (AudioUnitSetProperty(audioUnit,
                               kAudioUnitProperty_StreamFormat,
                               kAudioUnitScope_Output,
                               1,
                               &deviceFormat,
                               sizeof(deviceFormat)) != noErr) {
         qWarning() << "QAudioInput: Unable to set device format";
         return false;
      }
   }

   // Setup buffers
   UInt32 numberOfFrames;
   size = sizeof(UInt32);
   if (AudioUnitGetProperty(audioUnit,
                            kAudioDevicePropertyBufferFrameSize,
                            kAudioUnitScope_Global,
                            0,
                            &numberOfFrames,
                            &size) != noErr) {
      qWarning() << "QAudioInput: Failed to get audio period size";
      return false;
   }

   // Allocate buffer
   periodSizeBytes = numberOfFrames * streamFormat.mBytesPerFrame;

   if (internalBufferSize < periodSizeBytes * 2) {
      internalBufferSize = periodSizeBytes * 2;
   } else {
      internalBufferSize -= internalBufferSize % streamFormat.mBytesPerFrame;
   }

   audioBuffer = new QtMultimediaInternal::QAudioInputBuffer(internalBufferSize,
         periodSizeBytes,
         deviceFormat,
         streamFormat,
         this);

   audioIO = new QtMultimediaInternal::MacInputDevice(audioBuffer, this);

   // Init
   if (AudioUnitInitialize(audioUnit) != noErr) {
      qWarning() << "QAudioInput: Failed to initialize AudioUnit";
      return false;
   }

   isOpen = true;

   return isOpen;
}

void QAudioInputPrivate::close()
{
   if (audioUnit != 0) {
      AudioOutputUnitStop(audioUnit);
      AudioUnitUninitialize(audioUnit);
      CloseComponent(audioUnit);
   }

   delete audioBuffer;
}

QAudioFormat QAudioInputPrivate::format() const
{
   return audioFormat;
}

QIODevice *QAudioInputPrivate::start(QIODevice *device)
{
   QIODevice  *op = device;

   if (!audioDeviceInfo->isFormatSupported(audioFormat) || !open()) {
      stateCode = QAudio::StoppedState;
      errorCode = QAudio::OpenError;
      return audioIO;
   }

   reset();
   audioBuffer->reset();
   audioBuffer->setFlushDevice(op);

   if (op == 0) {
      op = audioIO;
   }

   // Start
   startTime = AudioGetCurrentHostTime();
   totalFrames = 0;

   audioThreadStart();

   stateCode = QAudio::ActiveState;
   errorCode = QAudio::NoError;
   emit stateChanged(stateCode);

   return op;
}

void QAudioInputPrivate::stop()
{
   QMutexLocker    lock(&mutex);
   if (stateCode != QAudio::StoppedState) {
      audioThreadStop();
      audioBuffer->flush(true);

      errorCode = QAudio::NoError;
      stateCode = QAudio::StoppedState;
      QMetaObject::invokeMethod(this, "stateChanged", Qt::QueuedConnection, Q_ARG(QAudio::State, stateCode));
   }
}

void QAudioInputPrivate::reset()
{
   QMutexLocker    lock(&mutex);
   if (stateCode != QAudio::StoppedState) {
      audioThreadStop();

      errorCode = QAudio::NoError;
      stateCode = QAudio::StoppedState;
      QMetaObject::invokeMethod(this, "stateChanged", Qt::QueuedConnection, Q_ARG(QAudio::State, stateCode));
   }
}

void QAudioInputPrivate::suspend()
{
   QMutexLocker    lock(&mutex);
   if (stateCode == QAudio::ActiveState || stateCode == QAudio::IdleState) {
      audioThreadStop();

      errorCode = QAudio::NoError;
      stateCode = QAudio::SuspendedState;
      QMetaObject::invokeMethod(this, "stateChanged", Qt::QueuedConnection, Q_ARG(QAudio::State, stateCode));
   }
}

void QAudioInputPrivate::resume()
{
   QMutexLocker    lock(&mutex);
   if (stateCode == QAudio::SuspendedState) {
      audioThreadStart();

      errorCode = QAudio::NoError;
      stateCode = QAudio::ActiveState;
      QMetaObject::invokeMethod(this, "stateChanged", Qt::QueuedConnection, Q_ARG(QAudio::State, stateCode));
   }
}

int QAudioInputPrivate::bytesReady() const
{
   return audioBuffer->used();
}

int QAudioInputPrivate::periodSize() const
{
   return periodSizeBytes;
}

void QAudioInputPrivate::setBufferSize(int bs)
{
   internalBufferSize = bs;
}

int QAudioInputPrivate::bufferSize() const
{
   return internalBufferSize;
}

void QAudioInputPrivate::setNotifyInterval(int milliSeconds)
{
   if (intervalTimer->interval() == milliSeconds) {
      return;
   }

   if (milliSeconds <= 0) {
      milliSeconds = 0;
   }

   intervalTimer->setInterval(milliSeconds);
}

int QAudioInputPrivate::notifyInterval() const
{
   return intervalTimer->interval();
}

qint64 QAudioInputPrivate::processedUSecs() const
{
   return totalFrames * 1000000 / audioFormat.frequency();
}

qint64 QAudioInputPrivate::elapsedUSecs() const
{
   if (stateCode == QAudio::StoppedState) {
      return 0;
   }

   return (AudioGetCurrentHostTime() - startTime) / (clockFrequency / 1000);
}

QAudio::Error QAudioInputPrivate::error() const
{
   return errorCode;
}

QAudio::State QAudioInputPrivate::state() const
{
   return stateCode;
}

void QAudioInputPrivate::audioThreadStop()
{
   stopTimers();
   if (audioThreadState.testAndSetAcquire(Running, Stopped)) {
      threadFinished.wait(&mutex);
   }
}

void QAudioInputPrivate::audioThreadStart()
{
   startTimers();
   audioThreadState = Running;
   AudioOutputUnitStart(audioUnit);
}

void QAudioInputPrivate::audioDeviceStop()
{
   AudioOutputUnitStop(audioUnit);
   audioThreadState = Stopped;
   threadFinished.wakeOne();
}

void QAudioInputPrivate::audioDeviceFull()
{
   QMutexLocker    lock(&mutex);
   if (stateCode == QAudio::ActiveState) {
      audioDeviceStop();

      errorCode = QAudio::UnderrunError;
      stateCode = QAudio::IdleState;
      QMetaObject::invokeMethod(this, "deviceStopped", Qt::QueuedConnection);
   }
}

void QAudioInputPrivate::audioDeviceError()
{
   QMutexLocker    lock(&mutex);
   if (stateCode == QAudio::ActiveState) {
      audioDeviceStop();

      errorCode = QAudio::IOError;
      stateCode = QAudio::StoppedState;
      QMetaObject::invokeMethod(this, "deviceStopped", Qt::QueuedConnection);
   }
}

void QAudioInputPrivate::startTimers()
{
   audioBuffer->startFlushTimer();
   if (intervalTimer->interval() > 0) {
      intervalTimer->start();
   }
}

void QAudioInputPrivate::stopTimers()
{
   audioBuffer->stopFlushTimer();
   intervalTimer->stop();
}

void QAudioInputPrivate::deviceStopped()
{
   stopTimers();
   emit stateChanged(stateCode);
}

// Input callback
OSStatus QAudioInputPrivate::inputCallback(void *inRefCon,
      AudioUnitRenderActionFlags *ioActionFlags,
      const AudioTimeStamp *inTimeStamp,
      UInt32 inBusNumber,
      UInt32 inNumberFrames,
      AudioBufferList *ioData)
{
   Q_UNUSED(ioData);

   QAudioInputPrivate *d = static_cast<QAudioInputPrivate *>(inRefCon);

   const int threadState = d->audioThreadState.fetchAndAddAcquire(0);
   if (threadState == Stopped) {
      d->audioDeviceStop();
   } else {
      qint64      framesWritten;

      framesWritten = d->audioBuffer->renderFromDevice(d->audioUnit,
                      ioActionFlags,
                      inTimeStamp,
                      inBusNumber,
                      inNumberFrames);

      if (framesWritten > 0) {
         d->totalFrames += framesWritten;
      } else if (framesWritten == 0) {
         d->audioDeviceFull();
      } else if (framesWritten < 0) {
         d->audioDeviceError();
      }
   }

   return noErr;
}


QT_END_NAMESPACE
