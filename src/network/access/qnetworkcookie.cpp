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

#include <qnetworkcookie.h>
#include <qnetworkcookie_p.h>

#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>

#include <stdlib.h>

QNetworkCookie::QNetworkCookie(const QByteArray &name, const QByteArray &value)
   : d(new QNetworkCookiePrivate)
{
   qRegisterMetaType<QNetworkCookie>();
   qRegisterMetaType<QList<QNetworkCookie> >();
   d->name = name;
   d->value = value;
}

/*!
    Creates a new QNetworkCookie object by copying the contents of \a
    other.
*/
QNetworkCookie::QNetworkCookie(const QNetworkCookie &other)
   : d(other.d)
{
}

/*!
    Destroys this QNetworkCookie object.
*/
QNetworkCookie::~QNetworkCookie()
{
   // QSharedDataPointer auto deletes
   d = 0;
}

/*!
    Copies the contents of the QNetworkCookie object \a other to this
    object.
*/
QNetworkCookie &QNetworkCookie::operator=(const QNetworkCookie &other)
{
   d = other.d;
   return *this;
}

/*!
    \fn bool QNetworkCookie::operator!=(const QNetworkCookie &other) const

    Returns true if this cookie is not equal to \a other.

    \sa operator==()
*/

/*!
    Returns true if this cookie is equal to \a other. This function
    only returns true if all fields of the cookie are the same.

    However, in some contexts, two cookies of the same name could be
    considered equal.

    \sa operator!=()
*/
bool QNetworkCookie::operator==(const QNetworkCookie &other) const
{
   if (d == other.d) {
      return true;
   }
   return d->name == other.d->name &&
          d->value == other.d->value &&
          d->expirationDate.toUTC() == other.d->expirationDate.toUTC() &&
          d->domain == other.d->domain &&
          d->path == other.d->path &&
          d->secure == other.d->secure &&
          d->comment == other.d->comment;
}

bool QNetworkCookie::hasSameIdentifier(const QNetworkCookie &other) const
{
   return d->name == other.d->name && d->domain == other.d->domain && d->path == other.d->path;
}

bool QNetworkCookie::isSecure() const
{
   return d->secure;
}

void QNetworkCookie::setSecure(bool enable)
{
   d->secure = enable;
}

bool QNetworkCookie::isHttpOnly() const
{
   return d->httpOnly;
}

/*!
    \since 4.5

    Sets this cookie's "HttpOnly" flag to \a enable.
*/
void QNetworkCookie::setHttpOnly(bool enable)
{
   d->httpOnly = enable;
}

/*!
    Returns true if this cookie is a session cookie. A session cookie
    is a cookie which has no expiration date, which means it should be
    discarded when the application's concept of session is over
    (usually, when the application exits).

    \sa expirationDate(), setExpirationDate()
*/
bool QNetworkCookie::isSessionCookie() const
{
   return !d->expirationDate.isValid();
}

/*!
    Returns the expiration date for this cookie. If this cookie is a
    session cookie, the QDateTime returned will not be valid. If the
    date is in the past, this cookie has already expired and should
    not be sent again back to a remote server.

    The expiration date corresponds to the parameters of the "expires"
    entry in the cookie string.

    \sa isSessionCookie(), setExpirationDate()
*/
QDateTime QNetworkCookie::expirationDate() const
{
   return d->expirationDate;
}

/*!
    Sets the expiration date of this cookie to \a date. Setting an
    invalid expiration date to this cookie will mean it's a session
    cookie.

    \sa isSessionCookie(), expirationDate()
*/
void QNetworkCookie::setExpirationDate(const QDateTime &date)
{
   d->expirationDate = date;
}

/*!
    Returns the domain this cookie is associated with. This
    corresponds to the "domain" field of the cookie string.

    Note that the domain here may start with a dot, which is not a
    valid hostname. However, it means this cookie matches all
    hostnames ending with that domain name.

    \sa setDomain()
*/
QString QNetworkCookie::domain() const
{
   return d->domain;
}

/*!
    Sets the domain associated with this cookie to be \a domain.

    \sa domain()
*/
void QNetworkCookie::setDomain(const QString &domain)
{
   d->domain = domain;
}

/*!
    Returns the path associated with this cookie. This corresponds to
    the "path" field of the cookie string.

    \sa setPath()
*/
QString QNetworkCookie::path() const
{
   return d->path;
}

/*!
    Sets the path associated with this cookie to be \a path.

    \sa path()
*/
void QNetworkCookie::setPath(const QString &path)
{
   d->path = path;
}

