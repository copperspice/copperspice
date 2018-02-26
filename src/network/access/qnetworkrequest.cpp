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

#include <qplatformdefs.h>
#include <qnetworkrequest.h>
#include <qnetworkcookie.h>
#include <qnetworkrequest_p.h>
#include <qsslconfiguration.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qlocale.h>
#include <QtCore/qdatetime.h>

#include <ctype.h>
#ifndef QT_NO_DATESTRING
# include <stdio.h>
#endif

class QNetworkRequestPrivate: public QSharedData, public QNetworkHeadersPrivate
{
 public:
   static const int maxRedirectCount = 50;

   inline QNetworkRequestPrivate()
      : priority(QNetworkRequest::NormalPriority)
#ifdef QT_SSL
      , sslConfiguration(0)
#endif
      , maxRedirectsAllowed(maxRedirectCount)
   { }

   ~QNetworkRequestPrivate() {

#ifdef QT_SSL
      delete sslConfiguration;
#endif
   }

   QNetworkRequestPrivate(const QNetworkRequestPrivate &other)
      : QSharedData(other), QNetworkHeadersPrivate(other) {
      url = other.url;
      priority = other.priority;
      maxRedirectsAllowed = other.maxRedirectsAllowed;

#ifdef QT_SSL
      sslConfiguration = 0;
      if (other.sslConfiguration) {
         sslConfiguration = new QSslConfiguration(*other.sslConfiguration);
      }
#endif
   }

   inline bool operator==(const QNetworkRequestPrivate &other) const {
      return url == other.url &&
             priority == other.priority &&
             rawHeaders == other.rawHeaders &&
             attributes == other.attributes &&
             maxRedirectsAllowed == other.maxRedirectsAllowed;
      // don't compare cookedHeaders
   }

   QUrl url;
   QNetworkRequest::Priority priority;

#ifdef QT_SSL
   mutable QSslConfiguration *sslConfiguration;
#endif
   int maxRedirectsAllowed;
};


QNetworkRequest::QNetworkRequest(const QUrl &url)
   : d(new QNetworkRequestPrivate)
{
   d->url = url;
}

QNetworkRequest::QNetworkRequest(const QNetworkRequest &other)
   : d(other.d)
{
}

QNetworkRequest::~QNetworkRequest()
{
   // QSharedDataPointer auto deletes
   d = 0;
}

bool QNetworkRequest::operator==(const QNetworkRequest &other) const
{
   return d == other.d || *d == *other.d;
}

QNetworkRequest &QNetworkRequest::operator=(const QNetworkRequest &other)
{
   d = other.d;
   return *this;
}


QUrl QNetworkRequest::url() const
{
   return d->url;
}

void QNetworkRequest::setUrl(const QUrl &url)
{
   d->url = url;
}

QVariant QNetworkRequest::header(KnownHeaders header) const
{
   return d->cookedHeaders.value(header);
}

void QNetworkRequest::setHeader(KnownHeaders header, const QVariant &value)
{
   d->setCookedHeader(header, value);
}

bool QNetworkRequest::hasRawHeader(const QByteArray &headerName) const
{
   return d->findRawHeader(headerName) != d->rawHeaders.constEnd();
}

QByteArray QNetworkRequest::rawHeader(const QByteArray &headerName) const
{
   QNetworkHeadersPrivate::RawHeadersList::ConstIterator it = d->findRawHeader(headerName);

   if (it != d->rawHeaders.constEnd()) {
      return it->second;
   }

   return QByteArray();
}

QList<QByteArray> QNetworkRequest::rawHeaderList() const
{
   return d->rawHeadersKeys();
}

void QNetworkRequest::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
   d->setRawHeader(headerName, headerValue);
}

QVariant QNetworkRequest::attribute(Attribute code, const QVariant &defaultValue) const
{
   return d->attributes.value(code, defaultValue);
}

void QNetworkRequest::setAttribute(Attribute code, const QVariant &value)
{
   if (value.isValid()) {
      d->attributes.insert(code, value);
   } else {
      d->attributes.remove(code);
   }
}

#ifdef QT_SSL

QSslConfiguration QNetworkRequest::sslConfiguration() const
{
   if (!d->sslConfiguration) {
      d->sslConfiguration = new QSslConfiguration(QSslConfiguration::defaultConfiguration());
   }
   return *d->sslConfiguration;
}

void QNetworkRequest::setSslConfiguration(const QSslConfiguration &config)
{
   if (!d->sslConfiguration) {
      d->sslConfiguration = new QSslConfiguration(config);
   } else {
      *d->sslConfiguration = config;
   }
}
#endif

