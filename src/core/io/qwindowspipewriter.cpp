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

#include <string.h>

#include <qwindowspipewriter_p.h>
#include <qiodevice_p.h>

QT_BEGIN_NAMESPACE

extern bool qt_cancelIo(HANDLE handle, OVERLAPPED *overlapped);     // from qwindowspipereader.cpp
QWindowsPipeWriter::Overlapped::Overlapped(QWindowsPipeWriter *pipeWriter)
   : pipeWriter(pipeWriter)
{
}
void QWindowsPipeWriter::Overlapped::clear()
{
   ZeroMemory(this, sizeof(OVERLAPPED));
}
QWindowsPipeWriter::QWindowsPipeWriter(HANDLE pipeWriteEnd, QObject *parent)
   : QObject(parent),
     handle(pipeWriteEnd),
     overlapped(this),
     numberOfBytesToWrite(0),
     pendingBytesWrittenValue(0),
     stopped(true),
     writeSequenceStarted(false),
     notifiedCalled(false),
     bytesWrittenPending(false),
     inBytesWritten(false)
{
   connect(this, &QWindowsPipeWriter::_q_queueBytesWritten,
           this, &QWindowsPipeWriter::emitPendingBytesWrittenValue, Qt::QueuedConnection);
}

QWindowsPipeWriter::~QWindowsPipeWriter()
{
   stop();
}

bool QWindowsPipeWriter::waitForWrite(int msecs)
{

   if (bytesWrittenPending) {
      emitPendingBytesWrittenValue();
      return true;
   }
   if (!writeSequenceStarted) {
      return false;
   }
   if (!waitForNotification(msecs)) {
      return false;
   }
   if (bytesWrittenPending) {
      emitPendingBytesWrittenValue();
      return true;
   }
   return false;
}

qint64 QWindowsPipeWriter::bytesToWrite() const
{
   return numberOfBytesToWrite + pendingBytesWrittenValue;
}
void QWindowsPipeWriter::emitPendingBytesWrittenValue()
{
   if (bytesWrittenPending) {
      bytesWrittenPending = false;
      const qint64 bytes = pendingBytesWrittenValue;
      pendingBytesWrittenValue = 0;
      emit canWrite();
      if (!inBytesWritten) {
         inBytesWritten = true;
         emit bytesWritten(bytes);
         inBytesWritten = false;
      }
   }
}
void QWindowsPipeWriter::writeFileCompleted(DWORD errorCode, DWORD numberOfBytesTransfered,
      OVERLAPPED *overlappedBase)
{
   Overlapped *overlapped = static_cast<Overlapped *>(overlappedBase);
   overlapped->pipeWriter->notified(errorCode, numberOfBytesTransfered);
}

void QWindowsPipeWriter::notified(DWORD errorCode, DWORD numberOfBytesWritten)
{
   notifiedCalled = true;
   writeSequenceStarted = false;
   numberOfBytesToWrite = 0;
   Q_ASSERT(errorCode != ERROR_SUCCESS || numberOfBytesWritten == DWORD(buffer.size()));
   buffer.clear();
   switch (errorCode) {
      case ERROR_SUCCESS:
         break;
      case ERROR_OPERATION_ABORTED:
         if (stopped) {
            break;
         }
      default:
         qErrnoWarning(errorCode, "QWindowsPipeWriter: asynchronous write failed.");
         break;
   }

   // After the writer was stopped, the only reason why this function can be called is the
   if (stopped) {
      return;
   }

   pendingBytesWrittenValue += qint64(numberOfBytesWritten);

   if (!bytesWrittenPending) {
      bytesWrittenPending = true;
      emit _q_queueBytesWritten();
   }
}

bool QWindowsPipeWriter::waitForNotification(int timeout)
{
   QElapsedTimer t;
   t.start();
   notifiedCalled = false;
   int msecs = timeout;
   while (SleepEx(msecs == -1 ? INFINITE : msecs, TRUE) == WAIT_IO_COMPLETION) {
      if (notifiedCalled) {
         return true;
      }
      msecs = qt_subtract_from_timeout(timeout, t.elapsed());
      if (!msecs) {
         break;
      }
   }
   return notifiedCalled;
}
bool QWindowsPipeWriter::write(const QByteArray &ba)
{
   if (writeSequenceStarted) {
      return false;
   }
   overlapped.clear();
   buffer = ba;
   numberOfBytesToWrite = buffer.size();
   stopped = false;
   writeSequenceStarted = true;
   if (!WriteFileEx(handle, buffer.constData(), numberOfBytesToWrite,
                    &overlapped, &writeFileCompleted)) {
      writeSequenceStarted = false;
      numberOfBytesToWrite = 0;
      buffer.clear();
      qErrnoWarning("QWindowsPipeWriter::write failed.");
      return false;
   }

   return true;
}
void QWindowsPipeWriter::stop()
{
   stopped = true;
   bytesWrittenPending = false;
   pendingBytesWrittenValue = 0;
   if (writeSequenceStarted) {
      if (!qt_cancelIo(handle, &overlapped)) {
         const DWORD dwError = GetLastError();
         if (dwError != ERROR_NOT_FOUND) {
            qErrnoWarning(dwError, "QWindowsPipeWriter: qt_cancelIo on handle %x failed.",
                          handle);
         }
      }
      waitForNotification(-1);
   }
}


