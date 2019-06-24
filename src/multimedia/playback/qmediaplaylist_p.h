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

#ifndef QMEDIAPLAYLIST_P_H
#define QMEDIAPLAYLIST_P_H

#include <qmediaplaylist.h>
#include <qmediaplayer.h>
#include <qmediaplayercontrol.h>
#include <qdebug.h>

#include <qmediaplaylistcontrol_p.h>
#include <qmedianetworkplaylistprovider_p.h>
#include <qmediaobject_p.h>

class QMediaPlaylistControl;
class QMediaPlaylistProvider;
class QMediaPlaylistReader;
class QMediaPlaylistWriter;
class QMediaPlayerControl;

class QMediaPlaylistPrivate
{
   Q_DECLARE_PUBLIC(QMediaPlaylist)

 public:
   QMediaPlaylistPrivate()
      : mediaObject(0), control(0), networkPlaylistControl(0), error(QMediaPlaylist::NoError) {
   }

   virtual ~QMediaPlaylistPrivate() {}

   void _q_loadFailed(QMediaPlaylist::Error error, const QString &errorString) {
      this->error = error;
      this->errorString = errorString;

      emit q_ptr->loadFailed();
   }

   void _q_mediaObjectDeleted() {
      Q_Q(QMediaPlaylist);
      mediaObject = 0;

      if (control != networkPlaylistControl) {
         control = 0;
      }

      q->setMediaObject(0);
   }

   QMediaObject *mediaObject;

   QMediaPlaylistControl *control;
   QMediaPlaylistProvider *playlist() const {
      return control->playlistProvider();
   }

   QMediaPlaylistControl *networkPlaylistControl;

   bool readItems(QMediaPlaylistReader *reader);
   bool writeItems(QMediaPlaylistWriter *writer);

   void syncControls(QMediaPlaylistControl *oldControl, QMediaPlaylistControl *newControl,
      int *removedStart, int *removedEnd, int *insertedStart, int *insertedEnd);

   QMediaPlaylist::Error error;
   QString errorString;

   QMediaPlaylist *q_ptr;
};

class QMediaNetworkPlaylistControl : public QMediaPlaylistControl
{
   MULTI_CS_OBJECT(QMediaNetworkPlaylistControl)

 public:
   QMediaNetworkPlaylistControl(QObject *parent)
      : QMediaPlaylistControl(parent) {
      QMediaPlaylistProvider *playlist = new QMediaNetworkPlaylistProvider(this);
      m_navigator = new QMediaPlaylistNavigator(playlist, this);
      m_navigator->setPlaybackMode(QMediaPlaylist::Sequential);

      connect(m_navigator, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
      connect(m_navigator, SIGNAL(activated(QMediaContent)), this, SLOT(currentMediaChanged(QMediaContent)));
      connect(m_navigator, SIGNAL(playbackModeChanged(QMediaPlaylist::PlaybackMode)), this,
         SLOT(playbackModeChanged(QMediaPlaylist::PlaybackMode)));
   }

   virtual ~QMediaNetworkPlaylistControl() {};

   QMediaPlaylistProvider *playlistProvider() const {
      return m_navigator->playlist();
   }
   bool setPlaylistProvider(QMediaPlaylistProvider *mediaPlaylist) {
      m_navigator->setPlaylist(mediaPlaylist);
      emit playlistProviderChanged();
      return true;
   }

   int currentIndex() const {
      return m_navigator->currentIndex();
   }
   void setCurrentIndex(int position) {
      m_navigator->jump(position);
   }
   int nextIndex(int steps) const {
      return m_navigator->nextIndex(steps);
   }
   int previousIndex(int steps) const {
      return m_navigator->previousIndex(steps);
   }

   void next() {
      m_navigator->next();
   }
   void previous() {
      m_navigator->previous();
   }

   QMediaPlaylist::PlaybackMode playbackMode() const {
      return m_navigator->playbackMode();
   }
   void setPlaybackMode(QMediaPlaylist::PlaybackMode mode) {
      m_navigator->setPlaybackMode(mode);
   }

 private:
   QMediaPlaylistNavigator *m_navigator;
};

#endif