void QNetworkRequest::setOriginatingObject(QObject *object)
{
   d->originatingObject = object;
}

QObject *QNetworkRequest::originatingObject() const
{
   return d->originatingObject.data();
}

QNetworkRequest::Priority QNetworkRequest::priority() const
{
   return d->priority;
}

void QNetworkRequest::setPriority(Priority priority)
{
   d->priority = priority;
}
int QNetworkRequest::maximumRedirectsAllowed() const
{
   return d->maxRedirectsAllowed;
}
void QNetworkRequest::setMaximumRedirectsAllowed(int maxRedirectsAllowed)
{
   d->maxRedirectsAllowed = maxRedirectsAllowed;
}

static QByteArray headerName(QNetworkRequest::KnownHeaders header)
{
   switch (header) {
      case QNetworkRequest::ContentTypeHeader:
         return "Content-Type";

      case QNetworkRequest::ContentLengthHeader:
         return "Content-Length";

      case QNetworkRequest::LocationHeader:
         return "Location";

      case QNetworkRequest::LastModifiedHeader:
         return "Last-Modified";

      case QNetworkRequest::CookieHeader:
         return "Cookie";

      case QNetworkRequest::SetCookieHeader:
         return "Set-Cookie";

      case QNetworkRequest::ContentDispositionHeader:
         return "Content-Disposition";

      case QNetworkRequest::UserAgentHeader:
         return "User-Agent";
      case QNetworkRequest::ServerHeader:
         return "Server";
         // no default:
         // if new values are added, this will generate a compiler warning
   }

   return QByteArray();
}

static QByteArray headerValue(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   switch (header) {
      case QNetworkRequest::ContentTypeHeader:
      case QNetworkRequest::ContentLengthHeader:
      case QNetworkRequest::ContentDispositionHeader:
      case QNetworkRequest::UserAgentHeader:
      case QNetworkRequest::ServerHeader:
         return value.toByteArray();

      case QNetworkRequest::LocationHeader:
         switch (value.userType()) {
            case QMetaType::QUrl:
               return value.toUrl().toEncoded();

            default:
               return value.toByteArray();
         }

      case QNetworkRequest::LastModifiedHeader:
         switch (value.userType()) {
            case QMetaType::QDate:
            case QMetaType::QDateTime:
               // generate RFC 1123/822 dates:
               return QNetworkHeadersPrivate::toHttpDate(value.toDateTime());

            default:
               return value.toByteArray();
         }

      case QNetworkRequest::CookieHeader: {
         QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(value);
         if (cookies.isEmpty() && value.userType() == qMetaTypeId<QNetworkCookie>()) {
            cookies << qvariant_cast<QNetworkCookie>(value);
         }

         QByteArray result;
         bool first = true;
         for (const QNetworkCookie &cookie : cookies) {
            if (!first) {
               result += "; ";
            }
            first = false;
            result += cookie.toRawForm(QNetworkCookie::NameAndValueOnly);
         }
         return result;
      }

      case QNetworkRequest::SetCookieHeader: {
         QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(value);
         if (cookies.isEmpty() && value.userType() == qMetaTypeId<QNetworkCookie>()) {
            cookies << qvariant_cast<QNetworkCookie>(value);
         }

         QByteArray result;
         bool first = true;
         for (const QNetworkCookie &cookie : cookies) {
            if (!first) {
               result += ", ";
            }
            first = false;
            result += cookie.toRawForm(QNetworkCookie::Full);
         }
         return result;
      }
   }

   return QByteArray();
}

static int parseHeaderName(const QByteArray &headerName)
{
   if (headerName.isEmpty()) {
      return -1;
   }

   switch (tolower(headerName.at(0))) {
      case 'c':
         if (qstricmp(headerName.constData(), "content-type") == 0) {
            return QNetworkRequest::ContentTypeHeader;
         }

         else if (qstricmp(headerName.constData(), "content-length") == 0) {
            return QNetworkRequest::ContentLengthHeader;
         }

         else if (qstricmp(headerName.constData(), "cookie") == 0) {
            return QNetworkRequest::CookieHeader;
         }

         break;

      case 'l':
         if (qstricmp(headerName.constData(), "location") == 0) {
            return QNetworkRequest::LocationHeader;
         }

         else if (qstricmp(headerName.constData(), "last-modified") == 0) {
            return QNetworkRequest::LastModifiedHeader;
         }

         break;

      case 's':
         if (qstricmp(headerName.constData(), "set-cookie") == 0) {
            return QNetworkRequest::SetCookieHeader;

         } else if (qstricmp(headerName.constData(), "server") == 0) {
            return QNetworkRequest::ServerHeader;

         }

         break;

      case 'u':
         if (qstricmp(headerName.constData(), "user-agent") == 0) {
            return QNetworkRequest::UserAgentHeader;
         }
         break;
   }

   return -1; // nothing found
}

