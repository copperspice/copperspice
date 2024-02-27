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

#ifndef QMEDIANETWORKPAYLISTPROVIDER_P_H
#define QMEDIANETWORKPAYLISTPROVIDER_P_H

#include <playlistfileparser_p.h>
#include <qmediaplaylistprovider_p.h>

class QMediaNetworkPlaylistProviderPrivate;

class Q_MULTIMEDIA_EXPORT QMediaNetworkPlaylistProvider : public QMediaPlaylistProvider
{
   MULTI_CS_OBJECT(QMediaNetworkPlaylistProvider)

 public:
   QMediaNetworkPlaylistProvider(QObject *parent = nullptr);

   QMediaNetworkPlaylistProvider(const QMediaNetworkPlaylistProvider &) = delete;
   QMediaNetworkPlaylistProvider &operator=(const QMediaNetworkPlaylistProvider &) = delete;

   virtual ~QMediaNetworkPlaylistProvider();

   bool load(const QNetworkRequest &request, const char *format = nullptr) override;

   int mediaCount() const override;
   QMediaContent media(int pos) const override;

   bool isReadOnly() const override;

   bool addMedia(const QMediaContent &content) override;
   bool addMedia(const QList<QMediaContent> &items) override;
   bool insertMedia(int pos, const QMediaContent &content) override;
   bool insertMedia(int pos, const QList<QMediaContent> &items) override;
   bool removeMedia(int pos) override;
   bool removeMedia(int start, int end) override;
   bool clear() override;

   MULTI_CS_SLOT_1(Public, void shuffle() override)
   MULTI_CS_SLOT_2(shuffle)

 private:
   Q_DECLARE_PRIVATE(QMediaNetworkPlaylistProvider)

   MULTI_CS_SLOT_1(Private, void _q_handleParserError(QPlaylistFileParser::ParserError error, const QString &errorMsg))
   MULTI_CS_SLOT_2(_q_handleParserError)

   MULTI_CS_SLOT_1(Private, void _q_handleNewItem(const QVariant &content))
   MULTI_CS_SLOT_2(_q_handleNewItem)
};

#endif

