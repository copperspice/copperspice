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

#ifndef QMEDIAPLAYLIST_H
#define QMEDIAPLAYLIST_H

#include <qobject.h>
#include <qmediacontent.h>
#include <qmediaobject.h>
#include <qmediabindableinterface.h>

class QMediaPlaylistProvider;
class QMediaPlaylistPrivate;

class Q_MULTIMEDIA_EXPORT QMediaPlaylist : public QObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QMediaPlaylist, QObject)

   CS_INTERFACES(QMediaBindableInterface)

   MULTI_CS_PROPERTY_READ(playbackMode,   playbackMode)
   MULTI_CS_PROPERTY_WRITE(playbackMode,  setPlaybackMode)
   MULTI_CS_PROPERTY_NOTIFY(playbackMode, playbackModeChanged)

   MULTI_CS_PROPERTY_READ(currentMedia,   currentMedia)
   MULTI_CS_PROPERTY_NOTIFY(currentMedia, currentMediaChanged)

   MULTI_CS_PROPERTY_READ(currentIndex,   currentIndex)
   MULTI_CS_PROPERTY_WRITE(currentIndex,  setCurrentIndex)
   MULTI_CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)

   MULTI_CS_ENUM(PlaybackMode)
   MULTI_CS_ENUM(Error)

 public:
   enum PlaybackMode {
      CurrentItemOnce,
      CurrentItemInLoop,
      Sequential,
      Loop,
      Random
   };

   enum Error {
      NoError,
      FormatError,
      FormatNotSupportedError,
      NetworkError,
      AccessDeniedError
   };

   explicit QMediaPlaylist(QObject *parent = nullptr);
   virtual ~QMediaPlaylist();

   QMediaObject *mediaObject() const override;

   PlaybackMode playbackMode() const;
   void setPlaybackMode(PlaybackMode mode);

   int currentIndex() const;
   QMediaContent currentMedia() const;

   int nextIndex(int steps = 1) const;
   int previousIndex(int steps = 1) const;

   QMediaContent media(int index) const;

   int mediaCount() const;
   bool isEmpty() const;
   bool isReadOnly() const;

   bool addMedia(const QMediaContent &content);
   bool addMedia(const QList<QMediaContent> &items);
   bool insertMedia(int index, const QMediaContent &content);
   bool insertMedia(int index, const QList<QMediaContent> &items);
   bool removeMedia(int pos);
   bool removeMedia(int start, int end);
   bool clear();

   void load(const QNetworkRequest &request, const char *format = nullptr);
   void load(const QUrl &location, const char *format = nullptr);
   void load(QIODevice *device, const char *format = nullptr);

   bool save(const QUrl &location, const char *format = nullptr);
   bool save(QIODevice *device, const char *format);

   Error error() const;
   QString errorString() const;

   MULTI_CS_SLOT_1(Public, void shuffle())
   MULTI_CS_SLOT_2(shuffle)

   MULTI_CS_SLOT_1(Public, void next())
   MULTI_CS_SLOT_2(next)

   MULTI_CS_SLOT_1(Public, void previous())
   MULTI_CS_SLOT_2(previous)

   MULTI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   MULTI_CS_SLOT_2(setCurrentIndex)

   MULTI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   MULTI_CS_SIGNAL_2(currentIndexChanged, index)

   MULTI_CS_SIGNAL_1(Public, void playbackModeChanged(QMediaPlaylist::PlaybackMode mode))
   MULTI_CS_SIGNAL_2(playbackModeChanged, mode)

   MULTI_CS_SIGNAL_1(Public, void currentMediaChanged(const QMediaContent &content))
   MULTI_CS_SIGNAL_2(currentMediaChanged, content)

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

   MULTI_CS_SIGNAL_1(Public, void loadFailed())
   MULTI_CS_SIGNAL_2(loadFailed)

 protected:
   bool setMediaObject(QMediaObject *object) override;
   QMediaPlaylistPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QMediaPlaylist)

   MULTI_CS_SLOT_1(Private, void _q_loadFailed(QMediaPlaylist::Error playlistError, const QString &errorMsg))
   MULTI_CS_SLOT_2(_q_loadFailed)
};

#endif