static QVariant parseHttpDate(const QByteArray &raw)
{
   QDateTime dt = QNetworkHeadersPrivate::fromHttpDate(raw);
   if (dt.isValid()) {
      return dt;
   }

   return QVariant();          // transform an invalid QDateTime into a null QVariant
}

static QVariant parseCookieHeader(const QByteArray &raw)
{
   QList<QNetworkCookie> result;
   QList<QByteArray> cookieList = raw.split(';');

   for (const QByteArray &cookie : cookieList) {
      QList<QNetworkCookie> parsed = QNetworkCookie::parseCookies(cookie.trimmed());
      if (parsed.count() != 1) {
         return QVariant();   // invalid Cookie: header
      }

      result += parsed;
   }

   return QVariant::fromValue(result);
}

static QVariant parseHeaderValue(QNetworkRequest::KnownHeaders header, const QByteArray &value)
{
   // header is always a valid value
   switch (header) {
      case QNetworkRequest::UserAgentHeader:
      case QNetworkRequest::ServerHeader:
      case QNetworkRequest::ContentTypeHeader:
         // copy exactly, convert to QString
         return QString::fromLatin1(value);

      case QNetworkRequest::ContentLengthHeader: {
         bool ok;
         qint64 result = value.trimmed().toLongLong(&ok);
         if (ok) {
            return result;
         }
         return QVariant();
      }

      case QNetworkRequest::LocationHeader: {
         QUrl result = QUrl::fromEncoded(value, QUrl::StrictMode);
         if (result.isValid() && !result.scheme().isEmpty()) {
            return result;
         }
         return QVariant();
      }

      case QNetworkRequest::LastModifiedHeader:
         return parseHttpDate(value);

      case QNetworkRequest::CookieHeader:
         return parseCookieHeader(value);

      case QNetworkRequest::SetCookieHeader:
         return QVariant::fromValue(QNetworkCookie::parseCookies(value));

      default:
         Q_ASSERT(0);
   }
   return QVariant();
}

QNetworkHeadersPrivate::RawHeadersList::ConstIterator
QNetworkHeadersPrivate::findRawHeader(const QByteArray &key) const
{
   RawHeadersList::ConstIterator it = rawHeaders.constBegin();
   RawHeadersList::ConstIterator end = rawHeaders.constEnd();
   for ( ; it != end; ++it)
      if (qstricmp(it->first.constData(), key.constData()) == 0) {
         return it;
      }

   return end;                 // not found
}

QNetworkHeadersPrivate::RawHeadersList QNetworkHeadersPrivate::allRawHeaders() const
{
   return rawHeaders;
}

QList<QByteArray> QNetworkHeadersPrivate::rawHeadersKeys() const
{
   QList<QByteArray> result;
   result.reserve(rawHeaders.size());
   RawHeadersList::ConstIterator it = rawHeaders.constBegin(), end = rawHeaders.constEnd();
   for ( ; it != end; ++it) {
      result << it->first;
   }

   return result;
}

void QNetworkHeadersPrivate::setRawHeader(const QByteArray &key, const QByteArray &value)
{
   if (key.isEmpty()) {
      // refuse to accept an empty raw header
      return;
   }

   setRawHeaderInternal(key, value);
   parseAndSetHeader(key, value);
}

/*!
    \internal
    Sets the internal raw headers list to match \a list. The cooked headers
    will also be updated.

    If \a list contains duplicates, they will be stored, but only the first one
    is usually accessed.
*/
void QNetworkHeadersPrivate::setAllRawHeaders(const RawHeadersList &list)
{
   cookedHeaders.clear();
   rawHeaders = list;

   RawHeadersList::ConstIterator it = rawHeaders.constBegin();
   RawHeadersList::ConstIterator end = rawHeaders.constEnd();
   for ( ; it != end; ++it) {
      parseAndSetHeader(it->first, it->second);
   }
}

void QNetworkHeadersPrivate::setCookedHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   QByteArray name = headerName(header);

   if (name.isEmpty()) {
      // headerName verifies that a header is a known value
      qWarning("QNetworkRequest::setHeader  Invalid header value KnownHeader(%d) received", header);
      return;
   }

   if (value.isNull()) {
      setRawHeaderInternal(name, QByteArray());
      cookedHeaders.remove(header);

   } else {

      QByteArray rawValue = headerValue(header, value);

      if (rawValue.isEmpty()) {
         qWarning("QNetworkRequest::setHeader: QVariant of type %s can not be used with header %s", value.typeName(),
                  name.constData());
         return;
      }

      setRawHeaderInternal(name, rawValue);
      cookedHeaders.insert(header, value);
   }
}

