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

#include <qwindowspipereader_p.h>

#include <qelapsedtimer.h>

#include <qiodevice_p.h>

QWindowsPipeReader::Overlapped::Overlapped(QWindowsPipeReader *reader)
   : pipeReader(reader)
{
}

void QWindowsPipeReader::Overlapped::clear()
{
   ZeroMemory(this, sizeof(OVERLAPPED));
}

QWindowsPipeReader::QWindowsPipeReader(QObject *parent)
   : QObject(parent),
     handle(INVALID_HANDLE_VALUE),
     overlapped(this),
     readBufferMaxSize(0),
     actualReadBufferSize(0),
     stopped(true),
     readSequenceStarted(false),
     notifiedCalled(false),
     pipeBroken(false),
     readyReadPending(false),
     inReadyRead(false)
{
   connect(this, &QWindowsPipeReader::_q_queueReadyRead,
         this, &QWindowsPipeReader::emitPendingReadyRead, Qt::QueuedConnection);
}

QWindowsPipeReader::~QWindowsPipeReader()
{
   stop();
}

void QWindowsPipeReader::setHandle(HANDLE hPipeReadEnd)
{
   readBuffer.clear();
   actualReadBufferSize = 0;
   handle = hPipeReadEnd;
   pipeBroken = false;
}

void QWindowsPipeReader::stop()
{
   stopped = true;

   if (readSequenceStarted) {
      if (! CancelIoEx(handle, &overlapped)) {
         const DWORD dwError = GetLastError();

         if (dwError != ERROR_NOT_FOUND) {
            qErrnoWarning(dwError, "QWindowsPipeReader: CancelIoEx on handle %x failed.", handle);
         }
      }

      waitForNotification(-1);
   }
}

qint64 QWindowsPipeReader::bytesAvailable() const
{
   return actualReadBufferSize;
}

qint64 QWindowsPipeReader::read(char *data, qint64 maxlen)
{
   if (pipeBroken && actualReadBufferSize == 0) {
      return 0;   // signal EOF
   }

   qint64 readSoFar;

   // If startAsyncRead() has read data, copy it to its destination.
   if (maxlen == 1 && actualReadBufferSize > 0) {
      *data = readBuffer.getChar();
      actualReadBufferSize--;
      readSoFar = 1;

   } else {
      qint64 bytesToRead = qMin(actualReadBufferSize, maxlen);
      readSoFar = 0;

      while (readSoFar < bytesToRead) {
         const char *ptr = readBuffer.readPointer();

         qint64 bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar, readBuffer.nextDataBlockSize());
         memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
         readSoFar += bytesToReadFromThisBlock;
         readBuffer.free(bytesToReadFromThisBlock);
         actualReadBufferSize -= bytesToReadFromThisBlock;
      }
   }

   if (! pipeBroken) {
      if (!readSequenceStarted && !stopped) {
         startAsyncRead();
      }

      if (readSoFar == 0) {
         return -2;   // signal EWOULDBLOCK
      }
   }

   return readSoFar;
}

bool QWindowsPipeReader::canReadLine() const
{
   return readBuffer.indexOf('\n', actualReadBufferSize) >= 0;
}

void QWindowsPipeReader::notified(DWORD errorCode, DWORD numberOfBytesRead)
{
   notifiedCalled = true;
   readSequenceStarted = false;

   switch (errorCode) {
      case ERROR_SUCCESS:
         break;

      case ERROR_MORE_DATA:
         // This is not an error. We're connected to a message mode
         // pipe and the message didn't fit into the pipe's system
         // buffer. We will read the remaining data in the next call.
         break;

      case ERROR_BROKEN_PIPE:
      case ERROR_PIPE_NOT_CONNECTED:
         pipeBroken = true;
         break;

      case ERROR_OPERATION_ABORTED:
         if (stopped) {
            break;
         }

         [[fallthrough]];

      default:
         emit winError(errorCode, QString("QWindowsPipeReader::notified"));
         pipeBroken = true;
         break;
   }

   // After the reader was stopped, the only reason why this function can be called is the
   // completion of a cancellation. No signals should be emitted, and no new read sequence should
   // be started in this case.
   if (stopped) {
      return;
   }

   if (pipeBroken) {
      emit pipeClosed();
      return;
   }

   actualReadBufferSize += numberOfBytesRead;
   readBuffer.truncate(actualReadBufferSize);
   startAsyncRead();

   if (!readyReadPending) {
      readyReadPending = true;
      emit _q_queueReadyRead();
   }
}

