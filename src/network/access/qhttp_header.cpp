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

#include <qhttp_header_p.h>

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
   virtual ~QHttpHeaderPrivate()
   { }

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

QHttpHeader::QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header)
   : d_ptr(&dd)
{
   Q_D(QHttpHeader);
   d->q_ptr = this;
   d->valid = header.d_func()->valid;
   d->values = header.d_func()->values;
}

QHttpHeader::~QHttpHeader()
{
}

QHttpHeader &QHttpHeader::operator=(const QHttpHeader &h)
{
   Q_D(QHttpHeader);
   d->values = h.d_func()->values;
   d->valid = h.d_func()->valid;
   return *this;
}

bool QHttpHeader::isValid() const
{
   Q_D(const QHttpHeader);
   return d->valid;
}

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
   QStringList::iterator it = lst.begin();

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

void QHttpHeader::setValid(bool v)
{
   Q_D(QHttpHeader);
   d->valid = v;
}

QString QHttpHeader::value(const QString &key) const
{
   Q_D(const QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::const_iterator it = d->values.constBegin();

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
   QList<QPair<QString, QString> >::const_iterator it = d->values.constBegin();

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
   QList<QPair<QString, QString> >::const_iterator it = d->values.constBegin();
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
   QList<QPair<QString, QString> >::const_iterator it = d->values.constBegin();

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
   QList<QPair<QString, QString> >::iterator it = d->values.begin();

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

void QHttpHeader::setValues(const QList<QPair<QString, QString> > &values)
{
   Q_D(QHttpHeader);
   d->values = values;
}

void QHttpHeader::addValue(const QString &key, const QString &value)
{
   Q_D(QHttpHeader);
   d->values.append(qMakePair(key, value));
}

QList<QPair<QString, QString> > QHttpHeader::values() const
{
   Q_D(const QHttpHeader);
   return d->values;
}

void QHttpHeader::removeValue(const QString &key)
{
   Q_D(QHttpHeader);
   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::iterator it = d->values.begin();

   while (it != d->values.end()) {
      if ((*it).first.toLower() == lowercaseKey) {
         d->values.erase(it);
         return;
      }
      ++it;
   }
}

void QHttpHeader::removeAllValues(const QString &key)
{
   Q_D(QHttpHeader);

   QString lowercaseKey = key.toLower();
   QList<QPair<QString, QString> >::iterator it = d->values.begin();

   while (it != d->values.end()) {
      if ((*it).first.toLower() == lowercaseKey) {
         it = d->values.erase(it);
         continue;
      }
      ++it;
   }
}

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

   QList<QPair<QString, QString> >::const_iterator it = d->values.constBegin();
   while (it != d->values.constEnd()) {
      ret += (*it).first + QLatin1String(": ") + (*it).second + QLatin1String("\r\n");
      ++it;
   }
   return ret;
}

bool QHttpHeader::hasContentLength() const
{
   return hasKey(QLatin1String("content-length"));
}

qint64 QHttpHeader::contentLength() const
{
   return value(QLatin1String("content-length")).toInteger<uint>();
}

void QHttpHeader::setContentLength(qint64 len)
{
   setValue(QLatin1String("content-length"), QString::number(len));
}

bool QHttpHeader::hasContentType() const
{
   return hasKey(QLatin1String("content-type"));
}

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

void QHttpHeader::setContentType(const QString &type)
{
   setValue("content-type", type);
}

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

QHttpRequestHeader::QHttpRequestHeader(const QString &str)
   : QHttpHeader(*new QHttpRequestHeaderPrivate)
{
   parse(str);
}

void QHttpRequestHeader::setRequest(const QString &method, const QString &path, int majorVer, int minorVer)
{
   Q_D(QHttpRequestHeader);
   setValid(true);
   d->m = method;
   d->p = path;
   d->majVer = majorVer;
   d->minVer = minorVer;
}

QString QHttpRequestHeader::method() const
{
   Q_D(const QHttpRequestHeader);
   return d->m;
}


QString QHttpRequestHeader::path() const
{
   Q_D(const QHttpRequestHeader);
   return d->p;
}

int QHttpRequestHeader::majorVersion() const
{
   Q_D(const QHttpRequestHeader);
   return d->majVer;
}

int QHttpRequestHeader::minorVersion() const
{
   Q_D(const QHttpRequestHeader);
   return d->minVer;
}

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

   return first.formatArg(d->m).formatArg(d->p) + last.formatArg(d->majVer).formatArg(d->minVer).formatArg(QHttpHeader::toString());
}

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

QHttpResponseHeader::QHttpResponseHeader(const QHttpResponseHeader &header)
   : QHttpHeader(*new QHttpResponseHeaderPrivate, header)
{
   Q_D(QHttpResponseHeader);
   d->statCode = header.d_func()->statCode;
   d->reasonPhr = header.d_func()->reasonPhr;
   d->majVer = header.d_func()->majVer;
   d->minVer = header.d_func()->minVer;
}

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

QString QHttpResponseHeader::reasonPhrase() const
{
   Q_D(const QHttpResponseHeader);
   return d->reasonPhr;
}

int QHttpResponseHeader::majorVersion() const
{
   Q_D(const QHttpResponseHeader);
   return d->majVer;
}

int QHttpResponseHeader::minorVersion() const
{
   Q_D(const QHttpResponseHeader);
   return d->minVer;
}

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
         d->statCode = l.mid(9, pos - 9).toInteger<int>();

      } else {
         d->statCode = l.mid(9).toInteger<int>();
         d->reasonPhr.clear();
      }

   } else {
      return false;
   }

   return true;
}

QString QHttpResponseHeader::toString() const
{
   Q_D(const QHttpResponseHeader);

   QString retval(QLatin1String("HTTP/%1.%2 %3 %4\r\n%5\r\n"));

   return retval.formatArg(d->majVer).formatArg(d->minVer).formatArg(d->statCode)
         .formatArg(d->reasonPhr).formatArg(QHttpHeader::toString());
}
