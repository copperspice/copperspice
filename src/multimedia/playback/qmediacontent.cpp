/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

// register by hand
Q_MULTIMEDIA_EXPORT const QString &cs_typeName_internal<QMediaContent, void>::typeName()
{
   static QString retval("QMediaContent");
   return retval;
}

class QMediaContentPrivate : public QSharedData
{
 public:
   QMediaContentPrivate()
      : isPlaylistOwned(false)
   {}

   QMediaContentPrivate(const QMediaResourceList &r)
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

   QMediaResourceList resources;

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

QMediaContent::QMediaContent(const QMediaResourceList &resources)
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
   return (d.constData() == 0 && other.d.constData() == 0) ||
      (d.constData() != 0 && other.d.constData() != 0 &&
         *d.constData() == *other.d.constData());
}

bool QMediaContent::operator!=(const QMediaContent &other) const
{
   return !(*this == other);
}

bool QMediaContent::isNull() const
{
   return d.constData() == 0;
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
   return d.constData() != 0 ?  d->resources.value(0) : QMediaResource();
}

QMediaResourceList QMediaContent::resources() const
{
   return d.constData() != 0 ? d->resources : QMediaResourceList();
}

QMediaPlaylist *QMediaContent::playlist() const
{
   return d.constData() != 0 ? d->playlist.data() : 0;
}

