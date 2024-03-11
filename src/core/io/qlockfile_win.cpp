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

/*****************************************************************
** Copyright (c) 2013 David Faure <faure+bluesystems@kde.org>
*****************************************************************/

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qthread.h>
#include <qt_windows.h>

#include <qlockfile_p.h>
#include <qfilesystementry_p.h>

#include <psapi.h>

static inline QByteArray localHostName()
{
   return qgetenv("COMPUTERNAME");
}

static inline bool fileExists(const wchar_t *fileName)
{
   WIN32_FILE_ATTRIBUTE_DATA  data;
   return GetFileAttributesEx(fileName, GetFileExInfoStandard, &data);
}

QLockFile::LockError QLockFilePrivate::tryLock_sys()
{
   const QFileSystemEntry fileEntry(fileName);

   // When writing, allow others to read.
   // When reading, QFile will allow others to read and write, all good.
   // Adding FILE_SHARE_DELETE would allow forceful deletion of stale files,
   // but Windows does not allow recreating it while this handle is open anyway,
   // so this would only create confusion (ca not lock, but no lock file to read from).
   const DWORD dwShareMode = FILE_SHARE_READ;

   SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };

   HANDLE fh = CreateFile(fileEntry.nativeFilePath().toStdWString().c_str(), GENERIC_WRITE, dwShareMode, &securityAtts,
         CREATE_NEW, // error if already exists
         FILE_ATTRIBUTE_NORMAL, nullptr);

   if (fh == INVALID_HANDLE_VALUE) {
      const DWORD lastError = GetLastError();

      switch (lastError) {
         case ERROR_SHARING_VIOLATION:
         case ERROR_ALREADY_EXISTS:
         case ERROR_FILE_EXISTS:
            return QLockFile::LockFailedError;

         case ERROR_ACCESS_DENIED:
            // readonly file, or file still in use by another process.
            // Assume the latter if the file exists, since we don't create it readonly.
            return fileExists(fileEntry.nativeFilePath().toStdWString().c_str())
                  ? QLockFile::LockFailedError : QLockFile::PermissionError;

         default:
            qWarning("QLockFilePrivate::tryLock_sys() Unexpected lock error %ld", lastError);
            return QLockFile::UnknownError;
      }
   }

   // hold the lock and continue
   fileHandle = fh;

   // Assemble data to write in a single call to write
   QByteArray fileData;

   fileData += QByteArray::number(QCoreApplication::applicationPid());
   fileData += '\n';
   fileData += QCoreApplication::applicationName().toUtf8();
   fileData += '\n';
   fileData += localHostName();
   fileData += '\n';

   DWORD bytesWritten = 0;
   QLockFile::LockError error = QLockFile::NoError;

   if (! WriteFile(fh, fileData.constData(), fileData.size(), &bytesWritten, nullptr) || ! FlushFileBuffers(fh)) {
      error = QLockFile::UnknownError;   // partition full
   }

   return error;
}

bool QLockFilePrivate::removeStaleLock()
{
   // QFile::remove fails on Windows if the other process is still using the file
   return QFile::remove(fileName);
}

bool QLockFilePrivate::isApparentlyStale() const
{
   qint64 pid;
   QString hostname, appname;

   // On WinRT there seems to be no way of obtaining information about other
   // processes due to sandboxing

   if (getLockInfo(&pid, &hostname, &appname)) {
      if (hostname.isEmpty() || hostname == QString::fromUtf8(localHostName())) {
         HANDLE procHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

         if (! procHandle) {
            return true;
         }

         // have a handle but check if process is still alive
         DWORD exitCode = 0;

         if (! ::GetExitCodeProcess(procHandle, &exitCode)) {
            exitCode = 0;
         }

         ::CloseHandle(procHandle);

         if (exitCode != STILL_ACTIVE) {
            return true;
         }

         const QString processName = processNameByPid(pid);

         if (! processName.isEmpty() && processName != appname) {
            return true;   // PID got reused by a different application.
         }
      }
   }

   const qint64 age = QFileInfo(fileName).lastModified().msecsTo(QDateTime::currentDateTime());

   return staleLockTime > 0 && age > staleLockTime;
}

QString QLockFilePrivate::processNameByPid(qint64 pid)
{
   HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, DWORD(pid));

   if (! hProcess) {
      return QString();
   }

   std::wstring buffer(MAX_PATH, L'\0');
   const DWORD length = GetModuleFileNameEx(hProcess, nullptr, &buffer[0], buffer.size());

   CloseHandle(hProcess);

   if (! length) {
      return QString();
   }

   QString name = QString::fromStdWString(buffer);
   int i = name.lastIndexOf('\\');

   if (i >= 0) {
      name.remove(0, i + 1);
   }

   i = name.lastIndexOf('.');

   if (i >= 0) {
      name.truncate(i);
   }

   return name;
}

void QLockFile::unlock()
{
   Q_D(QLockFile);

   static constexpr const int maxAttempts = 500;   // 500ms

   if (! d->isLocked) {
      return;
   }

   CloseHandle(d->fileHandle);
   int attempts = 0;

   while (! QFile::remove(d->fileName) && ++attempts < maxAttempts) {
      // Someone is reading the lock file right now, unable to delete
      QThread::msleep(1);
   }

   if (attempts == maxAttempts) {
      qWarning("QLockFile::unlock() Unable to remove file %s", csPrintable(d->fileName));
   }

   d->lockError = QLockFile::NoError;
   d->isLocked = false;
}
