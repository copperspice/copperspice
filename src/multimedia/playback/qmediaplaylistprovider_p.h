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

#ifndef QMEDIAPLAYLISTPROVIDER_P_H
#define QMEDIAPLAYLISTPROVIDER_P_H

#include <qmediacontent.h>
#include <qmediaplaylist.h>

class QMediaPlaylistProviderPrivate
{
 public:
   QMediaPlaylistProviderPrivate()
   {}

   virtual ~QMediaPlaylistProviderPrivate()
   {}
};

class Q_MULTIMEDIA_EXPORT QMediaPlaylistProvider : public QObject
{
   MULTI_CS_OBJECT(QMediaPlaylistProvider)

 public:
   QMediaPlaylistProvider(QObject *parent = nullptr);
   virtual ~QMediaPlaylistProvider();

   virtual bool load(const QNetworkRequest &request, const char *format = nullptr);
   virtual bool load(QIODevice *device, const char *format = nullptr);
   virtual bool save(const QUrl &location, const char *format = nullptr);
   virtual bool save(QIODevice *device, const char *format);

   virtual int mediaCount() const = 0;
   virtual QMediaContent media(int index) const = 0;

   virtual bool isReadOnly() const;

   virtual bool addMedia(const QMediaContent &content);
   virtual bool addMedia(const QList<QMediaContent> &contentList);
   virtual bool insertMedia(int index, const QMediaContent &content);
   virtual bool insertMedia(int index, const QList<QMediaContent> &content);
   virtual bool removeMedia(int pos);
   virtual bool removeMedia(int start, int end);
   virtual bool clear();

   MULTI_CS_SLOT_1(Public, virtual void shuffle())
   MULTI_CS_SLOT_2(shuffle)

   MULTI_CS_SIGNAL_1(Public, void mediaAboutToBeInserted(int start, int end))
   MULTI_CS_SIGNAL_2(mediaAboutToBeInserted, start, end)
   MULTI_CS_SIGNAL_1(Public, void mediaInserted(int start, int end))
   MULTI_CS_SIGNAL_2(mediaInserted, start, end)

   MULTI_CS_SIGNAL_1(Public, void mediaAboutToBeRemoved(int start, int end))
   MULTI_CS_SIGNAL_2(mediaAboutToBeRemoved, start, end)
   MULTI_CS_SIGNAL_1(Public, void mediaRemoved(int start, int end))
   MULTI_CS_SIGNAL_2(mediaRemoved, start, end)

   MULTI_CS_SIGNAL_1(Public, void mediaChanged(int start, int end))
   MULTI_CS_SIGNAL_2(mediaChanged, start, end)

   MULTI_CS_SIGNAL_1(Public, void loaded())
   MULTI_CS_SIGNAL_2(loaded)

   MULTI_CS_SIGNAL_1(Public, void loadFailed(QMediaPlaylist::Error playlistError, const QString &errorMsg))
   MULTI_CS_SIGNAL_2(loadFailed, playlistError, errorMsg)

 protected:
   QMediaPlaylistProviderPrivate *d_ptr;
   QMediaPlaylistProvider(QMediaPlaylistProviderPrivate &dd, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QMediaPlaylistProvider)
};

#endif