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

#include <qhttpheader_p.h>

#include <qplatformdefs.h>
#include <qnetworkproxy.h>
#include <qauthenticator.h>
#include <qauthenticator_p.h>
#include <qtcpsocket.h>
#include <qsslsocket.h>

#include <qbuffer.h>
#include <qcoreevent.h>
#include <qtextstream.h>
#include <qmap.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qringbuffer_p.h>
#include <qurl.h>

#include <qdebug.h>

class QHttpHeaderPrivate
{
   Q_DECLARE_PUBLIC(QHttpHeader)

 public:
   inline virtual ~QHttpHeaderPrivate() {}

   QList<QPair<QString, QString> > values;
   bool valid;
   QHttpHeader *q_ptr;
};

QHttpHeader::QHttpHeader()
   : d_ptr(new QHttpHeaderPrivate)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = true;
}

/*!
        Constructs a copy of \a header.
*/
QHttpHeader::QHttpHeader(const QHttpHeader &header)
   : d_ptr(new QHttpHeaderPrivate)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = header.d_func()->valid;
   d->values = header.d_func()->values;
}

QHttpHeader::QHttpHeader(const QString &str)
   : d_ptr(new QHttpHeaderPrivate)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = true;
   parse(str);
}

/*! \internal
 */
QHttpHeader::QHttpHeader(QHttpHeaderPrivate &dd, const QString &str)
   : d_ptr(&dd)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = true;
   if (!str.isEmpty()) {
      parse(str);
   }
}

/*! \internal
 */
QHttpHeader::QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header)
   : d_ptr(&dd)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = header.d_func()->valid;
   d->values = header.d_func()->values;
}
/*!
    Destructor.
*/
QHttpHeader::~QHttpHeader()
{
}

/*!
    Assigns \a h and returns a reference to this http header.
*/
QHttpHeader &QHttpHeader::operator=(const QHttpHeader &h)
{
   Q_D(QHttpHeader);
   d->values = h.d_func()->values;
   d->valid = h.d_func()->valid;
   return *this;
}

/*!
    Returns true if the HTTP header is valid; otherwise returns false.

    A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
   Q_D(const QHttpHeader);
   return d->valid;
}

/*! \internal
    Parses the HTTP header string \a str for header fields and adds
    the keys/values it finds. If the string is not parsed successfully
    the QHttpHeader becomes \link isValid() invalid\endlink.

    Returns true if \a str was successfully parsed; otherwise returns false.

    \sa toString()
*/
bool QHttpHeader::parse(const QString &str)
{
   Q_D(QHttpHeader);
   QStringList lst;
   int pos = str.indexOf(QLatin1Char('\n'));

   if (pos > 0 && str.at(pos - 1) == QLatin1Char('\r')) {
      lst = str.trimmed().split(QLatin1String("\r\n"));
   } else {
      lst = str.trimmed().split(QLatin1String("\n"));
   }

   lst.removeAll(QString()); // No empties

   if (lst.isEmpty()) {
      return true;
   }

   QStringList lines;
   QStringList::Iterator it = lst.begin();

   for (; it != lst.end(); ++it) {
      if (!(*it).isEmpty()) {
         if ((*it)[0].isSpace()) {
            if (!lines.isEmpty()) {
               lines.last() += QLatin1Char(' ');
               lines.last() += (*it).trimmed();
            }
         } else {
            lines.append((*it));
         }
      }
   }

   int number = 0;
   it = lines.begin();
   for (; it != lines.end(); ++it) {
      if (!parseLine(*it, number++)) {
         d->valid = false;
         return false;
      }
   }
   return true;
}

/*! \internal
*/
void QHttpHeader::setValid(bool v)
{
   Q_D(QHttpHeader);
   d->valid = v;
}

QString QHttpHeader::value(const QString &key) const
{
   Q_D(const QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      if ((*it).first.toLower() == lowercaseKey) {
         return (*it).second;
      }
      ++it;
   }
   return QString();
}

QStringList QHttpHeader::allValues(const QString &key) const
{
   Q_D(const QHttpHeader);
   QString lowercaseKey = key.toLower();
   QStringList valueList;
   QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      if ((*it).first.toLower() == lowercaseKey) {
         valueList.append((*it).second);
      }
      ++it;
   }
   return valueList;
}