/*!
    Returns the name of this cookie. The only mandatory field of a
    cookie is its name, without which it is not considered valid.

    \sa setName(), value()
*/
QByteArray QNetworkCookie::name() const
{
   return d->name;
}

/*!
    Sets the name of this cookie to be \a cookieName. Note that
    setting a cookie name to an empty QByteArray will make this cookie
    invalid.

    \sa name(), value()
*/
void QNetworkCookie::setName(const QByteArray &cookieName)
{
   d->name = cookieName;
}

/*!
    Returns this cookies value, as specified in the cookie
    string. Note that a cookie is still valid if its value is empty.

    Cookie name-value pairs are considered opaque to the application:
    that is, their values don't mean anything.

    \sa setValue(), name()
*/
QByteArray QNetworkCookie::value() const
{
   return d->value;
}

/*!
    Sets the value of this cookie to be \a value.

    \sa value(), name()
*/
void QNetworkCookie::setValue(const QByteArray &value)
{
   d->value = value;
}

// ### move this to qnetworkcookie_p.h and share with qnetworkaccesshttpbackend
static QPair<QByteArray, QByteArray> nextField(const QByteArray &text, int &position, bool isNameValue)
{
   // format is one of:
   //    (1)  token
   //    (2)  token = token
   //    (3)  token = quoted-string

   const int length = text.length();
   position = nextNonWhitespace(text, position);

   int semiColonPosition = text.indexOf(';', position);
   if (semiColonPosition < 0) {
      semiColonPosition = length;   //no ';' means take everything to end of string
   }
   int equalsPosition = text.indexOf('=', position);
   if (equalsPosition < 0 || equalsPosition > semiColonPosition) {
      if (isNameValue) {
         return qMakePair(QByteArray(), QByteArray());   //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
      }
      equalsPosition = semiColonPosition; //no '=' means there is an attribute-name but no attribute-value
   }

   QByteArray first = text.mid(position, equalsPosition - position).trimmed();
   QByteArray second;
   int secondLength = semiColonPosition - equalsPosition - 1;

   if (secondLength > 0)  {
      second = text.mid(equalsPosition + 1, secondLength).trimmed();
   }

   position = semiColonPosition;

   return qMakePair(first, second);
}

/*!
    \enum QNetworkCookie::RawForm

    This enum is used with the toRawForm() function to declare which
    form of a cookie shall be returned.

    \value NameAndValueOnly     makes toRawForm() return only the
        "NAME=VALUE" part of the cookie, as suitable for sending back
        to a server in a client request's "Cookie:" header. Multiple
        cookies are separated by a semi-colon in the "Cookie:" header
        field.

    \value Full                 makes toRawForm() return the full
        cookie contents, as suitable for sending to a client in a
        server's "Set-Cookie:" header.

    Note that only the Full form of the cookie can be parsed back into
    its original contents.

    \sa toRawForm(), parseCookies()
*/

/*!
    Returns the raw form of this QNetworkCookie. The QByteArray
    returned by this function is suitable for an HTTP header, either
    in a server response (the Set-Cookie header) or the client request
    (the Cookie header). You can choose from one of two formats, using
    \a form.

    \sa parseCookies()
*/
QByteArray QNetworkCookie::toRawForm(RawForm form) const
{
   QByteArray result;
   if (d->name.isEmpty()) {
      return result;   // not a valid cookie
   }

   result = d->name;
   result += '=';

   result += d->value;

   if (form == Full) {
      // same as above, but encoding everything back
      if (isSecure()) {
         result += "; secure";
      }

      if (isHttpOnly()) {
         result += "; HttpOnly";
      }

      if (!isSessionCookie()) {
         result += "; expires=";
         result += QLocale::c().toString(d->expirationDate.toUTC(),
                                         QLatin1String("ddd, dd-MMM-yyyy hh:mm:ss 'GMT")).toLatin1();
      }
      if (!d->domain.isEmpty()) {
         result += "; domain=";

         if (d->domain.startsWith(QLatin1Char('.'))) {
            result += '.';
            result += QUrl::toAce(d->domain.mid(1));
         } else {
            QHostAddress hostAddr(d->domain);
            if (hostAddr.protocol() == QAbstractSocket::IPv6Protocol) {
               result += '[';
               result += d->domain.toUtf8();
               result += ']';
            } else {
               result += QUrl::toAce(d->domain);
            }
         }
      }

      if (!d->path.isEmpty()) {
         result += "; path=";
         result += d->path.toUtf8();
      }
   }
   return result;
}

