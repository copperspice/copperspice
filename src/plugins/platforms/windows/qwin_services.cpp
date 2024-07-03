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

#include <qwin_services.h>

#include <qurl.h>
#include <qdebug.h>
#include <qdir.h>
#include <qwin_additional.h>

#include <shlobj.h>
#include <intshcut.h>

static inline bool shellExecute(const QUrl &url)
{
   const QString nativeFilePath = url.isLocalFile() && ! url.hasFragment() && !url.hasQuery()
      ? QDir::toNativeSeparators(url.toLocalFile())
      : url.toString(QUrl::FullyEncoded);

   const quintptr result = reinterpret_cast<quintptr>(ShellExecute(nullptr, nullptr, nativeFilePath.toStdWString().data(),
                nullptr, nullptr, SW_SHOWNORMAL));

   // ShellExecute returns a value greater than 32 if successful
   if (result <= 32) {
      qWarning("ShellExecute '%s' failed (error %s).", csPrintable(url.toString()), csPrintable(QString::number(result)));
      return false;
   }

   return true;
}

// Retrieve the commandline for the default mail client. It contains a
// placeholder %1 for the URL. The default key used below is the
// command line for the mailto: shell command.
static inline QString mailCommand()
{
   static constexpr const auto BufferSize = sizeof(wchar_t) * MAX_PATH;

   const wchar_t mailUserKey[] = L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\mailto\\UserChoice";

   wchar_t command[MAX_PATH] = {0};

   // Check if user has set preference, otherwise use default.
   HKEY handle;
   QString keyName;

   if (! RegOpenKeyEx(HKEY_CURRENT_USER, mailUserKey, 0, KEY_READ, &handle)) {
      DWORD bufferSize = BufferSize;

      if (! RegQueryValueEx(handle, L"Progid", nullptr, nullptr, reinterpret_cast<unsigned char *>(command), &bufferSize)) {
         keyName = QString::fromStdWString(std::wstring(command));
      }

      RegCloseKey(handle);
   }

   if (keyName.isEmpty()) {
      keyName = "mailto";
   }

   keyName += "\\Shell\\Open\\Command";

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << __FUNCTION__ << "keyName=" << keyName;
#endif

   command[0] = 0;

   if (! RegOpenKeyExW(HKEY_CLASSES_ROOT, keyName.toStdWString().data(), 0, KEY_READ, &handle)) {

      DWORD bufferSize = BufferSize;
      RegQueryValueEx(handle, L"", nullptr, nullptr, reinterpret_cast<unsigned char *>(command), &bufferSize);
      RegCloseKey(handle);
   }

   if (! command[0]) {
      return QString();
   }

   wchar_t expandedCommand[MAX_PATH] = {0};

   return ExpandEnvironmentStrings(command, expandedCommand, MAX_PATH) ?
      QString::fromStdWString(std::wstring(expandedCommand)) : QString::fromStdWString(std::wstring(command));
}

static inline bool launchMail(const QUrl &url)
{
   QString command = mailCommand();

   if (command.isEmpty()) {
      qWarning("Unable to launch '%s', there is no mail program installed.", csPrintable(url.toString()));
      return false;
   }

   //Make sure the path for the process is in quotes
   const QChar doubleQuote = '"';

   if (!command.startsWith(doubleQuote)) {
      const int exeIndex = command.indexOf(".exe ", 0, Qt::CaseInsensitive);

      if (exeIndex != -1) {
         command.insert(exeIndex + 4, doubleQuote);
         command.prepend(doubleQuote);
      }
   }

   // Pass the url as the parameter. Should use QProcess::startDetached(),
   // but that cannot handle a Windows command line [yet].
   command.replace(QString("%1"), url.toString(QUrl::FullyEncoded));

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << __FUNCTION__ << "Launching" << command;
#endif

   //start the process
   PROCESS_INFORMATION pi;
   ZeroMemory(&pi, sizeof(pi));

   STARTUPINFO si;
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);

   std::wstring tmp = command.toStdWString();

   if (! CreateProcess(nullptr, &tmp[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
      qErrnoWarning("Unable to launch '%s'", csPrintable(command));
      return false;
   }

   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   return true;
}

bool QWindowsServices::openUrl(const QUrl &url)
{
   const QString scheme = url.scheme();

   if (scheme == "mailto" && launchMail(url)) {
      return true;
   }

   return shellExecute(url);
}

bool QWindowsServices::openDocument(const QUrl &url)
{
   return shellExecute(url);
}

