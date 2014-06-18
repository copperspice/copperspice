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

#include "qwindowspipewriter_p.h"
#include <string.h>

QT_BEGIN_NAMESPACE

QWindowsPipeWriter::QWindowsPipeWriter(HANDLE pipe, QObject *parent)
   : QThread(parent),
     writePipe(INVALID_HANDLE_VALUE),
     quitNow(false),
     hasWritten(false)
{
   Q_UNUSED(pipe);
   writePipe = GetCurrentProcess();

}

QWindowsPipeWriter::~QWindowsPipeWriter()
{
   lock.lock();
   quitNow = true;
   waitCondition.wakeOne();
   lock.unlock();

   if (!wait(30000)) {
      terminate();
   }
}

bool QWindowsPipeWriter::waitForWrite(int msecs)
{
   QMutexLocker locker(&lock);
   bool hadWritten = hasWritten;
   hasWritten = false;
   if (hadWritten) {
      return true;
   }
   if (!waitCondition.wait(&lock, msecs)) {
      return false;
   }
   hadWritten = hasWritten;
   hasWritten = false;
   return hadWritten;
}

qint64 QWindowsPipeWriter::write(const char *ptr, qint64 maxlen)
{
   if (!isRunning()) {
      return -1;
   }

   QMutexLocker locker(&lock);
   data.append(QByteArray(ptr, maxlen));
   waitCondition.wakeOne();
   return maxlen;
}

void QWindowsPipeWriter::run()
{
   OVERLAPPED overl;
   memset(&overl, 0, sizeof overl);
   overl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   forever {
      lock.lock();
      while (data.isEmpty() && (!quitNow))
      {
         waitCondition.wakeOne();
         waitCondition.wait(&lock);
      }

      if (quitNow)
      {
         lock.unlock();
         quitNow = false;
         break;
      }

      QByteArray copy = data;

      lock.unlock();

      const char *ptrData = copy.data();
      qint64 maxlen = copy.size();
      qint64 totalWritten = 0;
      overl.Offset = 0;
      overl.OffsetHigh = 0;
      while ((!quitNow) && totalWritten < maxlen)
      {
         DWORD written = 0;
         if (!WriteFile(writePipe, ptrData + totalWritten,
         maxlen - totalWritten, &written, &overl)) {

            if (GetLastError() == 0xE8/*NT_STATUS_INVALID_USER_BUFFER*/) {
               // give the os a rest
               msleep(100);
               continue;
            }

            if (GetLastError() == ERROR_IO_PENDING) {
               if (!GetOverlappedResult(writePipe, &overl, &written, TRUE)) {
                  CloseHandle(overl.hEvent);
                  return;
               }
            } else {
               CloseHandle(overl.hEvent);
               return;
            }

         }
         totalWritten += written;
#if defined QPIPEWRITER_DEBUG
         qDebug("QWindowsPipeWriter::run() wrote %d %d/%d bytes",
                written, int(totalWritten), int(maxlen));
#endif
         lock.lock();
         data.remove(0, written);
         hasWritten = true;
         lock.unlock();
      }
      emit bytesWritten(totalWritten);
      emit canWrite();
   }
   CloseHandle(overl.hEvent);
}

QT_END_NAMESPACE
