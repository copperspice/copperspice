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

#include <qmediaplaylistnavigator_p.h>

#include <qdebug.h>
#include <qmediaplaylist.h>

#include <qmediaplaylistprovider_p.h>
#include <qmediaobject_p.h>

class QMediaPlaylistNullProvider : public QMediaPlaylistProvider
{
 public:
   QMediaPlaylistNullProvider()
      : QMediaPlaylistProvider()
   {
   }

   virtual ~QMediaPlaylistNullProvider()
   {
   }

   int mediaCount() const override {
      return 0;
   }

   QMediaContent media(int) const override {
      return QMediaContent();
   }
};

static QMediaPlaylistNullProvider *_q_nullMediaPlaylist()
{
   static QMediaPlaylistNullProvider retval;
   return &retval;
}

class QMediaPlaylistNavigatorPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QMediaPlaylistNavigator)

 public:
   QMediaPlaylistNavigatorPrivate()
      : playlist(nullptr), currentPos(-1), lastValidPos(-1),
        playbackMode(QMediaPlaylist::Sequential), randomPositionsOffset(-1)
   { }

   QMediaPlaylistProvider *playlist;
   int currentPos;
   int lastValidPos;          // to be used with CurrentItemOnce playback mode
   QMediaPlaylist::PlaybackMode playbackMode;
   QMediaContent currentItem;

   mutable QList<int> randomModePositions;
   mutable int randomPositionsOffset;

   int nextItemPos(int steps = 1) const;
   int previousItemPos(int steps = 1) const;

   void _q_mediaInserted(int start, int end);
   void _q_mediaRemoved(int start, int end);
   void _q_mediaChanged(int start, int end);

   QMediaPlaylistNavigator *q_ptr;
};

int QMediaPlaylistNavigatorPrivate::nextItemPos(int steps) const
{
   if (playlist->mediaCount() == 0) {
      return -1;
   }

   if (steps == 0) {
      return currentPos;
   }

   switch (playbackMode) {
      case QMediaPlaylist::CurrentItemOnce:
         return /*currentPos == -1 ? lastValidPos :*/ -1;

      case QMediaPlaylist::CurrentItemInLoop:
         return currentPos;

      case QMediaPlaylist::Sequential: {
         int nextPos = currentPos + steps;
         return nextPos < playlist->mediaCount() ? nextPos : -1;
      }

      case QMediaPlaylist::Loop:
         return (currentPos + steps) % playlist->mediaCount();

      case QMediaPlaylist::Random: {
         //TODO: limit the history size

         if (randomPositionsOffset == -1) {
            randomModePositions.clear();
            randomModePositions.append(currentPos);
            randomPositionsOffset = 0;
         }

         while (randomModePositions.size() < randomPositionsOffset + steps + 1) {
            randomModePositions.append(-1);
         }

         int res = randomModePositions[randomPositionsOffset + steps];
         if (res < 0 || res >= playlist->mediaCount()) {
            res = qrand() % playlist->mediaCount();
            randomModePositions[randomPositionsOffset + steps] = res;
         }

         return res;
      }
   }

   return -1;
}

int QMediaPlaylistNavigatorPrivate::previousItemPos(int steps) const
{
   if (playlist->mediaCount() == 0) {
      return -1;
   }

   if (steps == 0) {
      return currentPos;
   }

   switch (playbackMode) {
      case QMediaPlaylist::CurrentItemOnce:
         return -1;

      case QMediaPlaylist::CurrentItemInLoop:
         return currentPos;

      case QMediaPlaylist::Sequential: {
         int prevPos = currentPos == -1 ? playlist->mediaCount() - steps : currentPos - steps;
         return prevPos >= 0 ? prevPos : -1;
      }

      case QMediaPlaylist::Loop: {
         int prevPos = currentPos - steps;

         while (prevPos < 0) {
            prevPos += playlist->mediaCount();
         }

         return prevPos;
      }

      case QMediaPlaylist::Random: {
         //TODO: limit the history size

         if (randomPositionsOffset == -1) {
            randomModePositions.clear();
            randomModePositions.append(currentPos);
            randomPositionsOffset = 0;
         }

         while (randomPositionsOffset - steps < 0) {
            randomModePositions.prepend(-1);
            randomPositionsOffset++;
         }

         int res = randomModePositions[randomPositionsOffset - steps];
         if (res < 0 || res >= playlist->mediaCount()) {
            res = qrand() % playlist->mediaCount();
            randomModePositions[randomPositionsOffset - steps] = res;
         }

         return res;
      }
   }

   return -1;
}

QMediaPlaylistNavigator::QMediaPlaylistNavigator(QMediaPlaylistProvider *playlist, QObject *parent)
   : QObject(parent), d_ptr(new QMediaPlaylistNavigatorPrivate)
{
   d_ptr->q_ptr = this;

   setPlaylist(playlist ? playlist : _q_nullMediaPlaylist());
}

QMediaPlaylistNavigator::~QMediaPlaylistNavigator()
{
   delete d_ptr;
}

QMediaPlaylist::PlaybackMode QMediaPlaylistNavigator::playbackMode() const
{
   return d_func()->playbackMode;
}

void QMediaPlaylistNavigator::setPlaybackMode(QMediaPlaylist::PlaybackMode mode)
{
   Q_D(QMediaPlaylistNavigator);

   if (d->playbackMode == mode) {
      return;
   }

   if (mode == QMediaPlaylist::Random) {
      d->randomPositionsOffset = 0;
      d->randomModePositions.append(d->currentPos);
   } else if (d->playbackMode == QMediaPlaylist::Random) {
      d->randomPositionsOffset = -1;
      d->randomModePositions.clear();
   }

   d->playbackMode = mode;

   emit playbackModeChanged(mode);
   emit surroundingItemsChanged();
}

