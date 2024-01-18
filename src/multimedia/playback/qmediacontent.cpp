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

#include <qurl.h>
#include <qvariant.h>
#include <QPointer>

#include <qmediaplaylist.h>
#include <qmediacontent.h>

class QMediaContentPrivate : public QSharedData
{
 public:
   QMediaContentPrivate()
      : isPlaylistOwned(false)
   {}

   QMediaContentPrivate(const QList<QMediaResource> &r)
      : resources(r), isPlaylistOwned(false)
   {}

   QMediaContentPrivate(const QMediaContentPrivate &other):
      QSharedData(other), resources(other.resources), playlist(other.playlist), isPlaylistOwned(false)
   {}

   QMediaContentPrivate(QMediaPlaylist *pls, const QUrl &url, bool isOwn):
      playlist(pls),
      isPlaylistOwned(isOwn) {
      resources << QMediaResource(url);
   }

   ~QMediaContentPrivate() {
      if (isPlaylistOwned && !playlist.isNull()) {
         playlist.data()->deleteLater();
      }
   }

   bool operator ==(const QMediaContentPrivate &other) const {
      return resources == other.resources && playlist == other.playlist;
   }

   QList<QMediaResource> resources;

   QPointer<QMediaPlaylist> playlist;
   bool isPlaylistOwned;

 private:
   QMediaContentPrivate &operator=(const QMediaContentPrivate &other);
};

QMediaContent::QMediaContent()
{
}

QMediaContent::QMediaContent(const QUrl &url)
   : d(new QMediaContentPrivate)
{
   d->resources << QMediaResource(url);
}


QMediaContent::QMediaContent(const QNetworkRequest &request)
   : d(new QMediaContentPrivate)
{
   d->resources << QMediaResource(request);
}

QMediaContent::QMediaContent(const QMediaResource &resource)
   : d(new QMediaContentPrivate)
{
   d->resources << resource;
}

QMediaContent::QMediaContent(const QList<QMediaResource> &resources)
   : d(new QMediaContentPrivate(resources))
{
}

QMediaContent::QMediaContent(const QMediaContent &other)
   : d(other.d)
{
}

QMediaContent::QMediaContent(QMediaPlaylist *playlist, const QUrl &contentUrl, bool takeOwnership)
   : d(new QMediaContentPrivate(playlist, contentUrl, takeOwnership))
{
}

QMediaContent::~QMediaContent()
{
}

QMediaContent &QMediaContent::operator=(const QMediaContent &other)
{
   d = other.d;
   return *this;
}

bool QMediaContent::operator==(const QMediaContent &other) const
{
   return (d.constData() == nullptr && other.d.constData() == nullptr) ||
      (d.constData() != nullptr && other.d.constData() != nullptr &&
      *d.constData() == *other.d.constData());
}

bool QMediaContent::operator!=(const QMediaContent &other) const
{
   return !(*this == other);
}

bool QMediaContent::isNull() const
{
   return d.constData() == nullptr;
}

QUrl QMediaContent::canonicalUrl() const
{
   return canonicalResource().url();
}

QNetworkRequest QMediaContent::canonicalRequest() const
{
   return canonicalResource().request();
}

QMediaResource QMediaContent::canonicalResource() const
{
   return d.constData() != nullptr ?  d->resources.value(0) : QMediaResource();
}

QList<QMediaResource> QMediaContent::resources() const
{
   return d.constData() != nullptr ? d->resources : QList<QMediaResource>();
}

QMediaPlaylist *QMediaContent::playlist() const
{
   return d.constData() != nullptr ? d->playlist.data() : nullptr;
}

