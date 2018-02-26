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

#include <qnetworkcookiejar.h>
#include <qnetworkcookiejar_p.h>
#include <QtNetwork/qnetworkcookie.h>
#include <QtCore/qurl.h>
#include <QtCore/qdatetime.h>
#include <qtldurl_p.h>

/*!
    Creates a QNetworkCookieJar object and sets the parent object to
    be \a parent.

    The cookie jar is initialized to empty.
*/
QNetworkCookieJar::QNetworkCookieJar(QObject *parent)
   : QObject(parent), d_ptr(new QNetworkCookieJarPrivate)
{
   d_ptr->q_ptr = this;
}


QNetworkCookieJar::~QNetworkCookieJar()
{
}

/*!
    Returns all cookies stored in this cookie jar. This function is
    suitable for derived classes to save cookies to disk, as well as
    to implement cookie expiration and other policies.

    \sa setAllCookies(), cookiesForUrl()
*/
QList<QNetworkCookie> QNetworkCookieJar::allCookies() const
{
   return d_func()->allCookies;
}

/*!
    Sets the internal list of cookies held by this cookie jar to be \a
    cookieList. This function is suitable for derived classes to
    implement loading cookies from permanent storage, or their own
    cookie acceptance policies by reimplementing
    setCookiesFromUrl().

    \sa allCookies(), setCookiesFromUrl()
*/
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

/*!
    Adds the cookies in the list \a cookieList to this cookie
    jar. Default values for path and domain are taken from the \a
    url object.

    Returns true if one or more cookies are set for \a url,
    otherwise false.

    If a cookie already exists in the cookie jar, it will be
    overridden by those in \a cookieList.

    The default QNetworkCookieJar class implements only a very basic
    security policy (it makes sure that the cookies' domain and path
    match the reply's). To enhance the security policy with your own
    algorithms, override setCookiesFromUrl().

    Also, QNetworkCookieJar does not have a maximum cookie jar
    size. Reimplement this function to discard older cookies to create
    room for new ones.

    \sa cookiesForUrl(), QNetworkAccessManager::setCookieJar()
*/
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

/*!
    Returns the cookies to be added to when a request is sent to
    \a url. This function is called by the default
    QNetworkAccessManager::createRequest(), which adds the
    cookies returned by this function to the request being sent.

    If more than one cookie with the same name is found, but with
    differing paths, the one with longer path is returned before the
    one with shorter path. In other words, this function returns
    cookies sorted decreasingly by path length.

    The default QNetworkCookieJar class implements only a very basic
    security policy (it makes sure that the cookies' domain and path
    match the reply's). To enhance the security policy with your own
    algorithms, override cookiesForUrl().

    \sa setCookiesFromUrl(), QNetworkAccessManager::setCookieJar()
*/
QList<QNetworkCookie> QNetworkCookieJar::cookiesForUrl(const QUrl &url) const
{
   //     \b Warning! This is only a dumb implementation!
   //     It does NOT follow all of the recommendations from
   //     http://wp.netscape.com/newsref/std/cookie_spec.html
   //     It does not implement a very good cross-domain verification yet.

   Q_D(const QNetworkCookieJar);

   const QDateTime now = QDateTime::currentDateTime();
   QList<QNetworkCookie> result;
   bool isEncrypted = url.scheme().toLower() == "https";

   // scan our cookies for something that matches
   QList<QNetworkCookie>::ConstIterator it  = d->allCookies.constBegin();
   QList<QNetworkCookie>::ConstIterator end = d->allCookies.constEnd();

   for ( ; it != end; ++it) {
      if (!isParentDomain(url.host(), it->domain())) {
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
      QList<QNetworkCookie>::Iterator insertIt = result.begin();
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
   bool isDeletion = !cookie.isSessionCookie() &&
                     cookie.expirationDate() < now;
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
   QList<QNetworkCookie>::Iterator it;
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