static const char zones[] =
   "pst\0" // -8
   "pdt\0"
   "mst\0" // -7
   "mdt\0"
   "cst\0" // -6
   "cdt\0"
   "est\0" // -5
   "edt\0"
   "ast\0" // -4
   "nst\0" // -3
   "gmt\0" // 0
   "utc\0"
   "bst\0"
   "met\0" // 1
   "eet\0" // 2
   "jst\0" // 9
   "\0";
static const int zoneOffsets[] = {-8, -8, -7, -7, -6, -6, -5, -5, -4, -3, 0, 0, 0, 1, 2, 9 };

static const char months[] =
   "jan\0"
   "feb\0"
   "mar\0"
   "apr\0"
   "may\0"
   "jun\0"
   "jul\0"
   "aug\0"
   "sep\0"
   "oct\0"
   "nov\0"
   "dec\0"
   "\0";

static inline bool isNumber(char s)
{
   return s >= '0' && s <= '9';
}

static inline bool isTerminator(char c)
{
   return c == '\n' || c == '\r';
}

static inline bool isValueSeparator(char c)
{
   return isTerminator(c) || c == ';';
}

static inline bool isWhitespace(char c)
{
   return c == ' '  || c == '\t';
}

static bool checkStaticArray(int &val, const QByteArray &dateString, int at, const char *array, int size)
{
   if (dateString[at] < 'a' || dateString[at] > 'z') {
      return false;
   }

   if (val == -1 && dateString.length() >= at + 3) {
      int j = 0;
      int i = 0;

      while (i <= size) {
         const char *str = array + i;
         if (str[0] == dateString[at]
               && str[1] == dateString[at + 1]
               && str[2] == dateString[at + 2]) {
            val = j;
            return true;
         }

         i += strlen(str) + 1;
         ++j;
      }
   }
   return false;
}

//#define PARSEDATESTRINGDEBUG

#define ADAY   1
#define AMONTH 2
#define AYEAR  4

