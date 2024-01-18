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

#ifndef QMEDIAPLAYLISTNAVIGATOR_P_H
#define QMEDIAPLAYLISTNAVIGATOR_P_H

#include <qmediaplaylistprovider_p.h>
#include <qmediaplaylist.h>
#include <qobject.h>

class QMediaPlaylistNavigatorPrivate;

class Q_MULTIMEDIA_EXPORT QMediaPlaylistNavigator : public QObject
{
   MULTI_CS_OBJECT(QMediaPlaylistNavigator)

   MULTI_CS_PROPERTY_READ(playbackMode, playbackMode)
   MULTI_CS_PROPERTY_WRITE(playbackMode, setPlaybackMode)
   MULTI_CS_PROPERTY_NOTIFY(playbackMode, playbackModeChanged)
   MULTI_CS_PROPERTY_READ(currentIndex, currentIndex)
   MULTI_CS_PROPERTY_WRITE(currentIndex, jump)
   MULTI_CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
   MULTI_CS_PROPERTY_READ(currentItem, currentItem)

 public:
   QMediaPlaylistNavigator(QMediaPlaylistProvider *playlist, QObject *parent = nullptr);

   QMediaPlaylistNavigator(const QMediaPlaylistNavigator &) = delete;
   QMediaPlaylistNavigator &operator=(const QMediaPlaylistNavigator &) = delete;

   virtual ~QMediaPlaylistNavigator();

   QMediaPlaylistProvider *playlist() const;
   void setPlaylist(QMediaPlaylistProvider *playlist);

   QMediaPlaylist::PlaybackMode playbackMode() const;

   QMediaContent currentItem() const;
   QMediaContent nextItem(int steps = 1) const;
   QMediaContent previousItem(int steps = 1) const;

   QMediaContent itemAt(int position) const;

   int currentIndex() const;
   int nextIndex(int steps = 1) const;
   int previousIndex(int steps = 1) const;

   MULTI_CS_SLOT_1(Public, void next())
   MULTI_CS_SLOT_2(next)

   MULTI_CS_SLOT_1(Public, void previous())
   MULTI_CS_SLOT_2(previous)

   MULTI_CS_SLOT_1(Public, void jump(int position))
   MULTI_CS_SLOT_2(jump)

   MULTI_CS_SLOT_1(Public, void setPlaybackMode(QMediaPlaylist::PlaybackMode mode))
   MULTI_CS_SLOT_2(setPlaybackMode)

   MULTI_CS_SIGNAL_1(Public, void activated(const QMediaContent &content))
   MULTI_CS_SIGNAL_2(activated, content)

   MULTI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   MULTI_CS_SIGNAL_2(currentIndexChanged, index)

   MULTI_CS_SIGNAL_1(Public, void playbackModeChanged(QMediaPlaylist::PlaybackMode mode))
   MULTI_CS_SIGNAL_2(playbackModeChanged, mode)

   MULTI_CS_SIGNAL_1(Public, void surroundingItemsChanged())
   MULTI_CS_SIGNAL_2(surroundingItemsChanged)

 protected:
   QMediaPlaylistNavigatorPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QMediaPlaylistNavigator)

   MULTI_CS_SLOT_1(Private, void _q_mediaInserted(int start, int end))
   MULTI_CS_SLOT_2(_q_mediaInserted)

   MULTI_CS_SLOT_1(Private, void _q_mediaRemoved(int start, int end))
   MULTI_CS_SLOT_2(_q_mediaRemoved)

   MULTI_CS_SLOT_1(Private, void _q_mediaChanged(int start, int end))
   MULTI_CS_SLOT_2(_q_mediaChanged)
};

#endif



