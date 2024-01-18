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

#include "qscriptscriptdata_p.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class QScriptScriptDataPrivate
{
 public:
   QScriptScriptDataPrivate();
   ~QScriptScriptDataPrivate();

   QString contents;
   QString fileName;
   int baseLineNumber;
   QDateTime timeStamp;

   QAtomicInt ref;
};

QScriptScriptDataPrivate::QScriptScriptDataPrivate()
{
   ref.store(0);
}

QScriptScriptDataPrivate::~QScriptScriptDataPrivate()
{
}

QScriptScriptData::QScriptScriptData()
   : d_ptr(0)
{
}

QScriptScriptData::QScriptScriptData(const QString &contents, const QString &fileName,
                                     int baseLineNumber, const QDateTime &timeStamp)
   : d_ptr(new QScriptScriptDataPrivate)
{
   d_ptr->contents = contents;
   d_ptr->fileName = fileName;
   d_ptr->baseLineNumber = baseLineNumber;
   if (timeStamp.isValid()) {
      d_ptr->timeStamp = timeStamp;
   } else {
      d_ptr->timeStamp = QDateTime::currentDateTime();
   }
   d_ptr->ref.ref();
}

QScriptScriptData::QScriptScriptData(const QScriptScriptData &other)
   : d_ptr(other.d_ptr.data())
{
   if (d_ptr) {
      d_ptr->ref.ref();
   }
}

QScriptScriptData::~QScriptScriptData()
{
}

QScriptScriptData &QScriptScriptData::operator=(const QScriptScriptData &other)
{
   d_ptr.assign(other.d_ptr.data());
   return *this;
}

QString QScriptScriptData::contents() const
{
   Q_D(const QScriptScriptData);
   if (!d) {
      return QString();
   }
   return d->contents;
}

QStringList QScriptScriptData::lines(int startLineNumber, int count) const
{
   Q_D(const QScriptScriptData);
   if (!d) {
      return QStringList();
   }
   QStringList allLines = d->contents.split(QLatin1Char('\n'));
   return allLines.mid(qMax(0, startLineNumber - d->baseLineNumber), count);
}

QString QScriptScriptData::fileName() const
{
   Q_D(const QScriptScriptData);
   if (!d) {
      return QString();
   }
   return d->fileName;
}

int QScriptScriptData::baseLineNumber() const
{
   Q_D(const QScriptScriptData);
   if (!d) {
      return -1;
   }
   return d->baseLineNumber;
}

QDateTime QScriptScriptData::timeStamp() const
{
   Q_D(const QScriptScriptData);
   if (!d) {
      return QDateTime();
   }
   return d->timeStamp;
}

bool QScriptScriptData::isValid() const
{
   Q_D(const QScriptScriptData);
   return (d != 0);
}

bool QScriptScriptData::operator==(const QScriptScriptData &other) const
{
   Q_D(const QScriptScriptData);
   const QScriptScriptDataPrivate *od = other.d_func();
   if (d == od) {
      return true;
   }
   if (!d || !od) {
      return false;
   }
   return ((d->contents == od->contents)
           && (d->fileName == od->fileName)
           && (d->baseLineNumber == od->baseLineNumber));
}

bool QScriptScriptData::operator!=(const QScriptScriptData &other) const
{
   return !(*this == other);
}

QDataStream &operator<<(QDataStream &out, const QScriptScriptData &data)
{
   const QScriptScriptDataPrivate *d = data.d_ptr.data();
   if (d) {
      out << d->contents;
      out << d->fileName;
      out << qint32(d->baseLineNumber);
   } else {
      out << QString();
      out << QString();
      out << qint32(0);
   }
   return out;
}

QDataStream &operator>>(QDataStream &in, QScriptScriptData &data)
{
   if (!data.d_ptr) {
      data.d_ptr.reset(new QScriptScriptDataPrivate());
      data.d_ptr->ref.ref();
   }
   QScriptScriptDataPrivate *d = data.d_ptr.data();
   in >> d->contents;
   in >> d->fileName;
   qint32 ln;
   in >> ln;
   d->baseLineNumber = ln;
   return in;
}

QT_END_NAMESPACE