/*
    Parse all the date formats that Firefox can.

    The official format is:
    expires=ddd(d)?, dd-MMM-yyyy hh:mm:ss GMT

    But browsers have been supporting a very wide range of date
    strings. To work on many sites we need to support more then
    just the official date format.

    For reference see Firefox's PR_ParseTimeStringToExplodedTime in
    prtime.c. The Firefox date parser is coded in a very complex way
    and is slightly over ~700 lines long.  While this implementation
    will be slightly slower for the non standard dates it is smaller,
    more readable, and maintainable.

    Or in their own words:
        "} // else what the hell is this."
*/
static QDateTime parseDateString(const QByteArray &dateString)
{
   QTime time;
   // placeholders for values when we are not sure it is a year, month or day
   int unknown[3] = { -1, -1, -1};
   int month = -1;
   int day = -1;
   int year = -1;
   int zoneOffset = -1;

   // hour:minute:second.ms pm
   QRegExp timeRx(QLatin1String("(\\d{1,2}):(\\d{1,2})(:(\\d{1,2})|)(\\.(\\d{1,3})|)((\\s{0,}(am|pm))|)"));

   int at = 0;
   while (at < dateString.length()) {
#ifdef PARSEDATESTRINGDEBUG
      qDebug() << dateString.mid(at);
#endif
      bool isNum = isNumber(dateString[at]);

      // Month
      if (!isNum
            && checkStaticArray(month, dateString, at, months, sizeof(months) - 1)) {
         ++month;
#ifdef PARSEDATESTRINGDEBUG
         qDebug() << "Month:" << month;
#endif
         at += 3;
         continue;
      }
      // Zone
      if (!isNum
            && zoneOffset == -1
            && checkStaticArray(zoneOffset, dateString, at, zones, sizeof(zones) - 1)) {
         int sign = (at >= 0 && dateString[at - 1] == '-') ? -1 : 1;
         zoneOffset = sign * zoneOffsets[zoneOffset] * 60 * 60;
#ifdef PARSEDATESTRINGDEBUG
         qDebug() << "Zone:" << month;
#endif
         at += 3;
         continue;
      }
      // Zone offset
      if (!isNum
            && (zoneOffset == -1 || zoneOffset == 0) // Can only go after gmt
            && (dateString[at] == '+' || dateString[at] == '-')
            && (at == 0
                || isWhitespace(dateString[at - 1])
                || dateString[at - 1] == ','
                || (at >= 3
                    && (dateString[at - 3] == 'g')
                    && (dateString[at - 2] == 'm')
                    && (dateString[at - 1] == 't')))) {

         int end = 1;
         while (end < 5 && dateString.length() > at + end
                && dateString[at + end] >= '0' && dateString[at + end] <= '9') {
            ++end;
         }
         int minutes = 0;
         int hours = 0;
         switch (end - 1) {
            case 4:
               minutes = atoi(dateString.mid(at + 3, 2).constData());
            // fall through
            case 2:
               hours = atoi(dateString.mid(at + 1, 2).constData());
               break;
            case 1:
               hours = atoi(dateString.mid(at + 1, 1).constData());
               break;
            default:
               at += end;
               continue;
         }
         if (end != 1) {
            int sign = dateString[at] == '-' ? -1 : 1;
            zoneOffset = sign * ((minutes * 60) + (hours * 60 * 60));
#ifdef PARSEDATESTRINGDEBUG
            qDebug() << "Zone offset:" << zoneOffset << hours << minutes;
#endif
            at += end;
            continue;
         }
      }

      // Time
      if (isNum && time.isNull()
            && dateString.length() >= at + 3
            && (dateString[at + 2] == ':' || dateString[at + 1] == ':')) {
         // While the date can be found all over the string the format
         // for the time is set and a nice regexp can be used.
         int pos = timeRx.indexIn(QLatin1String(dateString), at);
         if (pos != -1) {
            QStringList list = timeRx.capturedTexts();
            int h = atoi(list.at(1).toLatin1().constData());
            int m = atoi(list.at(2).toLatin1().constData());
            int s = atoi(list.at(4).toLatin1().constData());
            int ms = atoi(list.at(6).toLatin1().constData());
            if (h < 12 && !list.at(9).isEmpty())
               if (list.at(9) == QLatin1String("pm")) {
                  h += 12;
               }
            time = QTime(h, m, s, ms);
#ifdef PARSEDATESTRINGDEBUG
            qDebug() << "Time:" << list << timeRx.matchedLength();
#endif
            at += timeRx.matchedLength();
            continue;
         }
      }

      // 4 digit Year
      if (isNum
            && year == -1
            && dateString.length() > at + 3) {
         if (isNumber(dateString[at + 1])
               && isNumber(dateString[at + 2])
               && isNumber(dateString[at + 3])) {
            year = atoi(dateString.mid(at, 4).constData());
            at += 4;
#ifdef PARSEDATESTRINGDEBUG
            qDebug() << "Year:" << year;
#endif
            continue;
         }
      }

      // a one or two digit number
      // Could be month, day or year
      if (isNum) {
         int length = 1;
         if (dateString.length() > at + 1 && isNumber(dateString[at + 1])) {
            ++length;
         }

         int x = atoi(dateString.mid(at, length).constData());
         if (year == -1 && (x > 31 || x == 0)) {
            year = x;

         } else {
            if (unknown[0] == -1) {
               unknown[0] = x;
            } else if (unknown[1] == -1) {
               unknown[1] = x;
            } else if (unknown[2] == -1) {
               unknown[2] = x;
            }
         }
         at += length;
#ifdef PARSEDATESTRINGDEBUG
         qDebug() << "Saving" << x;
#endif
         continue;
      }

      // Unknown character, typically a weekday such as 'Mon'
      ++at;
   }

   // Once we are done parsing the string take the digits in unknown
   // and determine which is the unknown year/month/day

   int couldBe[3] = { 0, 0, 0 };
   int unknownCount = 3;
   for (int i = 0; i < unknownCount; ++i) {
      if (unknown[i] == -1) {
         couldBe[i] = ADAY | AYEAR | AMONTH;
         unknownCount = i;
         continue;
      }

      if (unknown[i] >= 1) {
         couldBe[i] = ADAY;
      }

      if (month == -1 && unknown[i] >= 1 && unknown[i] <= 12) {
         couldBe[i] |= AMONTH;
      }

      if (year == -1) {
         couldBe[i] |= AYEAR;
      }
   }

   // For any possible day make sure one of the values that could be a month
   // can contain that day.
   // For any possible month make sure one of the values that can be a
   // day that month can have.
   // Example: 31 11 06
   // 31 can't be a day because 11 and 6 don't have 31 days
   for (int i = 0; i < unknownCount; ++i) {
      int currentValue = unknown[i];
      bool findMatchingMonth = couldBe[i] & ADAY && currentValue >= 29;
      bool findMatchingDay = couldBe[i] & AMONTH;

      if (!findMatchingMonth || !findMatchingDay) {
         continue;
      }

      for (int j = 0; j < 3; ++j) {
         if (j == i) {
            continue;
         }
         for (int k = 0; k < 2; ++k) {
            if (k == 0 && !(findMatchingMonth && (couldBe[j] & AMONTH))) {
               continue;
            } else if (k == 1 && !(findMatchingDay && (couldBe[j] & ADAY))) {
               continue;
            }

            int m = currentValue;
            int d = unknown[j];
            if (k == 0) {
               qSwap(m, d);
            }

            if (m == -1) {
               m = month;
            }

            bool found = true;
            switch (m) {
               case 2:
                  // When we get 29 and the year ends up having only 28
                  // See date.isValid below
                  // Example: 29 23 Feb
                  if (d <= 29) {
                     found = false;
                  }
                  break;

               case 4:
               case 6:
               case 9:
               case 11:
                  if (d <= 30) {
                     found = false;
                  }
                  break;
               default:
                  if (d > 0 && d <= 31) {
                     found = false;
                  }
            }

            if (k == 0) {
               findMatchingMonth = found;
            } else if (k == 1) {
               findMatchingDay = found;
            }
         }
      }
      if (findMatchingMonth) {
         couldBe[i] &= ~ADAY;
      }
      if (findMatchingDay) {
         couldBe[i] &= ~AMONTH;
      }
   }

   // First set the year/month/day that have been deduced
   // and reduce the set as we go along to deduce more
   for (int i = 0; i < unknownCount; ++i) {
      int unset = 0;
      for (int j = 0; j < 3; ++j) {
         if (couldBe[j] == ADAY && day == -1) {
            day = unknown[j];
            unset |= ADAY;
         } else if (couldBe[j] == AMONTH && month == -1) {
            month = unknown[j];
            unset |= AMONTH;
         } else if (couldBe[j] == AYEAR && year == -1) {
            year = unknown[j];
            unset |= AYEAR;
         } else {
            // common case
            break;
         }
         couldBe[j] &= ~unset;
      }
   }

   // Now fallback to a standardized order to fill in the rest with
   for (int i = 0; i < unknownCount; ++i) {
      if (couldBe[i] & AMONTH && month == -1) {
         month = unknown[i];
      } else if (couldBe[i] & ADAY && day == -1) {
         day = unknown[i];
      } else if (couldBe[i] & AYEAR && year == -1) {
         year = unknown[i];
      }
   }
#ifdef PARSEDATESTRINGDEBUG
   qDebug() << "Final set" << year << month << day;
#endif

   if (year == -1 || month == -1 || day == -1) {
#ifdef PARSEDATESTRINGDEBUG
      qDebug() << "Parser failure" << year << month << day;
#endif
      return QDateTime();
   }

   // Y2k behavior
   int y2k = 0;
   if (year < 70) {
      y2k = 2000;
   } else if (year < 100) {
      y2k = 1900;
   }

   QDate date(year + y2k, month, day);

   // When we were given a bad cookie that when parsed
   // set the day to 29 and the year to one that doesn't
   // have the 29th of Feb rather then adding the extra
   // complicated checking earlier just swap here.
   // Example: 29 23 Feb
   if (!date.isValid()) {
      date = QDate(day + y2k, month, year);
   }

   QDateTime dateTime(date, time, Qt::UTC);

   if (zoneOffset != -1) {
      dateTime = dateTime.addSecs(zoneOffset);
   }
   if (!dateTime.isValid()) {
      return QDateTime();
   }
   return dateTime;
}

