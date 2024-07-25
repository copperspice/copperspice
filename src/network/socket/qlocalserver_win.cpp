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

#include <qdebug.h>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qscopedarraypointer.h>
#include <qstring.h>

#include <qlocalserver_p.h>
#include <qsystemerror_p.h>

#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>

// The buffer size need to be 0 otherwise data could be
// lost if the socket that has written data closes the connection
// before it is read.  Pipewriter is used for write buffering.
#define BUFSIZE 0

// ###: This should be a property. Should replace the insane 50 on unix as well.
#define SYSTEM_MAX_PENDING_SOCKETS 8

bool QLocalServerPrivate::addListener()
{
   // The object must not change its address once the
   // contained OVERLAPPED struct is passed to Windows.
   listeners << Listener();
   Listener &listener = listeners.last();
   SECURITY_ATTRIBUTES sa;
   sa.nLength = sizeof(SECURITY_ATTRIBUTES);
   sa.bInheritHandle = false;                  // non inheritable handle, same as default
   sa.lpSecurityDescriptor = nullptr;         // default security descriptor
   QScopedPointer<SECURITY_DESCRIPTOR> pSD;
   PSID worldSID = nullptr;
   QByteArray aclBuffer;
   QByteArray tokenUserBuffer;
   QByteArray tokenGroupBuffer;

   // create security descriptor if access options were specified
   if ((socketOptions & QLocalServer::WorldAccessOption)) {
      pSD.reset(new SECURITY_DESCRIPTOR);

      if (! InitializeSecurityDescriptor(pSD.data(), SECURITY_DESCRIPTOR_REVISION)) {
         setError("QLocalServerPrivate::addListener");
         return false;
      }

      HANDLE hToken = nullptr;
      if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
         return false;
      }

      DWORD dwBufferSize = 0;
      GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwBufferSize);
      tokenUserBuffer.fill(0, dwBufferSize);
      PTOKEN_USER pTokenUser = (PTOKEN_USER)tokenUserBuffer.data();

      if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize)) {
         setError("QLocalServerPrivate::addListener");
         CloseHandle(hToken);

         return false;
      }

      dwBufferSize = 0;
      GetTokenInformation(hToken, TokenPrimaryGroup, nullptr, 0, &dwBufferSize);
      tokenGroupBuffer.fill(0, dwBufferSize);
      PTOKEN_PRIMARY_GROUP pTokenGroup = (PTOKEN_PRIMARY_GROUP)tokenGroupBuffer.data();

      if (! GetTokenInformation(hToken, TokenPrimaryGroup, pTokenGroup, dwBufferSize, &dwBufferSize)) {
         setError("QLocalServerPrivate::addListener");
         CloseHandle(hToken);
         return false;
      }

      CloseHandle(hToken);

#if defined(CS_SHOW_DEBUG_NETWORK)
      DWORD groupNameSize;
      DWORD domainNameSize;
      SID_NAME_USE groupNameUse;
      LPWSTR groupNameSid;

      LookupAccountSid(nullptr, pTokenGroup->PrimaryGroup, nullptr, &groupNameSize, nullptr, &domainNameSize, &groupNameUse);

      std::wstring groupName(groupNameSize, L'0');
      std::wstring domainName(domainNameSize, L'0');

      if (LookupAccountSid(nullptr, pTokenGroup->PrimaryGroup, groupName.data(), &groupNameSize, domainName.data(),
            &domainNameSize, &groupNameUse)) {

         qDebug() << "primary group" << QString::fromStdWString(domainName) << "\\"
               << QString::fromStdWString(groupName) << "type=" << groupNameUse;
      }

      if (ConvertSidToStringSid(pTokenGroup->PrimaryGroup, &groupNameSid)) {
         qDebug() << "primary group SID" << QString::fromStdWString(std::wstring(groupNameSid)) << "valid"
               << IsValidSid(pTokenGroup->PrimaryGroup);

         LocalFree(groupNameSid);
      }
#endif

      SID_IDENTIFIER_AUTHORITY WorldAuth = { SECURITY_WORLD_SID_AUTHORITY };

      if (! AllocateAndInitializeSid(&WorldAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &worldSID)) {
         setError("QLocalServerPrivate::addListener");
         return false;
      }

      DWORD aclSize = sizeof(ACL) + ((sizeof(ACCESS_ALLOWED_ACE)) * 3);
      aclSize += GetLengthSid(pTokenUser->User.Sid) - sizeof(DWORD);
      aclSize += GetLengthSid(pTokenGroup->PrimaryGroup) - sizeof(DWORD);
      aclSize += GetLengthSid(worldSID) - sizeof(DWORD);
      aclSize = (aclSize + (sizeof(DWORD) - 1)) & 0xfffffffc;
      aclBuffer.fill(0, aclSize);
      PACL acl = (PACL)aclBuffer.data();
      InitializeAcl(acl, aclSize, ACL_REVISION_DS);

      if (socketOptions & QLocalServer::UserAccessOption) {
         if (! AddAccessAllowedAce(acl, ACL_REVISION, FILE_ALL_ACCESS, pTokenUser->User.Sid)) {
            setError("QLocalServerPrivate::addListener");
            FreeSid(worldSID);

            return false;
         }
      }
      if (socketOptions & QLocalServer::GroupAccessOption) {
         if (!AddAccessAllowedAce(acl, ACL_REVISION, FILE_ALL_ACCESS, pTokenGroup->PrimaryGroup)) {
            setError("QLocalServerPrivate::addListener");
            FreeSid(worldSID);
            return false;
         }
      }
      if (socketOptions & QLocalServer::OtherAccessOption) {
         if (!AddAccessAllowedAce(acl, ACL_REVISION, FILE_ALL_ACCESS, worldSID)) {
            setError("QLocalServerPrivate::addListener");
            FreeSid(worldSID);
            return false;
         }
      }

      SetSecurityDescriptorOwner(pSD.data(), pTokenUser->User.Sid, FALSE);
      SetSecurityDescriptorGroup(pSD.data(), pTokenGroup->PrimaryGroup, FALSE);

      if (!SetSecurityDescriptorDacl(pSD.data(), TRUE, acl, FALSE)) {
         setError("QLocalServerPrivate::addListener");
         FreeSid(worldSID);
         return false;
      }

      sa.lpSecurityDescriptor = pSD.data();
   }

   listener.handle = CreateNamedPipe(fullServerName.toStdWString().c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                        PIPE_TYPE_BYTE |          // byte type pipe
                        PIPE_READMODE_BYTE |      // byte-read mode
                        PIPE_WAIT,                // blocking mode
                        PIPE_UNLIMITED_INSTANCES, // max. instances
                        BUFSIZE,                  // output buffer size
                        BUFSIZE,                  // input buffer size
                        3000,                     // client time-out
                        &sa);

   if (listener.handle == INVALID_HANDLE_VALUE) {
      setError("QLocalServerPrivate::addListener");
      listeners.removeLast();

      return false;
   }

   if (worldSID) {
      FreeSid(worldSID);
   }
   memset(&listener.overlapped, 0, sizeof(listener.overlapped));
   listener.overlapped.hEvent = eventHandle;
   if (!ConnectNamedPipe(listener.handle, &listener.overlapped)) {
      switch (GetLastError()) {
         case ERROR_IO_PENDING:
            listener.connected = false;
            break;

         case ERROR_PIPE_CONNECTED:
            listener.connected = true;
            break;

         default:
            CloseHandle(listener.handle);
            setError(QLatin1String("QLocalServerPrivate::addListener"));
            listeners.removeLast();
            return false;
      }
   } else {
      Q_ASSERT_X(false, "QLocalServerPrivate::addListener", "The impossible happened");
      SetEvent(eventHandle);
   }
   return true;
}

void QLocalServerPrivate::setError(const QString &function)
{
   int windowsError = GetLastError();
   errorString = QString::fromLatin1("%1: %2").formatArg(function).formatArg(qt_error_string(windowsError));
   error = QAbstractSocket::UnknownSocketError;
}

void QLocalServerPrivate::init()
{
}

bool QLocalServerPrivate::removeServer(const QString &name)
{
   (void) name;
   return true;
}

bool QLocalServerPrivate::listen(const QString &name)
{
   Q_Q(QLocalServer);

   QString pipePath = QString("\\\\.\\pipe\\");

   if (name.startsWith(pipePath)) {
      fullServerName = name;
   } else {
      fullServerName = pipePath + name;
   }

   // Use only one event for all listeners of one socket.
   // The idea is that listener events are rare, so polling all listeners once in a while is
   // cheap compared to waiting for N additional events in each iteration of the main loop.
   eventHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);
   connectionEventNotifier = new QWinEventNotifier(eventHandle, q);

   q->connect(connectionEventNotifier, &QWinEventNotifier::activated, q, &QLocalServer::_q_onNewConnection);

   for (int i = 0; i < SYSTEM_MAX_PENDING_SOCKETS; ++i) {
      if (! addListener()) {
         return false;
      }
   }

   _q_onNewConnection();
   return true;
}

bool QLocalServerPrivate::listen(qintptr) {
   qWarning("QLocalServer::listen() Not supported on Windows");
   return false;
}

void QLocalServerPrivate::_q_onNewConnection() {
   Q_Q(QLocalServer);
   DWORD dummy;

   bool tryAgain;
   do {
      tryAgain = false;

      // Reset first, otherwise we could reset an event which was asserted
      // immediately after we checked the conn status.
      ResetEvent(eventHandle);

      // Testing shows that there is indeed absolutely no guarantee which listener gets
      // a client connection first, so there is no way around polling all of them.
      for (int i = 0; i < listeners.size(); ) {
         HANDLE handle = listeners[i].handle;

         if (listeners[i].connected
               || GetOverlappedResult(handle, &listeners[i].overlapped, &dummy, FALSE)) {
            listeners.removeAt(i);
            addListener();

            if (pendingConnections.size() > maxPendingConnections) {
               connectionEventNotifier->setEnabled(false);
            }  else {
               tryAgain = true;
            }

            // Make this the last thing so connected slots can wreak the least havoc
            q->incomingConnection((quintptr)handle);

         } else {
            if (GetLastError() != ERROR_IO_INCOMPLETE) {
               q->close();
               setError(QLatin1String("QLocalServerPrivate::_q_onNewConnection"));
               return;
            }

            ++i;
         }

      }
   } while (tryAgain);
}

void QLocalServerPrivate::closeServer() {
   connectionEventNotifier->setEnabled(false); // Otherwise, closed handle is checked before deleter runs
   connectionEventNotifier->deleteLater();
   connectionEventNotifier = nullptr;
   CloseHandle(eventHandle);

   for (int i = 0; i < listeners.size(); ++i) {
      CloseHandle(listeners[i].handle);
   }

   listeners.clear();
}

void QLocalServerPrivate::waitForNewConnection(int msecs, bool * timedOut) {
   Q_Q(QLocalServer);

   if (!pendingConnections.isEmpty() || !q->isListening()) {
      return;
   }

   DWORD result = WaitForSingleObject(eventHandle, (msecs == -1) ? INFINITE : msecs);
   if (result == WAIT_TIMEOUT) {
      if (timedOut) {
         *timedOut = true;
      }

   } else {
      _q_onNewConnection();
   }
}