QStringList QHttpHeader::keys() const
{
   Q_D(const QHttpHeader);
   QStringList keyList;
   QSet<QString> seenKeys;
   QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      const QString &key = (*it).first;
      QString lowercaseKey = key.toLower();
      if (!seenKeys.contains(lowercaseKey)) {
         keyList.append(key);
         seenKeys.insert(lowercaseKey);
      }
      ++it;
   }
   return keyList;
}

bool QHttpHeader::hasKey(const QString &key) const
{
   Q_D(const QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      if ((*it).first.toLower() == lowercaseKey) {
         return true;
      }
      ++it;
   }
   return false;
}

void QHttpHeader::setValue(const QString &key, const QString &value)
{
   Q_D(QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::Iterator it = d->values.begin();
   while (it != d->values.end()) {
      if ((*it).first.toLower() == lowercaseKey) {
         (*it).second = value;
         return;
      }
      ++it;
   }
   // not found so add
   addValue(key, value);
}

/*!
    Sets the header entries to be the list of key value pairs in \a values.
*/
void QHttpHeader::setValues(const QList<QPair<QString, QString> > &values)
{
   Q_D(QHttpHeader);
   d->values = values;
}

/*!
    Adds a new entry with the \a key and \a value.
*/
void QHttpHeader::addValue(const QString &key, const QString &value)
{
   Q_D(QHttpHeader);
   d->values.append(qMakePair(key, value));
}

/*!
    Returns all the entries in the header.
*/
QList<QPair<QString, QString> > QHttpHeader::values() const
{
   Q_D(const QHttpHeader);
   return d->values;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue(const QString &key)
{
   Q_D(QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::Iterator it = d->values.begin();
   while (it != d->values.end()) {
      if ((*it).first.toLower() == lowercaseKey) {
         d->values.erase(it);
         return;
      }
      ++it;
   }
}

/*!
    Removes all the entries with the key \a key from the HTTP header.
*/
void QHttpHeader::removeAllValues(const QString &key)
{
   Q_D(QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::Iterator it = d->values.begin();
   while (it != d->values.end()) {
      if ((*it).first.toLower() == lowercaseKey) {
         it = d->values.erase(it);
         continue;
      }
      ++it;
   }
}

/*! \internal
    Parses the single HTTP header line \a line which has the format
    key, colon, space, value, and adds key/value to the headers. The
    linenumber is \a number. Returns true if the line was successfully
    parsed and the key/value added; otherwise returns false.

    \sa parse()
*/
bool QHttpHeader::parseLine(const QString &line, int)
{
   int i = line.indexOf(QLatin1Char(':'));
   if (i == -1) {
      return false;
   }

   addValue(line.left(i).trimmed(), line.mid(i + 1).trimmed());

   return true;
}

QString QHttpHeader::toString() const
{
   Q_D(const QHttpHeader);
   if (!isValid()) {
      return QLatin1String("");
   }

   QString ret = QLatin1String("");

   QList<QPair<QString, QString> >::ConstIterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      ret += (*it).first + QLatin1String(": ") + (*it).second + QLatin1String("\r\n");
      ++it;
   }
   return ret;
}

/*!
    Returns true if the header has an entry for the special HTTP
    header field \c content-length; otherwise returns false.

    \sa contentLength() setContentLength()
*/
bool QHttpHeader::hasContentLength() const
{
   return hasKey(QLatin1String("content-length"));
}

/*!
    Returns the value of the special HTTP header field \c
    content-length.

    \sa setContentLength() hasContentLength()
*/
qint64 QHttpHeader::contentLength() const
{
   return value(QLatin1String("content-length")).toUInt();
}

/*!
    Sets the value of the special HTTP header field \c content-length
    to \a len.

    \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength(qint64 len)
{
   setValue(QLatin1String("content-length"), QString::number(len));
}

/*!
    Returns true if the header has an entry for the special HTTP
    header field \c content-type; otherwise returns false.

    \sa contentType() setContentType()
*/
bool QHttpHeader::hasContentType() const
{
   return hasKey(QLatin1String("content-type"));
}

/*!
    Returns the value of the special HTTP header field \c content-type.

    \sa setContentType() hasContentType()
*/
QString QHttpHeader::contentType() const
{
   QString type = value(QLatin1String("content-type"));
   if (type.isEmpty()) {
      return QString();
   }

   int pos = type.indexOf(QLatin1Char(';'));
   if (pos == -1) {
      return type;
   }

   return type.left(pos).trimmed();
}

/*!
    Sets the value of the special HTTP header field \c content-type to
    \a type.

    \sa contentType() hasContentType()
*/
void QHttpHeader::setContentType(const QString &type)
{
   setValue(QLatin1String("content-type"), type);
}

// * *    Request Header

class QHttpRequestHeaderPrivate : public QHttpHeaderPrivate
{
   Q_DECLARE_PUBLIC(QHttpRequestHeader)
 public:
   QString m;
   QString p;
   int majVer;
   int minVer;
};

QHttpRequestHeader::QHttpRequestHeader()
   : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
   setValid(false);
}

QHttpRequestHeader::QHttpRequestHeader(const QString &method, const QString &path, int majorVer, int minorVer)
   : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
   Q_D(QHttpRequestHeader);
   d->m = method;
   d->p = path;
   d->majVer = majorVer;
   d->minVer = minorVer;
}

QHttpRequestHeader::QHttpRequestHeader(const QHttpRequestHeader &header)
   : QHttpHeader(*new QHttpRequestHeaderPrivate, header)
{
   Q_D(QHttpRequestHeader);
   d->m = header.d_func()->m;
   d->p = header.d_func()->p;
   d->majVer = header.d_func()->majVer;
   d->minVer = header.d_func()->minVer;
}

/*!
    Copies the content of \a header into this QHttpRequestHeader
*/
QHttpRequestHeader &QHttpRequestHeader::operator=(const QHttpRequestHeader &header)
{
   Q_D(QHttpRequestHeader);
   QHttpHeader::operator=(header);
   d->m = header.d_func()->m;
   d->p = header.d_func()->p;
   d->majVer = header.d_func()->majVer;
   d->minVer = header.d_func()->minVer;
   return *this;
}

/*!
    Constructs a HTTP request header from the string \a str. The \a
    str should consist of one or more "\r\n" delimited lines; the first line
    should be the request-line (format: method, space, request-URI, space
    HTTP-version); each of the remaining lines should have the format key,
    colon, space, value.
*/
QHttpRequestHeader::QHttpRequestHeader(const QString &str)
   : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
   parse(str);
}