/*!
    Parses the cookie string \a cookieString as received from a server
    response in the "Set-Cookie:" header. If there's a parsing error,
    this function returns an empty list.

    Since the HTTP header can set more than one cookie at the same
    time, this function returns a QList<QNetworkCookie>, one for each
    cookie that is parsed.

    \sa toRawForm()
*/
QList<QNetworkCookie> QNetworkCookie::parseCookies(const QByteArray &cookieString)
{
   // cookieString can be a number of set-cookie header strings joined together
   // by \n, parse each line separately.
   QList<QNetworkCookie> cookies;
   QList<QByteArray> list = cookieString.split('\n');
   for (int a = 0; a < list.size(); a++) {
      cookies += QNetworkCookiePrivate::parseSetCookieHeaderLine(list.at(a));
   }
   return cookies;
}

QList<QNetworkCookie> QNetworkCookiePrivate::parseSetCookieHeaderLine(const QByteArray &cookieString)
{
   // According to http://wp.netscape.com/newsref/std/cookie_spec.html,<
   // the Set-Cookie response header is of the format:
   //
   //   Set-Cookie: NAME=VALUE; expires=DATE; path=PATH; domain=DOMAIN_NAME; secure
   //
   // where only the NAME=VALUE part is mandatory
   //
   // We do not support RFC 2965 Set-Cookie2-style cookies

   QList<QNetworkCookie> result;
   const QDateTime now = QDateTime::currentDateTime().toUTC();

   int position = 0;
   const int length = cookieString.length();
   while (position < length) {
      QNetworkCookie cookie;

      // The first part is always the "NAME=VALUE" part
      QPair<QByteArray, QByteArray> field = nextField(cookieString, position, true);

      if (field.first.isEmpty()) {
         // parsing error
         break;
      }

      cookie.setName(field.first);
      cookie.setValue(field.second);

      position = nextNonWhitespace(cookieString, position);
      while (position < length) {
         switch (cookieString.at(position++)) {
            case ';':
               // new field in the cookie
               field = nextField(cookieString, position, false);
               field.first = field.first.toLower(); // everything but the NAME=VALUE is case-insensitive

               if (field.first == "expires") {
                  position -= field.second.length();
                  int end;

                  for (end = position; end < length; ++end)
                     if (isValueSeparator(cookieString.at(end))) {
                        break;
                     }

                  QByteArray dateString = cookieString.mid(position, end - position).trimmed();
                  position = end;
                  QDateTime dt = parseDateString(dateString.toLower());

                  if (dt.isValid()) {
                     cookie.setExpirationDate(dt);
                  }

               } else if (field.first == "domain") {
                  QByteArray rawDomain = field.second;
                  if (!rawDomain.isEmpty()) {
                     QString maybeLeadingDot;
                     if (rawDomain.startsWith('.')) {
                        maybeLeadingDot = QLatin1Char('.');
                        rawDomain = rawDomain.mid(1);
                     }
                     QString normalizedDomain = QUrl::fromAce(QUrl::toAce(QString::fromUtf8(rawDomain)));
                     if (!normalizedDomain.isEmpty()) {
                        cookie.setDomain(maybeLeadingDot + normalizedDomain);
                     } else {
                        return result;
                     }
                  }


               } else if (field.first == "max-age") {
                  bool ok = false;
                  int secs = field.second.toInt(&ok);

                  if (ok) {
                     if (secs <= 0) {
                        cookie.setExpirationDate(QDateTime::fromTime_t(0));
                     } else {
                        cookie.setExpirationDate(now.addSecs(secs));
                     }
                  }
               } else if (field.first == "path") {
                  if (field.second.startsWith('/')) {
                     cookie.setPath(QString::fromUtf8(field.second));
                  } else {
                     cookie.setPath(QString());
                  }

               } else if (field.first == "secure") {
                  cookie.setSecure(true);

               } else if (field.first == "httponly") {
                  cookie.setHttpOnly(true);

               } else {
                  // got an unknown field in the cookie
                  // what do we do?
               }

               position = nextNonWhitespace(cookieString, position);
         }
      }

      if (!cookie.name().isEmpty()) {
         result += cookie;
      }
   }

   return result;
}

void QNetworkCookie::normalize(const QUrl &url)
{
   if (d->path.isEmpty()) {
      QString pathAndFileName = url.path();
      QString defaultPath = pathAndFileName.left(pathAndFileName.lastIndexOf(QLatin1Char('/')) + 1);
      if (defaultPath.isEmpty()) {
         defaultPath = QLatin1Char('/');
      }
      d->path = defaultPath;
   }

   if (d->domain.isEmpty()) {
      d->domain = url.host();
   } else {
      QHostAddress hostAddress(d->domain);
      if (hostAddress.protocol() != QAbstractSocket::IPv4Protocol
            && hostAddress.protocol() != QAbstractSocket::IPv6Protocol
            && !d->domain.startsWith(QLatin1Char('.'))) {
         d->domain.prepend(QLatin1Char('.'));
      }
   }
}

QDebug operator<<(QDebug s, const QNetworkCookie &cookie)
{
   // QDebugStateSaver saver(s);
   // s.resetFormat().nospace();

   s.nospace() << "QNetworkCookie(" << cookie.toRawForm(QNetworkCookie::Full) << ')';

   return s.space();
}


