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

#include <qmediaplaylistprovider_p.h>

#include <qurl.h>

QMediaPlaylistProvider::QMediaPlaylistProvider(QObject *parent)
   : QObject(parent), d_ptr(new QMediaPlaylistProviderPrivate)
{
}

QMediaPlaylistProvider::QMediaPlaylistProvider(QMediaPlaylistProviderPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
}

QMediaPlaylistProvider::~QMediaPlaylistProvider()
{
   delete d_ptr;
}

bool QMediaPlaylistProvider::load(const QNetworkRequest &request, const char *format)
{
   (void) request;
   (void) format;
   return false;
}

bool QMediaPlaylistProvider::load(QIODevice *device, const char *format)
{
   (void) device;
   (void) format;
   return false;
}

bool QMediaPlaylistProvider::save(const QUrl &location, const char *format)
{
   (void) location;
   (void) format;
   return false;
}

bool QMediaPlaylistProvider::save(QIODevice *device, const char *format)
{
   (void) device;
   (void) format;
   return false;
}

bool QMediaPlaylistProvider::isReadOnly() const
{
   return true;
}

bool QMediaPlaylistProvider::addMedia(const QMediaContent &media)
{
   (void) media;
   return false;
}

bool QMediaPlaylistProvider::addMedia(const QList<QMediaContent> &items)
{
   for (const QMediaContent &item : items) {
      if (! addMedia(item)) {
         return false;
      }
   }

   return true;
}

bool QMediaPlaylistProvider::insertMedia(int position, const QMediaContent &media)
{
   (void) position;
   (void) media;
   return false;
}

bool QMediaPlaylistProvider::insertMedia(int position, const QList<QMediaContent> &items)
{
   for (int i = 0; i < items.count(); i++) {
      if (!insertMedia(position + i, items.at(i))) {
         return false;
      }
   }

   return true;
}

bool QMediaPlaylistProvider::removeMedia(int position)
{
   (void) position;
   return false;
}

bool QMediaPlaylistProvider::removeMedia(int start, int end)
{
   for (int pos = start; pos <= end; pos++) {
      if (! removeMedia(pos)) {
         return false;
      }
   }

   return true;
}

bool QMediaPlaylistProvider::clear()
{
   return removeMedia(0, mediaCount() - 1);
}

void QMediaPlaylistProvider::shuffle()
{
}