void QNetworkHeadersPrivate::setRawHeaderInternal(const QByteArray &key, const QByteArray &value)
{
   RawHeadersList::Iterator it = rawHeaders.begin();

   while (it != rawHeaders.end()) {
      if (qstricmp(it->first.constData(), key.constData()) == 0) {
         it = rawHeaders.erase(it);
      } else {
         ++it;
      }
   }

   if (value.isNull()) {
      return;                 // only wanted to erase key
   }

   RawHeaderPair pair;
   pair.first = key;
   pair.second = value;

   rawHeaders.append(pair);
}

void QNetworkHeadersPrivate::parseAndSetHeader(const QByteArray &key, const QByteArray &value)
{
   // is it a known header?
   const int parsedKeyAsInt = parseHeaderName(key);

   if (parsedKeyAsInt != -1) {
      const QNetworkRequest::KnownHeaders parsedKey
         = static_cast<QNetworkRequest::KnownHeaders>(parsedKeyAsInt);
      if (value.isNull()) {
         cookedHeaders.remove(parsedKey);

      } else if (parsedKey == QNetworkRequest::ContentLengthHeader
                 && cookedHeaders.contains(QNetworkRequest::ContentLengthHeader)) {
         // Only set the cooked header "Content-Length" once.
         // See bug QTBUG-15311

      } else {
         cookedHeaders.insert(parsedKey, parseHeaderValue(parsedKey, value));

      }
   }
}

// Fast month string to int conversion. This code
// assumes that the Month name is correct and that
// the string is at least three chars long.
static int name_to_month(const char *month_str)
{
   switch (month_str[0]) {
      case 'J':
         switch (month_str[1]) {
            case 'a':
               return 1;

            case 'u':
               switch (month_str[2] ) {
                  case 'n':
                     return 6;

                  case 'l':
                     return 7;

               }
         }
         break;
      case 'F':
         return 2;

      case 'M':
         switch (month_str[2] ) {
            case 'r':
               return 3;

            case 'y':
               return 5;

         }
         break;
      case 'A':
         switch (month_str[1]) {
            case 'p':
               return 4;

            case 'u':
               return 8;

         }
         break;
      case 'O':
         return 10;

      case 'S':
         return 9;

      case 'N':
         return 11;

      case 'D':
         return 12;

   }

   return 0;
}

QDateTime QNetworkHeadersPrivate::fromHttpDate(const QByteArray &value)
{
   // HTTP dates have three possible formats:
   //  RFC 1123/822      -   ddd, dd MMM yyyy hh:mm:ss "GMT"
   //  RFC 850           -   dddd, dd-MMM-yy hh:mm:ss "GMT"
   //  ANSI C's asctime  -   ddd MMM d hh:mm:ss yyyy
   // We only handle them exactly. If they deviate, we bail out.

   int pos = value.indexOf(',');
   QDateTime dt;
#ifndef QT_NO_DATESTRING
   if (pos == -1) {
      // no comma -> asctime(3) format
      dt = QDateTime::fromString(QString::fromLatin1(value), Qt::TextDate);
   } else {
      // Use sscanf over QLocal/QDateTimeParser for speed reasons. See the
      // QtWebKit performance benchmarks to get an idea.
      if (pos == 3) {
         char month_name[4];
         int day, year, hour, minute, second;
         if (sscanf(value.constData(), "%*3s, %d %3s %d %d:%d:%d 'GMT'", &day, month_name, &year, &hour, &minute,
                    &second) == 6) {
            dt = QDateTime(QDate(year, name_to_month(month_name), day), QTime(hour, minute, second));
         }
      } else {
         QLocale c = QLocale::c();
         // eat the weekday, the comma and the space following it
         QString sansWeekday = QString::fromLatin1(value.constData() + pos + 2);
         // must be RFC 850 date
         dt = c.toDateTime(sansWeekday, QLatin1String("dd-MMM-yy hh:mm:ss 'GMT'"));
      }
   }
#endif // QT_NO_DATESTRING

   if (dt.isValid()) {
      dt.setTimeSpec(Qt::UTC);
   }
   return dt;
}

QByteArray QNetworkHeadersPrivate::toHttpDate(const QDateTime &dt)
{
   return QLocale::c().toString(dt, QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT'"))
          .toLatin1();
}
