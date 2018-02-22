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

#include <qhttpnetworkheader_p.h>



QHttpNetworkHeaderPrivate::QHttpNetworkHeaderPrivate(const QUrl &newUrl)
   : url(newUrl)
{
}

QHttpNetworkHeaderPrivate::QHttpNetworkHeaderPrivate(const QHttpNetworkHeaderPrivate &other)
   : QSharedData(other)
{
   url = other.url;
   fields = other.fields;
}

qint64 QHttpNetworkHeaderPrivate::contentLength() const
{
   bool ok = false;

   // We are not using the headerField() method here because servers might send us multiple content-length
   // headers, just take the first content-length header field

   QByteArray value;

   QList<QPair<QByteArray, QByteArray> >::ConstIterator it  = fields.constBegin();
   QList<QPair<QByteArray, QByteArray> >::ConstIterator end = fields.constEnd();

   for ( ; it != end; ++it)
      if ( qstricmp("content-length", it->first.constData() ) == 0) {
         value = it->second;
         break;
      }

   qint64 length = value.toULongLong(&ok);

   if (ok) {
      return length;
   }

   return -1; // the header field is not set
}

void QHttpNetworkHeaderPrivate::setContentLength(qint64 length)
{
   setHeaderField("Content-Length", QByteArray::number(length));
}

QByteArray QHttpNetworkHeaderPrivate::headerField(const QByteArray &name, const QByteArray &defaultValue) const
{
   QList<QByteArray> allValues = headerFieldValues(name);
   if (allValues.isEmpty()) {
      return defaultValue;
   }

   QByteArray result;
   bool first = true;

   for (const QByteArray &value : allValues) {
      if (! first) {
         result += ", ";
      }

      first = false;
      result += value;
   }

   return result;
}

QList<QByteArray> QHttpNetworkHeaderPrivate::headerFieldValues(const QByteArray &name) const
{
   QList<QByteArray> result;
   QList<QPair<QByteArray, QByteArray> >::ConstIterator it  = fields.constBegin();
   QList<QPair<QByteArray, QByteArray> >::ConstIterator end = fields.constEnd();

   for ( ; it != end; ++it)
      if (qstricmp(name.constData(), it->first.constData()) == 0) {
         result += it->second;
      }

   return result;
}

void QHttpNetworkHeaderPrivate::setHeaderField(const QByteArray &name, const QByteArray &data)
{
   QList<QPair<QByteArray, QByteArray> >::Iterator it = fields.begin();

   while (it != fields.end()) {
      if (qstricmp(name.constData(), it->first.constData()) == 0) {
         it = fields.erase(it);
      } else {
         ++it;
      }
   }

   fields.append(qMakePair(name, data));
}

bool QHttpNetworkHeaderPrivate::operator==(const QHttpNetworkHeaderPrivate &other) const
{
   return (url == other.url);
}


