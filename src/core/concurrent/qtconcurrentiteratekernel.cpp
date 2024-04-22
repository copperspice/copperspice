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

#include <qtconcurrentiteratekernel.h>

#include <qglobal.h>

#if defined(Q_OS_DARWIN)
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

#elif defined(Q_OS_UNIX)
#include <time.h>
#include <unistd.h>

#elif defined(Q_OS_WIN)
#include <qt_windows.h>

#endif

static constexpr const int TargetRatio = 100;
static constexpr const int MedianSize  = 7;

#ifdef Q_OS_DARWIN

static qint64 getticks()
{
   return mach_absolute_time();
}

#elif defined(Q_OS_UNIX)

static qint64 getticks()
{
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
   clockid_t clockId;

#ifndef _POSIX_THREAD_CPUTIME
   clockId = CLOCK_REALTIME;

#elif (_POSIX_THREAD_CPUTIME-0 <= 0)
   // if we do not have CLOCK_THREAD_CPUTIME_ID, we have to just use elapsed realtime instead
   clockId = CLOCK_REALTIME;

#  if (_POSIX_THREAD_CPUTIME-0 == 0)
   // detect availablility of CLOCK_THREAD_CPUTIME_ID
   static long useThreadCpuTime = -2;

   if (useThreadCpuTime == -2) {
      useThreadCpuTime = sysconf(_SC_THREAD_CPUTIME);
   }

   if (useThreadCpuTime != -1) {
      clockId = CLOCK_THREAD_CPUTIME_ID;
   }

#  endif

#else
   clockId = CLOCK_THREAD_CPUTIME_ID;
#endif

   struct timespec ts;

   if (clock_gettime(clockId, &ts) == -1) {
      return 0;
   }

   return (ts.tv_sec * 1000000000) + ts.tv_nsec;
#else

   // no clock_gettime(), fall back to wall time
   struct timeval tv;
   gettimeofday(&tv, 0);
   return (tv.tv_sec * 1000000) + tv.tv_usec;

#endif
}

#elif defined(Q_OS_WIN)

static qint64 getticks()
{
   LARGE_INTEGER x;

   if (!QueryPerformanceCounter(&x)) {
      return 0;
   }

   return x.QuadPart;
}

#endif

static double elapsed(qint64 after, qint64 before)
{
   return double(after - before);
}

namespace QtConcurrent {

BlockSizeManager::BlockSizeManager(int iterationCount)
   : maxBlockSize(iterationCount / (QThreadPool::globalInstance()->maxThreadCount() * 2)),
     beforeUser(0), afterUser(0), controlPartElapsed(MedianSize),
     userPartElapsed(MedianSize), m_blockSize(1)
{ }

// Records the time before user code.
void BlockSizeManager::timeBeforeUser()
{
   if (blockSizeMaxed()) {
      return;
   }

   beforeUser = getticks();
   controlPartElapsed.addValue(elapsed(beforeUser, afterUser));
}

// Records the time after user code and adjust the block size if we are spending
// to much time in the for control code compared with the user code.
void BlockSizeManager::timeAfterUser()
{
   if (blockSizeMaxed()) {
      return;
   }

   afterUser = getticks();
   userPartElapsed.addValue(elapsed(afterUser, beforeUser));

   if (controlPartElapsed.isMedianValid() == false) {
      return;
   }

   if (controlPartElapsed.median() * TargetRatio < userPartElapsed.median()) {
      return;
   }

   m_blockSize = qMin(m_blockSize * 2,  maxBlockSize);

#if defined(CS_SHOW_DEBUG_CORE)
   qDebug() << QThread::currentThread() << "adjusting block size" << controlPartElapsed.median() <<
            userPartElapsed.median() << m_blockSize;
#endif

   // Reset the medians after adjusting the block size so we get
   // new measurements with the new block size.
   controlPartElapsed.reset();
   userPartElapsed.reset();
}

int BlockSizeManager::blockSize()
{
   return m_blockSize;
}

} // namespace QtConcurrent

