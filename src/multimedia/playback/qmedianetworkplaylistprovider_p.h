/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMEDIANETWORKPAYLISTPROVIDER_P_H
#define QMEDIANETWORKPAYLISTPROVIDER_P_H

#include <playlistfileparser_p.h>
#include <qmediaplaylistprovider_p.h>

class QMediaNetworkPlaylistProviderPrivate;

class Q_MULTIMEDIA_EXPORT QMediaNetworkPlaylistProvider : public QMediaPlaylistProvider
{
   MULTI_CS_OBJECT(QMediaNetworkPlaylistProvider)

 public:
   QMediaNetworkPlaylistProvider(QObject *parent = 0);
   virtual ~QMediaNetworkPlaylistProvider();

   virtual bool load(const QNetworkRequest &request, const char *format = 0);

   virtual int mediaCount() const;
   virtual QMediaContent media(int pos) const;

   virtual bool isReadOnly() const;

   virtual bool addMedia(const QMediaContent &content);
   virtual bool addMedia(const QList<QMediaContent> &items);
   virtual bool insertMedia(int pos, const QMediaContent &content);
   virtual bool insertMedia(int pos, const QList<QMediaContent> &items);
   virtual bool removeMedia(int pos);
   virtual bool removeMedia(int start, int end);
   virtual bool clear();

   MULTI_CS_SLOT_1(Public, virtual void shuffle())
   MULTI_CS_SLOT_2(shuffle)

 private:
   Q_DISABLE_COPY(QMediaNetworkPlaylistProvider)
   Q_DECLARE_PRIVATE(QMediaNetworkPlaylistProvider)

   MULTI_CS_SLOT_1(Private, void _q_handleParserError(QPlaylistFileParser::ParserError err, const QString &un_named_arg2))
   MULTI_CS_SLOT_2(_q_handleParserError)

   MULTI_CS_SLOT_1(Private, void _q_handleNewItem(const QVariant &content))
   MULTI_CS_SLOT_2(_q_handleNewItem)
};

#endif



