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

#include <qnetwork_cookiejar.h>

#include <qdatetime.h>
#include <qnetwork_cookie.h>
#include <qurl.h>

#include <qnetwork_cookiejar_p.h>
#include <qtldurl_p.h>

QNetworkCookieJar::QNetworkCookieJar(QObject *parent)
   : QObject(parent), d_ptr(new QNetworkCookieJarPrivate)
{
   d_ptr->q_ptr = this;
}

QNetworkCookieJar::~QNetworkCookieJar()
{
}

QList<QNetworkCookie> QNetworkCookieJar::allCookies() const
{
   return d_func()->allCookies;
}

void QNetworkCookieJar::setAllCookies(const QList<QNetworkCookie> &cookieList)
{
   Q_D(QNetworkCookieJar);
   d->allCookies = cookieList;
}

static inline bool isParentPath(const QString &path, const QString &reference)
{
   if (path.startsWith(reference)) {

      if (path.length() == reference.length()) {
         return true;
      }
      if (reference.endsWith('/')) {
         return true;
      }
      if (path.at(reference.length()) == '/') {
         return true;
      }
   }

   return false;
}

static inline bool isParentDomain(const QString &domain, const QString &reference)
{
   if (! reference.startsWith('.')) {
      return domain == reference;
   }

   return domain.endsWith(reference) || domain == reference.mid(1);
}

bool QNetworkCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList,
      const QUrl &url)
{
   bool added = false;

   for (QNetworkCookie cookie : cookieList) {
      cookie.normalize(url);


      if (validateCookie(cookie, url) && insertCookie(cookie)) {
         added = true;
      }
   }

   return added;
}

QList<QNetworkCookie> QNetworkCookieJar::cookiesForUrl(const QUrl &url) const
{
   //  This is only a weak implementation
   //  It does NOT follow all of the recommendations from
   //  http://wp.netscape.com/newsref/std/cookie_spec.html
   //  It does not implement a very good cross-domain verification yet.

   Q_D(const QNetworkCookieJar);

   const QDateTime now = QDateTime::currentDateTime();
   QList<QNetworkCookie> result;
   bool isEncrypted = url.scheme().toLower() == "https";

   // scan our cookies for something that matches
   QList<QNetworkCookie>::const_iterator it  = d->allCookies.constBegin();
   QList<QNetworkCookie>::const_iterator end = d->allCookies.constEnd();

   for ( ; it != end; ++it) {
      if (! isParentDomain(url.host(), it->domain())) {
         continue;
      }

      if (!isParentPath(url.path(), it->path())) {
         continue;
      }

      if (!(*it).isSessionCookie() && (*it).expirationDate() < now) {
         continue;
      }

      if ((*it).isSecure() && !isEncrypted) {
         continue;
      }

      // insert this cookie into result, sorted by path
      QList<QNetworkCookie>::iterator insertIt = result.begin();

      while (insertIt != result.end()) {
         if (insertIt->path().length() < it->path().length()) {
            // insert here
            insertIt = result.insert(insertIt, *it);
            break;
         } else {
            ++insertIt;
         }
      }

      // this is the shortest path yet, just append
      if (insertIt == result.end()) {
         result += *it;
      }
   }

   return result;
}

bool QNetworkCookieJar::insertCookie(const QNetworkCookie &cookie)
{
   Q_D(QNetworkCookieJar);
   const QDateTime now = QDateTime::currentDateTimeUtc();

   bool isDeletion = !cookie.isSessionCookie() && cookie.expirationDate() < now;
   deleteCookie(cookie);

   if (!isDeletion) {
      d->allCookies += cookie;
      return true;
   }

   return false;
}

bool QNetworkCookieJar::updateCookie(const QNetworkCookie &cookie)
{
   if (deleteCookie(cookie)) {
      return insertCookie(cookie);
   }
   return false;
}

bool QNetworkCookieJar::deleteCookie(const QNetworkCookie &cookie)
{
   Q_D(QNetworkCookieJar);

   QList<QNetworkCookie>::iterator it;

   for (it = d->allCookies.begin(); it != d->allCookies.end(); ++it) {
      if (it->hasSameIdentifier(cookie)) {
         d->allCookies.erase(it);
         return true;
      }
   }

   return false;
}

bool QNetworkCookieJar::validateCookie(const QNetworkCookie &cookie, const QUrl &url) const
{
   QString domain = cookie.domain();

   if (!(isParentDomain(domain, url.host()) || isParentDomain(url.host(), domain))) {
      return false;   // not accepted
   }

   if (qIsEffectiveTLD(domain.startsWith('.') ? domain.remove(0, 1) : domain)) {
      return false;   // not accepted
   }

   return true;
}
