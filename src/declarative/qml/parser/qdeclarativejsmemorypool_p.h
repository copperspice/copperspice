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

#ifndef QDECLARATIVEJSMEMORYPOOL_P_H
#define QDECLARATIVEJSMEMORYPOOL_P_H

#include <qdeclarativejsglobal_p.h>
#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>
#include <string.h>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

class QML_PARSER_EXPORT MemoryPool : public QSharedData
{
 public:
   enum { maxBlockCount = -1 };
   enum { defaultBlockSize = 1 << 12 };

   MemoryPool() {
      m_blockIndex = maxBlockCount;
      m_currentIndex = 0;
      m_storage = 0;
      m_currentBlock = 0;
      m_currentBlockSize = 0;
   }

   virtual ~MemoryPool() {
      for (int index = 0; index < m_blockIndex + 1; ++index) {
         qFree(m_storage[index]);
      }

      qFree(m_storage);
   }

   char *allocate(int bytes) {
      bytes += (8 - bytes) & 7; // ensure multiple of 8 bytes (maintain alignment)
      if (m_currentBlock == 0 || m_currentBlockSize < m_currentIndex + bytes) {
         ++m_blockIndex;
         m_currentBlockSize = defaultBlockSize << m_blockIndex;

         m_storage = reinterpret_cast<char **>(qRealloc(m_storage, sizeof(char *) * (1 + m_blockIndex)));
         m_currentBlock = m_storage[m_blockIndex] = reinterpret_cast<char *>(qMalloc(m_currentBlockSize));
         ::memset(m_currentBlock, 0, m_currentBlockSize);

         m_currentIndex = (8 - quintptr(m_currentBlock)) & 7; // ensure first chunk is 64-bit aligned
         Q_ASSERT(m_currentIndex + bytes <= m_currentBlockSize);
      }

      char *p = reinterpret_cast<char *>
                (m_currentBlock + m_currentIndex);

      m_currentIndex += bytes;

      return p;
   }

   int bytesAllocated() const {
      int bytes = 0;
      for (int index = 0; index < m_blockIndex; ++index) {
         bytes += (defaultBlockSize << index);
      }
      bytes += m_currentIndex;
      return bytes;
   }

 private:
   int m_blockIndex;
   int m_currentIndex;
   char *m_currentBlock;
   int m_currentBlockSize;
   char **m_storage;

 private:
   Q_DISABLE_COPY(MemoryPool)
};

} // namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif
