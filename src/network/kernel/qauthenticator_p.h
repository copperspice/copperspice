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

#ifndef QAUTHENTICATOR_P_H
#define QAUTHENTICATOR_P_H

#include <qhash.h>
#include <qbytearray.h>
#include <qstring.h>
#include <qauthenticator.h>
#include <qvariant.h>

QT_BEGIN_NAMESPACE

class QHttpResponseHeader;
#ifdef Q_OS_WIN
class QNtlmWindowsHandles;
#endif

class QAuthenticatorPrivate
{
 public:
   enum Method { None, Basic, Plain, Login, Ntlm, CramMd5, DigestMd5 };
   QAuthenticatorPrivate();
   ~QAuthenticatorPrivate();

   QString user;
   QString extractedUser;
   QString password;
   QVariantHash options;
   Method method;
   QString realm;
   QByteArray challenge;

#ifdef Q_OS_WIN
    QNtlmWindowsHandles *ntlmWindowsHandles;
#endif

   bool hasFailed; //credentials have been tried but rejected by server.

   enum Phase {
      Start,
      Phase2,
      Done,
      Invalid
   };
   Phase phase;

   // digest specific
   QByteArray cnonce;
   int nonceCount;

   // ntlm specific
   QString workstation;
   QString userDomain;

   QByteArray calculateResponse(const QByteArray &method, const QByteArray &path);

   inline static QAuthenticatorPrivate *getPrivate(QAuthenticator &auth) {
      return auth.d;
   }
   inline static const QAuthenticatorPrivate *getPrivate(const QAuthenticator &auth) {
      return auth.d;
   }

   QByteArray digestMd5Response(const QByteArray &challenge, const QByteArray &method, const QByteArray &path);
   static QHash<QByteArray, QByteArray> parseDigestAuthenticationChallenge(const QByteArray &challenge);
   void parseHttpResponse(const QList<QPair<QByteArray, QByteArray> > &, bool isProxy);
   void updateCredentials();
};

QT_END_NAMESPACE

#endif
