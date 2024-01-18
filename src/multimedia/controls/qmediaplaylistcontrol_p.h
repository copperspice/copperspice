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

#ifndef QMEDIAPLAYLISTCONTROL_P_H
#define QMEDIAPLAYLISTCONTROL_P_H

#include <qobject.h>
#include <qmediacontrol.h>
#include <qmediaplaylistnavigator_p.h>

class QMediaPlaylistProvider;

class Q_MULTIMEDIA_EXPORT QMediaPlaylistControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaPlaylistControl)

 public:
   virtual ~QMediaPlaylistControl();

   virtual QMediaPlaylistProvider *playlistProvider() const = 0;
   virtual bool setPlaylistProvider(QMediaPlaylistProvider *playlist) = 0;

   virtual int currentIndex() const = 0;
   virtual void setCurrentIndex(int position) = 0;
   virtual int nextIndex(int steps) const = 0;
   virtual int previousIndex(int steps) const = 0;

   virtual void next() = 0;
   virtual void previous() = 0;

   virtual QMediaPlaylist::PlaybackMode playbackMode() const = 0;
   virtual void setPlaybackMode(QMediaPlaylist::PlaybackMode mode) = 0;

   MULTI_CS_SIGNAL_1(Public, void playlistProviderChanged())
   MULTI_CS_SIGNAL_2(playlistProviderChanged)

   MULTI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   MULTI_CS_SIGNAL_2(currentIndexChanged, index)

   MULTI_CS_SIGNAL_1(Public, void currentMediaChanged(const QMediaContent &content))
   MULTI_CS_SIGNAL_2(currentMediaChanged, content)

   MULTI_CS_SIGNAL_1(Public, void playbackModeChanged(QMediaPlaylist::PlaybackMode mode))
   MULTI_CS_SIGNAL_2(playbackModeChanged, mode)

 protected:
   QMediaPlaylistControl(QObject *parent = nullptr);
};

#define QMediaPlaylistControl_iid "com.copperspice.CS.mediaPlayListControl/1.0"
CS_DECLARE_INTERFACE(QMediaPlaylistControl, QMediaPlaylistControl_iid)

#endif
