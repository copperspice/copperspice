/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

QT_BEGIN_NAMESPACE

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

/*!
    Destroys this cookie jar object and discards all cookies stored in
    it. Cookies are not saved to disk in the QNetworkCookieJar default
    implementation.

    If you need to save the cookies to disk, you have to derive from
    QNetworkCookieJar and save the cookies to disk yourself.
*/
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

static inline bool isParentPath(QString path, QString reference)
{
   if (!path.endsWith(QLatin1Char('/'))) {
      path += QLatin1Char('/');
   }
   if (!reference.endsWith(QLatin1Char('/'))) {
      reference += QLatin1Char('/');
   }
   return path.startsWith(reference);
}

static inline bool isParentDomain(QString domain, QString reference)
{
   if (!reference.startsWith(QLatin1Char('.'))) {
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
   Q_D(QNetworkCookieJar);
   QString defaultDomain = url.host();
   QString pathAndFileName = url.path();
   QString defaultPath = pathAndFileName.left(pathAndFileName.lastIndexOf(QLatin1Char('/')) + 1);
   if (defaultPath.isEmpty()) {
      defaultPath = QLatin1Char('/');
   }

   int added = 0;
   QDateTime now = QDateTime::currentDateTime();

   for (QNetworkCookie cookie : cookieList) {
      bool isDeletion = !cookie.isSessionCookie() && cookie.expirationDate() < now;

      // validate the cookie & set the defaults if unset
      if (cookie.path().isEmpty()) {
         cookie.setPath(defaultPath);
      }
      // don't do path checking. See http://bugreports.qt-project.org/browse/QTBUG-5815
      //        else if (!isParentPath(pathAndFileName, cookie.path())) {
      //            continue;           // not accepted
      //        }
      if (cookie.domain().isEmpty()) {
         cookie.setDomain(defaultDomain);
      } else {
         // Ensure the domain starts with a dot if its field was not empty
         // in the HTTP header. There are some servers that forget the
         // leading dot and this is actually forbidden according to RFC 2109,
         // but all browsers accept it anyway so we do that as well.
         if (!cookie.domain().startsWith(QLatin1Char('.'))) {
            cookie.setDomain(QLatin1Char('.') + cookie.domain());
         }

         QString domain = cookie.domain();
         if (!(isParentDomain(domain, defaultDomain)
               || isParentDomain(defaultDomain, domain))) {
            continue;   // not accepted
         }

         // the check for effective TLDs makes the "embedded dot" rule from RFC 2109 section 4.3.2
         // redundant; the "leading dot" rule has been relaxed anyway, see above
         // we remove the leading dot for this check
         if (qIsEffectiveTLD(domain.remove(0, 1))) {
            continue;   // not accepted
         }
      }

      for (int i = 0; i < d->allCookies.size(); ++i) {
         // does this cookie already exist?
         const QNetworkCookie &current = d->allCookies.at(i);
         if (cookie.name() == current.name() &&
               cookie.domain() == current.domain() &&
               cookie.path() == current.path()) {
            // found a match
            d->allCookies.removeAt(i);
            break;
         }
      }

      // did not find a match
      if (!isDeletion) {
         int countForDomain = 0;
         for (int i = d->allCookies.size() - 1; i >= 0; --i) {
            // Start from the end and delete the oldest cookies to keep a maximum count of 50.
            const QNetworkCookie &current = d->allCookies.at(i);
            if (isParentDomain(cookie.domain(), current.domain())
                  || isParentDomain(current.domain(), cookie.domain())) {
               if (countForDomain >= 49) {
                  d->allCookies.removeAt(i);
               } else {
                  ++countForDomain;
               }
            }
         }

         d->allCookies += cookie;
         ++added;
      }
   }
   return (added > 0);
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
   QDateTime now = QDateTime::currentDateTime();
   QList<QNetworkCookie> result;
   bool isEncrypted = url.scheme().toLower() == QLatin1String("https");

   // scan our cookies for something that matches
   QList<QNetworkCookie>::ConstIterator it = d->allCookies.constBegin(),
                                        end = d->allCookies.constEnd();
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

QT_END_NAMESPACE
