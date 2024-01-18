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

#ifndef QNETWORK_REQUEST_P_H
#define QNETWORK_REQUEST_P_H

#include <qnetwork_request.h>
#include <qbytearray.h>
#include <qlist.h>
#include <qhash.h>
#include <qshareddata.h>
#include <qsharedpointer.h>

// this is the common part between QNetworkRequestPrivate, QNetworkReplyPrivate and QHttpPartPrivate
class QNetworkHeadersPrivate
{
 public:
   typedef QPair<QByteArray, QByteArray> RawHeaderPair;
   typedef QList<RawHeaderPair> RawHeadersList;
   typedef QHash<QNetworkRequest::KnownHeaders, QVariant> CookedHeadersMap;
   typedef QHash<QNetworkRequest::Attribute, QVariant> AttributesMap;

   RawHeadersList rawHeaders;
   CookedHeadersMap cookedHeaders;
   AttributesMap attributes;
   QPointer<QObject> originatingObject;

   RawHeadersList::const_iterator findRawHeader(const QByteArray &key) const;
   RawHeadersList allRawHeaders() const;
   QList<QByteArray> rawHeadersKeys() const;
   void setRawHeader(const QByteArray &key, const QByteArray &value);
   void setAllRawHeaders(const RawHeadersList &list);
   void setCookedHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);

   static QDateTime fromHttpDate(const QByteArray &value);
   static QByteArray toHttpDate(const QDateTime &dt);

 private:
   void setRawHeaderInternal(const QByteArray &key, const QByteArray &value);
   void parseAndSetHeader(const QByteArray &key, const QByteArray &value);
};

#endif
