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

#ifndef QMEDIAPLAYLIST_P_H
#define QMEDIAPLAYLIST_P_H

#include <qmediaplaylist.h>

#include <qdebug.h>
#include <qmediaplayer.h>
#include <qmediaplayercontrol.h>

#include <qmedianetworkplaylistprovider_p.h>
#include <qmediaobject_p.h>
#include <qmediaplaylistcontrol_p.h>

class QMediaPlayerControl;
class QMediaPlaylistControl;
class QMediaPlaylistProvider;
class QMediaPlaylistReader;
class QMediaPlaylistWriter;

class QMediaPlaylistPrivate
{
 public:
   QMediaPlaylistPrivate()
      : mediaObject(nullptr), control(nullptr), networkPlaylistControl(nullptr), error(QMediaPlaylist::NoError)
   {
   }

   virtual ~QMediaPlaylistPrivate() {}

   void _q_loadFailed(QMediaPlaylist::Error playlistError, const QString &errorMsg) {
      this->error       = playlistError;
      this->errorString = errorMsg;

      emit q_ptr->loadFailed();
   }

   void _q_mediaObjectDeleted() {
      Q_Q(QMediaPlaylist);
      mediaObject = nullptr;

      if (control != networkPlaylistControl) {
         control = nullptr;
      }

      q->setMediaObject(nullptr);
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

 private:
   Q_DECLARE_PUBLIC(QMediaPlaylist)
};

class QMediaNetworkPlaylistControl : public QMediaPlaylistControl
{
   MULTI_CS_OBJECT(QMediaNetworkPlaylistControl)

 public:
   QMediaNetworkPlaylistControl(QObject *parent)
      : QMediaPlaylistControl(parent)
   {
      QMediaPlaylistProvider *playlist = new QMediaNetworkPlaylistProvider(this);
      m_navigator = new QMediaPlaylistNavigator(playlist, this);
      m_navigator->setPlaybackMode(QMediaPlaylist::Sequential);

      connect(m_navigator, &QMediaPlaylistNavigator::currentIndexChanged, this, &QMediaNetworkPlaylistControl::currentIndexChanged);
      connect(m_navigator, &QMediaPlaylistNavigator::activated,           this, &QMediaNetworkPlaylistControl::currentMediaChanged);
      connect(m_navigator, &QMediaPlaylistNavigator::playbackModeChanged, this, &QMediaNetworkPlaylistControl::playbackModeChanged);
   }

   virtual ~QMediaNetworkPlaylistControl() {};

   QMediaPlaylistProvider *playlistProvider() const override {
      return m_navigator->playlist();
   }

   bool setPlaylistProvider(QMediaPlaylistProvider *mediaPlaylist) override {
      m_navigator->setPlaylist(mediaPlaylist);
      emit playlistProviderChanged();
      return true;
   }

   int currentIndex() const override {
      return m_navigator->currentIndex();
   }

   void setCurrentIndex(int position) override {
      m_navigator->jump(position);
   }

   int nextIndex(int steps) const override {
      return m_navigator->nextIndex(steps);
   }

   int previousIndex(int steps) const override {
      return m_navigator->previousIndex(steps);
   }

   void next() override {
      m_navigator->next();
   }

   void previous() override {
      m_navigator->previous();
   }

   QMediaPlaylist::PlaybackMode playbackMode() const override {
      return m_navigator->playbackMode();
   }

   void setPlaybackMode(QMediaPlaylist::PlaybackMode mode) override {
      m_navigator->setPlaybackMode(mode);
   }

 private:
   QMediaPlaylistNavigator *m_navigator;
};

#endif
