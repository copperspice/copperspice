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

#ifndef QNETWORKCOOKIE_P_H
#define QNETWORKCOOKIE_P_H

#include <qdatetime.h>

QT_BEGIN_NAMESPACE

class QNetworkCookiePrivate: public QSharedData
{
 public:
   inline QNetworkCookiePrivate() : secure(false), httpOnly(false) { }
   static QList<QNetworkCookie> parseSetCookieHeaderLine(const QByteArray &cookieString);

   QDateTime expirationDate;
   QString domain;
   QString path;
   QString comment;
   QByteArray name;
   QByteArray value;
   bool secure;
   bool httpOnly;
};

static inline bool isLWS(char c)
{
   return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int nextNonWhitespace(const QByteArray &text, int from)
{
   // RFC 2616 defines linear whitespace as:
   //  LWS = [CRLF] 1*( SP | HT )
   // We ignore the fact that CRLF must come as a pair at this point
   // It's an invalid HTTP header if that happens.
   while (from < text.length()) {
      if (isLWS(text.at(from))) {
         ++from;
      } else {
         return from;   // non-whitespace
      }
   }

   // reached the end
   return text.length();
}


#endif
