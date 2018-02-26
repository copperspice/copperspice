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

#ifndef QNETWORKCOOKIEJAR_H
#define QNETWORKCOOKIEJAR_H

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QNetworkCookie;
class QNetworkCookieJarPrivate;

class Q_NETWORK_EXPORT QNetworkCookieJar: public QObject
{
   NET_CS_OBJECT(QNetworkCookieJar)

 public:
   QNetworkCookieJar(QObject *parent = nullptr);
   virtual ~QNetworkCookieJar();

   virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
   virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

   virtual bool insertCookie(const QNetworkCookie &cookie);
   virtual bool updateCookie(const QNetworkCookie &cookie);
   virtual bool deleteCookie(const QNetworkCookie &cookie);
 protected:
   QList<QNetworkCookie> allCookies() const;
   void setAllCookies(const QList<QNetworkCookie> &cookieList);
   virtual bool validateCookie(const QNetworkCookie &cookie, const QUrl &url) const;

   QScopedPointer<QNetworkCookieJarPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QNetworkCookieJar)
   Q_DISABLE_COPY(QNetworkCookieJar)

};

QT_END_NAMESPACE

#endif