QMediaPlaylistProvider *QMediaPlaylistNavigator::playlist() const
{
   return d_func()->playlist;
}

void QMediaPlaylistNavigator::setPlaylist(QMediaPlaylistProvider *playlist)
{
   Q_D(QMediaPlaylistNavigator);

   if (d->playlist == playlist) {
      return;
   }

   if (d->playlist) {
      d->playlist->disconnect(this);
   }

   if (playlist) {
      d->playlist = playlist;
   } else {
      //assign to shared readonly null playlist
      d->playlist = _q_nullMediaPlaylist();
   }

   connect(d->playlist, &QMediaPlaylistProvider::mediaInserted, this, &QMediaPlaylistNavigator::_q_mediaInserted);
   connect(d->playlist, &QMediaPlaylistProvider::mediaRemoved,  this, &QMediaPlaylistNavigator::_q_mediaRemoved);
   connect(d->playlist, &QMediaPlaylistProvider::mediaChanged,  this, &QMediaPlaylistNavigator::_q_mediaChanged);

   d->randomPositionsOffset = -1;
   d->randomModePositions.clear();

   if (d->currentPos != -1) {
      d->currentPos = -1;
      emit currentIndexChanged(-1);
   }

   if (! d->currentItem.isNull()) {
      d->currentItem = QMediaContent();
      emit activated(d->currentItem); //stop playback
   }
}

QMediaContent QMediaPlaylistNavigator::currentItem() const
{
   return itemAt(d_func()->currentPos);
}

QMediaContent QMediaPlaylistNavigator::nextItem(int steps) const
{
   return itemAt(nextIndex(steps));
}

QMediaContent QMediaPlaylistNavigator::previousItem(int steps) const
{
   return itemAt(previousIndex(steps));
}

QMediaContent QMediaPlaylistNavigator::itemAt(int position) const
{
   return d_func()->playlist->media(position);
}

int QMediaPlaylistNavigator::currentIndex() const
{
   return d_func()->currentPos;
}

int QMediaPlaylistNavigator::nextIndex(int steps) const
{
   return d_func()->nextItemPos(steps);
}

int QMediaPlaylistNavigator::previousIndex(int steps) const
{
   return d_func()->previousItemPos(steps);
}

void QMediaPlaylistNavigator::next()
{
   Q_D(QMediaPlaylistNavigator);

   int nextPos = d->nextItemPos();

   if ( playbackMode() == QMediaPlaylist::Random ) {
      d->randomPositionsOffset++;
   }

   jump(nextPos);
}

void QMediaPlaylistNavigator::previous()
{
   Q_D(QMediaPlaylistNavigator);

   int prevPos = d->previousItemPos();
   if ( playbackMode() == QMediaPlaylist::Random ) {
      d->randomPositionsOffset--;
   }

   jump(prevPos);
}

void QMediaPlaylistNavigator::jump(int position)
{
   Q_D(QMediaPlaylistNavigator);

   if (position < -1 || position >= d->playlist->mediaCount()) {
      position = -1;
   }

   if (position != -1) {
      d->lastValidPos = position;
   }

   if (playbackMode() == QMediaPlaylist::Random) {
      if (d->randomModePositions[d->randomPositionsOffset] != position) {
         d->randomModePositions.clear();
         d->randomModePositions.append(position);
         d->randomPositionsOffset = 0;
      }
   }

   if (position != -1) {
      d->currentItem = d->playlist->media(position);
   } else {
      d->currentItem = QMediaContent();
   }

   if (position != d->currentPos) {
      d->currentPos = position;
      emit currentIndexChanged(d->currentPos);
      emit surroundingItemsChanged();
   }

   emit activated(d->currentItem);
}

void QMediaPlaylistNavigatorPrivate::_q_mediaInserted(int start, int end)
{
   Q_Q(QMediaPlaylistNavigator);

   if (currentPos >= start) {
      currentPos = end - start + 1;
      q->jump(currentPos);
   }

   //TODO: check if they really changed
   emit q->surroundingItemsChanged();
}

void QMediaPlaylistNavigatorPrivate::_q_mediaRemoved(int start, int end)
{
   Q_Q(QMediaPlaylistNavigator);

   if (currentPos > end) {
      currentPos = currentPos - end - start + 1;
      q->jump(currentPos);
   } else if (currentPos >= start) {
      //current item was removed
      currentPos = qMin(start, playlist->mediaCount() - 1);
      q->jump(currentPos);
   }

   //TODO: check if they really changed
   emit q->surroundingItemsChanged();
}

void QMediaPlaylistNavigatorPrivate::_q_mediaChanged(int start, int end)
{
   Q_Q(QMediaPlaylistNavigator);

   if (currentPos >= start && currentPos <= end) {
      QMediaContent src = playlist->media(currentPos);
      if (src != currentItem) {
         currentItem = src;
         emit q->activated(src);
      }
   }

   //TODO: check if they really changed
   emit q->surroundingItemsChanged();
}

void QMediaPlaylistNavigator::_q_mediaInserted(int start, int end)
{
   Q_D(QMediaPlaylistNavigator);
   d->_q_mediaInserted(start, end);
}

void QMediaPlaylistNavigator::_q_mediaRemoved(int start, int end)
{
   Q_D(QMediaPlaylistNavigator);
   d->_q_mediaRemoved(start, end);
}

void QMediaPlaylistNavigator::_q_mediaChanged(int start, int end)
{
   Q_D(QMediaPlaylistNavigator);
   d->_q_mediaChanged(start, end);
}
