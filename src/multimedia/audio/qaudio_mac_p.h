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

#ifndef QAUDIO_MAC_P_H
#define QAUDIO_MAC_P_H

#include <CoreAudio/CoreAudio.h>
#include <QtCore/qdebug.h>
#include <QtCore/qatomic.h>
#include <QtMultimedia/qaudioformat.h>

QT_BEGIN_NAMESPACE

extern QDebug operator<<(QDebug dbg, const QAudioFormat &audioFormat);
extern QAudioFormat toQAudioFormat(const AudioStreamBasicDescription &streamFormat);
extern AudioStreamBasicDescription toAudioStreamBasicDescription(QAudioFormat const &audioFormat);

class QAudioRingBuffer
{
 public:
   typedef QPair<char *, int> Region;

   QAudioRingBuffer(int bufferSize);
   ~QAudioRingBuffer();

   Region acquireReadRegion(int size) {
      const int used = m_bufferUsed.fetchAndAddAcquire(0);

      if (used > 0) {
         const int readSize = qMin(size, qMin(m_bufferSize - m_readPos, used));

         return readSize > 0 ? Region(m_buffer + m_readPos, readSize) : Region(0, 0);
      }

      return Region(0, 0);
   }

   void releaseReadRegion(Region const &region) {
      m_readPos = (m_readPos + region.second) % m_bufferSize;

      m_bufferUsed.fetchAndAddRelease(-region.second);
   }

   Region acquireWriteRegion(int size) {
      const int free = m_bufferSize - m_bufferUsed.fetchAndAddAcquire(0);

      if (free > 0) {
         const int writeSize = qMin(size, qMin(m_bufferSize - m_writePos, free));

         return writeSize > 0 ? Region(m_buffer + m_writePos, writeSize) : Region(0, 0);
      }

      return Region(0, 0);
   }

   void releaseWriteRegion(Region const &region) {
      m_writePos = (m_writePos + region.second) % m_bufferSize;

      m_bufferUsed.fetchAndAddRelease(region.second);
   }

   int used() const;
   int free() const;
   int size() const;

   void reset();

 private:
   int     m_bufferSize;
   int     m_readPos;
   int     m_writePos;
   char   *m_buffer;
   QAtomicInt  m_bufferUsed;
};

QT_END_NAMESPACE

#endif  // QAUDIO_MAC_P_H