/*!
    This function sets the request method to \a method, the
    request-URI to \a path and the protocol-version to \a majorVer and
    \a minorVer. The \a path argument must be properly encoded for an
    HTTP request.

    \sa method() path() majorVersion() minorVersion()
*/
void QHttpRequestHeader::setRequest(const QString &method, const QString &path, int majorVer, int minorVer)
{
   Q_D(QHttpRequestHeader);
   setValid(true);
   d->m = method;
   d->p = path;
   d->majVer = majorVer;
   d->minVer = minorVer;
}

/*!
    Returns the method of the HTTP request header.

    \sa path() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::method() const
{
   Q_D(const QHttpRequestHeader);
   return d->m;
}

/*!
    Returns the request-URI of the HTTP request header.

    \sa method() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::path() const
{
   Q_D(const QHttpRequestHeader);
   return d->p;
}

/*!
    Returns the major protocol-version of the HTTP request header.

    \sa minorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::majorVersion() const
{
   Q_D(const QHttpRequestHeader);
   return d->majVer;
}

/*!
    Returns the minor protocol-version of the HTTP request header.

    \sa majorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::minorVersion() const
{
   Q_D(const QHttpRequestHeader);
   return d->minVer;
}

/*! \internal
*/
bool QHttpRequestHeader::parseLine(const QString &line, int number)
{
   Q_D(QHttpRequestHeader);
   if (number != 0) {
      return QHttpHeader::parseLine(line, number);
   }

   QStringList lst = line.simplified().split(QLatin1String(" "));
   if (lst.count() > 0) {
      d->m = lst[0];
      if (lst.count() > 1) {
         d->p = lst[1];
         if (lst.count() > 2) {
            QString v = lst[2];
            if (v.length() >= 8 && v.left(5) == QLatin1String("HTTP/") &&
                  v[5].isDigit() && v[6] == QLatin1Char('.') && v[7].isDigit()) {
               d->majVer = v[5].toLatin1() - '0';
               d->minVer = v[7].toLatin1() - '0';
               return true;
            }
         }
      }
   }

   return false;
}

