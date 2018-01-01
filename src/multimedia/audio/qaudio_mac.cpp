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

#include <qaudio_mac_p.h>

QT_BEGIN_NAMESPACE

// Debugging
QDebug operator<<(QDebug dbg, const QAudioFormat &audioFormat)
{
   dbg.nospace() << "QAudioFormat(" <<
                 audioFormat.frequency() << "," <<
                 audioFormat.channels() << "," <<
                 audioFormat.sampleSize() << "," <<
                 audioFormat.codec() << "," <<
                 audioFormat.byteOrder() << "," <<
                 audioFormat.sampleType() << ")";

   return dbg.space();
}


// Conversion
QAudioFormat toQAudioFormat(AudioStreamBasicDescription const &sf)
{
   QAudioFormat    audioFormat;

   audioFormat.setFrequency(sf.mSampleRate);
   audioFormat.setChannels(sf.mChannelsPerFrame);
   audioFormat.setSampleSize(sf.mBitsPerChannel);
   audioFormat.setCodec(QString::fromLatin1("audio/pcm"));
   audioFormat.setByteOrder((sf.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0 ? QAudioFormat::BigEndian :
                            QAudioFormat::LittleEndian);
   QAudioFormat::SampleType type = QAudioFormat::UnSignedInt;
   if ((sf.mFormatFlags & kAudioFormatFlagIsSignedInteger) != 0) {
      type = QAudioFormat::SignedInt;
   } else if ((sf.mFormatFlags & kAudioFormatFlagIsFloat) != 0) {
      type = QAudioFormat::Float;
   }
   audioFormat.setSampleType(type);

   return audioFormat;
}

AudioStreamBasicDescription toAudioStreamBasicDescription(QAudioFormat const &audioFormat)
{
   AudioStreamBasicDescription sf;

   sf.mFormatFlags         = kAudioFormatFlagIsPacked;
   sf.mSampleRate          = audioFormat.frequency();
   sf.mFramesPerPacket     = 1;
   sf.mChannelsPerFrame    = audioFormat.channels();
   sf.mBitsPerChannel      = audioFormat.sampleSize();
   sf.mBytesPerFrame       = sf.mChannelsPerFrame * (sf.mBitsPerChannel / 8);
   sf.mBytesPerPacket      = sf.mFramesPerPacket * sf.mBytesPerFrame;
   sf.mFormatID            = kAudioFormatLinearPCM;

   switch (audioFormat.sampleType()) {
      case QAudioFormat::SignedInt:
         sf.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
         break;
      case QAudioFormat::UnSignedInt: /* default */
         break;
      case QAudioFormat::Float:
         sf.mFormatFlags |= kAudioFormatFlagIsFloat;
         break;
      case QAudioFormat::Unknown:
      default:
         break;
   }

   if (audioFormat.byteOrder() == QAudioFormat::BigEndian) {
      sf.mFormatFlags |= kAudioFormatFlagIsBigEndian;
   }

   return sf;
}

// QAudioRingBuffer
QAudioRingBuffer::QAudioRingBuffer(int bufferSize):
   m_bufferSize(bufferSize)
{
   m_buffer = new char[m_bufferSize];
   reset();
}

QAudioRingBuffer::~QAudioRingBuffer()
{
   delete m_buffer;
}

int QAudioRingBuffer::used() const
{
   return m_bufferUsed.load();
}

int QAudioRingBuffer::free() const
{
    return m_bufferSize - m_bufferUsed.load();
}

int QAudioRingBuffer::size() const
{
   return m_bufferSize;
}

void QAudioRingBuffer::reset()
{
   m_readPos = 0;
   m_writePos = 0;
   m_bufferUsed.store(0);
}

QT_END_NAMESPACE


