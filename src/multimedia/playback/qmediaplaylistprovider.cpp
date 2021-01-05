/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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
   Q_UNUSED(request);
   Q_UNUSED(format);
   return false;
}

bool QMediaPlaylistProvider::load(QIODevice *device, const char *format)
{
   Q_UNUSED(device);
   Q_UNUSED(format);
   return false;
}

bool QMediaPlaylistProvider::save(const QUrl &location, const char *format)
{
   Q_UNUSED(location);
   Q_UNUSED(format);
   return false;
}

bool QMediaPlaylistProvider::save(QIODevice *device, const char *format)
{
   Q_UNUSED(device);
   Q_UNUSED(format);
   return false;
}

bool QMediaPlaylistProvider::isReadOnly() const
{
   return true;
}

bool QMediaPlaylistProvider::addMedia(const QMediaContent &media)
{
   Q_UNUSED(media);
   return false;
}

/*!
    Append multiple media \a items to a playlist.

    Returns true if the media items were appended; and false otherwise.
*/
bool QMediaPlaylistProvider::addMedia(const QList<QMediaContent> &items)
{
   for (const QMediaContent &item : items) {
      if (!addMedia(item)) {
         return false;
      }
   }

   return true;
}

/*!
    Inserts \a media into a playlist at \a position.

    Returns true if the media was inserted; and false otherwise.
*/
bool QMediaPlaylistProvider::insertMedia(int position, const QMediaContent &media)
{
   Q_UNUSED(position);
   Q_UNUSED(media);
   return false;
}

/*!
    Inserts multiple media \a items into a playlist at \a position.

    Returns true if the media \a items were inserted; and false otherwise.
*/
bool QMediaPlaylistProvider::insertMedia(int position, const QList<QMediaContent> &items)
{
   for (int i = 0; i < items.count(); i++) {
      if (!insertMedia(position + i, items.at(i))) {
         return false;
      }
   }

   return true;
}


/*!
    Removes the media at \a position from a playlist.

    Returns true if the media was removed; and false otherwise.
*/
bool QMediaPlaylistProvider::removeMedia(int position)
{
   Q_UNUSED(position);
   return false;
}

/*!
    Removes the media between the given \a start and \a end positions from a playlist.

    Returns true if the media was removed; and false otherwise.
  */
bool QMediaPlaylistProvider::removeMedia(int start, int end)
{
   for (int pos = start; pos <= end; pos++) {
      if (!removeMedia(pos)) {
         return false;
      }
   }

   return true;
}

/*!
    Removes all media from a playlist.

    Returns true if the media was removed; and false otherwise.
*/
bool QMediaPlaylistProvider::clear()
{
   return removeMedia(0, mediaCount() - 1);
}

/*!
    Shuffles the contents of a playlist.
*/
void QMediaPlaylistProvider::shuffle()
{
}