void QWindowsPipeReader::startAsyncRead()
{
   const DWORD minReadBufferSize = 4096;
   DWORD bytesToRead = qMax(checkPipeState(), minReadBufferSize);

   if (pipeBroken) {
      return;
   }

   if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - readBuffer.size())) {
      bytesToRead = readBufferMaxSize - readBuffer.size();

      if (bytesToRead == 0) {
         // Buffer is full. User must read data from the buffer
         // before we can read more from the pipe.
         return;
      }
   }

   char *ptr = readBuffer.reserve(bytesToRead);

   stopped = false;
   readSequenceStarted = true;
   overlapped.clear();

   if (!ReadFileEx(handle, ptr, bytesToRead, &overlapped, &readFileCompleted)) {
      readSequenceStarted = false;

      const DWORD dwError = GetLastError();

      switch (dwError) {
         case ERROR_BROKEN_PIPE:
         case ERROR_PIPE_NOT_CONNECTED:
            // It may happen, that the other side closes the connection directly
            // after writing data. Then we must set the appropriate socket state.
            pipeBroken = true;
            emit pipeClosed();
            break;

         default:
            emit winError(dwError, QString("QWindowsPipeReader::startAsyncRead"));
            break;
      }
   }
}

void QWindowsPipeReader::readFileCompleted(DWORD errorCode, DWORD numberOfBytesTransfered,
      OVERLAPPED *overlappedBase)
{
   Overlapped *overlapped = static_cast<Overlapped *>(overlappedBase);
   overlapped->pipeReader->notified(errorCode, numberOfBytesTransfered);
}

DWORD QWindowsPipeReader::checkPipeState()
{
   DWORD bytes;

   if (PeekNamedPipe(handle, nullptr, 0, nullptr, &bytes, nullptr)) {
      return bytes;
   } else {
      if (!pipeBroken) {
         pipeBroken = true;
         emit pipeClosed();
      }
   }

   return 0;
}

bool QWindowsPipeReader::waitForNotification(int timeout)
{
   QElapsedTimer t;
   t.start();
   notifiedCalled = false;
   int msecs = timeout;

   while (SleepEx(msecs == -1 ? INFINITE : msecs, TRUE) == WAIT_IO_COMPLETION) {
      if (notifiedCalled) {
         return true;
      }

      // Some other I/O completion routine was called. Wait some more.
      msecs = qt_subtract_from_timeout(timeout, t.elapsed());

      if (!msecs) {
         break;
      }
   }

   return notifiedCalled;
}

void QWindowsPipeReader::emitPendingReadyRead()
{
   if (readyReadPending) {
      readyReadPending = false;
      inReadyRead      = true;
      emit readyRead();
      inReadyRead      = false;
   }
}

bool QWindowsPipeReader::waitForReadyRead(int msecs)
{
   if (!readSequenceStarted) {
      return false;
   }

   if (readyReadPending) {
      if (! inReadyRead) {
         emitPendingReadyRead();
      }

      return true;
   }

   if (! waitForNotification(msecs)) {
      return false;
   }

   if (readyReadPending) {
      if (!inReadyRead) {
         emitPendingReadyRead();
      }

      return true;
   }

   return false;
}

bool QWindowsPipeReader::waitForPipeClosed(int msecs)
{
   const int sleepTime = 10;
   QElapsedTimer stopWatch;
   stopWatch.start();

   while (true) {
      waitForReadyRead(0);
      checkPipeState();

      if (pipeBroken) {
         return true;
      }

      if (stopWatch.hasExpired(msecs - sleepTime)) {
         return false;
      }

      Sleep(sleepTime);
   }
}