QString QHttpRequestHeader::toString() const
{
   Q_D(const QHttpRequestHeader);
   QString first(QLatin1String("%1 %2"));
   QString last(QLatin1String(" HTTP/%3.%4\r\n%5\r\n"));
   return first.arg(d->m).arg(d->p) +
          last.arg(d->majVer).arg(d->minVer).arg(QHttpHeader::toString());
}


// * *    Response Header

class QHttpResponseHeaderPrivate : public QHttpHeaderPrivate
{
   Q_DECLARE_PUBLIC(QHttpResponseHeader)

 public:
   int statCode;
   QString reasonPhr;
   int majVer;
   int minVer;
};

QHttpResponseHeader::QHttpResponseHeader()
   : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
   setValid(false);
}

/*!
    Constructs a copy of \a header.
*/
QHttpResponseHeader::QHttpResponseHeader(const QHttpResponseHeader &header)
   : QHttpHeader(*new QHttpResponseHeaderPrivate, header)
{
   Q_D(QHttpResponseHeader);
   d->statCode = header.d_func()->statCode;
   d->reasonPhr = header.d_func()->reasonPhr;
   d->majVer = header.d_func()->majVer;
   d->minVer = header.d_func()->minVer;
}

/*!
    Copies the contents of \a header into this QHttpResponseHeader.
*/
QHttpResponseHeader &QHttpResponseHeader::operator=(const QHttpResponseHeader &header)
{
   Q_D(QHttpResponseHeader);
   QHttpHeader::operator=(header);
   d->statCode = header.d_func()->statCode;
   d->reasonPhr = header.d_func()->reasonPhr;
   d->majVer = header.d_func()->majVer;
   d->minVer = header.d_func()->minVer;
   return *this;
}

QHttpResponseHeader::QHttpResponseHeader(const QString &str)
   : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
   parse(str);
}

QHttpResponseHeader::QHttpResponseHeader(int code, const QString &text, int majorVer, int minorVer)
   : QHttpHeader(*new QHttpResponseHeaderPrivate)
{
   setStatusLine(code, text, majorVer, minorVer);
}

void QHttpResponseHeader::setStatusLine(int code, const QString &text, int majorVer, int minorVer)
{
   Q_D(QHttpResponseHeader);
   setValid(true);
   d->statCode = code;
   d->reasonPhr = text;
   d->majVer = majorVer;
   d->minVer = minorVer;
}

int QHttpResponseHeader::statusCode() const
{
   Q_D(const QHttpResponseHeader);
   return d->statCode;
}

/*!
    Returns the reason phrase of the HTTP response header.

    \sa statusCode() majorVersion() minorVersion()
*/
QString QHttpResponseHeader::reasonPhrase() const
{
   Q_D(const QHttpResponseHeader);
   return d->reasonPhr;
}

/*!
    Returns the major protocol-version of the HTTP response header.

    \sa minorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::majorVersion() const
{
   Q_D(const QHttpResponseHeader);
   return d->majVer;
}

/*!
    Returns the minor protocol-version of the HTTP response header.

    \sa majorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::minorVersion() const
{
   Q_D(const QHttpResponseHeader);
   return d->minVer;
}

/*! \internal
*/
bool QHttpResponseHeader::parseLine(const QString &line, int number)
{
   Q_D(QHttpResponseHeader);
   if (number != 0) {
      return QHttpHeader::parseLine(line, number);
   }

   QString l = line.simplified();
   if (l.length() < 10) {
      return false;
   }

   if (l.left(5) == QLatin1String("HTTP/") && l[5].isDigit() && l[6] == QLatin1Char('.') &&
         l[7].isDigit() && l[8] == QLatin1Char(' ') && l[9].isDigit()) {
      d->majVer = l[5].toLatin1() - '0';
      d->minVer = l[7].toLatin1() - '0';

      int pos = l.indexOf(QLatin1Char(' '), 9);
      if (pos != -1) {
         d->reasonPhr = l.mid(pos + 1);
         d->statCode = l.mid(9, pos - 9).toInt();
      } else {
         d->statCode = l.mid(9).toInt();
         d->reasonPhr.clear();
      }
   } else {
      return false;
   }

   return true;
}

/*! \reimp
*/
QString QHttpResponseHeader::toString() const
{
   Q_D(const QHttpResponseHeader);
   QString ret(QLatin1String("HTTP/%1.%2 %3 %4\r\n%5\r\n"));
   return ret.arg(d->majVer).arg(d->minVer).arg(d->statCode).arg(d->reasonPhr).arg(QHttpHeader::toString());
}
