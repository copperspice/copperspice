/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qmediaplaylist.h>
#include <qmediaplaylist_p.h>

#include <qcoreapplication.h>
#include <qcoreevent.h>
#include <qfile.h>
#include <qlist.h>
#include <qmediaplayercontrol.h>
#include <qmediaservice.h>
#include <qurl.h>

#include <qfactoryloader_p.h>
#include <qmedianetworkplaylistprovider_p.h>
#include <qmediaplaylistcontrol_p.h>
#include <qmediaplaylistioplugin_p.h>
#include <qmediaplaylistprovider_p.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QMediaPlaylistInterface_ID, "/playlistformats", Qt::CaseInsensitive);
   return &retval;
}

QMediaPlaylist::QMediaPlaylist(QObject *parent)
   : QObject(parent), d_ptr(new QMediaPlaylistPrivate)
{
   Q_D(QMediaPlaylist);

   d->q_ptr = this;
   d->networkPlaylistControl = new QMediaNetworkPlaylistControl(this);

   setMediaObject(nullptr);
}

QMediaPlaylist::~QMediaPlaylist()
{
   Q_D(QMediaPlaylist);

   if (d->mediaObject) {
      d->mediaObject->unbind(this);
   }

   delete d_ptr;
}

QMediaObject *QMediaPlaylist::mediaObject() const
{
   return d_func()->mediaObject;
}

bool QMediaPlaylist::setMediaObject(QMediaObject *mediaObject)
{
   Q_D(QMediaPlaylist);

   if (mediaObject && mediaObject == d->mediaObject) {
      return true;
   }

   QMediaService *service = mediaObject ? mediaObject->service() : nullptr;
   QMediaPlaylistControl *newControl = nullptr;

   if (service) {
      newControl = dynamic_cast<QMediaPlaylistControl *>(service->requestControl(QMediaPlaylistControl_iid));
   }

   if (! newControl) {
      newControl = d->networkPlaylistControl;
   }

   if (d->control != newControl) {
      int removedStart  = -1;
      int removedEnd    = -1;
      int insertedStart = -1;
      int insertedEnd   = -1;

      if (d->control) {
         QMediaPlaylistProvider *playlist = d->control->playlistProvider();

         disconnect(playlist,   &QMediaPlaylistProvider::loadFailed,             this, &QMediaPlaylist::_q_loadFailed);
         disconnect(playlist,   &QMediaPlaylistProvider::mediaChanged,           this, &QMediaPlaylist::mediaChanged);
         disconnect(playlist,   &QMediaPlaylistProvider::mediaAboutToBeInserted, this, &QMediaPlaylist::mediaAboutToBeInserted);
         disconnect(playlist,   &QMediaPlaylistProvider::mediaInserted,          this, &QMediaPlaylist::mediaInserted);
         disconnect(playlist,   &QMediaPlaylistProvider::mediaAboutToBeRemoved,  this, &QMediaPlaylist::mediaAboutToBeRemoved);
         disconnect(playlist,   &QMediaPlaylistProvider::mediaRemoved,           this, &QMediaPlaylist::mediaRemoved);
         disconnect(playlist,   &QMediaPlaylistProvider::loaded,                 this, &QMediaPlaylist::loaded);

         disconnect(d->control, &QMediaPlaylistControl::playbackModeChanged,     this, &QMediaPlaylist::playbackModeChanged);
         disconnect(d->control, &QMediaPlaylistControl::currentIndexChanged,     this, &QMediaPlaylist::currentIndexChanged);
         disconnect(d->control, &QMediaPlaylistControl::currentMediaChanged,     this, &QMediaPlaylist::currentMediaChanged);

         // Copy playlist items, sync playback mode and sync current index between old control and new control
         d->syncControls(d->control, newControl, &removedStart, &removedEnd, &insertedStart, &insertedEnd);

         if (d->mediaObject) {
            d->mediaObject->service()->releaseControl(d->control);
         }
      }

      d->control = newControl;
      QMediaPlaylistProvider *playlist = d->control->playlistProvider();

      connect(playlist,   &QMediaPlaylistProvider::loadFailed,             this, &QMediaPlaylist::_q_loadFailed);
      connect(playlist,   &QMediaPlaylistProvider::mediaChanged,           this, &QMediaPlaylist::mediaChanged);
      connect(playlist,   &QMediaPlaylistProvider::mediaAboutToBeInserted, this, &QMediaPlaylist::mediaAboutToBeInserted);
      connect(playlist,   &QMediaPlaylistProvider::mediaInserted,          this, &QMediaPlaylist::mediaInserted);
      connect(playlist,   &QMediaPlaylistProvider::mediaAboutToBeRemoved,  this, &QMediaPlaylist::mediaAboutToBeRemoved);
      connect(playlist,   &QMediaPlaylistProvider::mediaRemoved,           this, &QMediaPlaylist::mediaRemoved);
      connect(playlist,   &QMediaPlaylistProvider::loaded,                  this, &QMediaPlaylist::loaded);

      connect(d->control, &QMediaPlaylistControl::playbackModeChanged,     this, &QMediaPlaylist::playbackModeChanged);
      connect(d->control, &QMediaPlaylistControl::currentIndexChanged,     this, &QMediaPlaylist::currentIndexChanged);
      connect(d->control, &QMediaPlaylistControl::currentMediaChanged,     this, &QMediaPlaylist::currentMediaChanged);

      if (removedStart != -1 && removedEnd != -1) {
         emit mediaAboutToBeRemoved(removedStart, removedEnd);
         emit mediaRemoved(removedStart, removedEnd);
      }

      if (insertedStart != -1 && insertedEnd != -1) {
         emit mediaAboutToBeInserted(insertedStart, insertedEnd);
         emit mediaInserted(insertedStart, insertedEnd);
      }
   }

   d->mediaObject = mediaObject;

   return true;
}

QMediaPlaylist::PlaybackMode QMediaPlaylist::playbackMode() const
{
   return d_func()->control->playbackMode();
}

void QMediaPlaylist::setPlaybackMode(QMediaPlaylist::PlaybackMode mode)
{
   Q_D(QMediaPlaylist);
   d->control->setPlaybackMode(mode);
}

int QMediaPlaylist::currentIndex() const
{
   return d_func()->control->currentIndex();
}

QMediaContent QMediaPlaylist::currentMedia() const
{
   return d_func()->playlist()->media(currentIndex());
}

int QMediaPlaylist::nextIndex(int steps) const
{
   return d_func()->control->nextIndex(steps);
}

int QMediaPlaylist::previousIndex(int steps) const
{
   return d_func()->control->previousIndex(steps);
}

int QMediaPlaylist::mediaCount() const
{
   return d_func()->playlist()->mediaCount();
}

bool QMediaPlaylist::isEmpty() const
{
   return mediaCount() == 0;
}

bool QMediaPlaylist::isReadOnly() const
{
   return d_func()->playlist()->isReadOnly();
}

QMediaContent QMediaPlaylist::media(int index) const
{
   return d_func()->playlist()->media(index);
}

bool QMediaPlaylist::addMedia(const QMediaContent &content)
{
   return d_func()->control->playlistProvider()->addMedia(content);
}

bool QMediaPlaylist::addMedia(const QList<QMediaContent> &items)
{
   return d_func()->control->playlistProvider()->addMedia(items);
}

bool QMediaPlaylist::insertMedia(int pos, const QMediaContent &content)
{
   QMediaPlaylistProvider *playlist = d_func()->playlist();
   return playlist->insertMedia(qBound(0, pos, playlist->mediaCount()), content);
}

bool QMediaPlaylist::insertMedia(int pos, const QList<QMediaContent> &items)
{
   QMediaPlaylistProvider *playlist = d_func()->playlist();
   return playlist->insertMedia(qBound(0, pos, playlist->mediaCount()), items);
}

bool QMediaPlaylist::removeMedia(int pos)
{
   QMediaPlaylistProvider *playlist = d_func()->playlist();
   if (pos >= 0 && pos < playlist->mediaCount()) {
      return playlist->removeMedia(pos);
   } else {
      return false;
   }
}

bool QMediaPlaylist::removeMedia(int start, int end)
{
   QMediaPlaylistProvider *playlist = d_func()->playlist();
   start = qMax(0, start);
   end = qMin(end, playlist->mediaCount() - 1);
   if (start <= end) {
      return playlist->removeMedia(start, end);
   } else {
      return false;
   }
}

bool QMediaPlaylist::clear()
{
   Q_D(QMediaPlaylist);
   return d->playlist()->clear();
}

bool QMediaPlaylistPrivate::readItems(QMediaPlaylistReader *reader)
{
   QList<QMediaContent> items;

   while (!reader->atEnd()) {
      items.append(reader->readItem());
   }

   return playlist()->addMedia(items);
}

bool QMediaPlaylistPrivate::writeItems(QMediaPlaylistWriter *writer)
{
   for (int i = 0; i < playlist()->mediaCount(); i++) {
      if (!writer->writeItem(playlist()->media(i))) {
         return false;
      }
   }
   writer->close();
   return true;
}

void QMediaPlaylistPrivate::syncControls(QMediaPlaylistControl *oldControl, QMediaPlaylistControl *newControl,
   int *removedStart, int *removedEnd, int *insertedStart, int *insertedEnd)
{
   Q_ASSERT(oldControl != nullptr && newControl != nullptr);
   Q_ASSERT(removedStart != nullptr && removedEnd != nullptr
      && insertedStart != nullptr && insertedEnd != nullptr);

   QMediaPlaylistProvider *oldPlaylist = oldControl->playlistProvider();
   QMediaPlaylistProvider *newPlaylist = newControl->playlistProvider();

   Q_ASSERT(oldPlaylist != nullptr && newPlaylist != NULL);

   *removedStart  = -1;
   *removedEnd    = -1;
   *insertedStart = -1;
   *insertedEnd   = -1;

   if (newPlaylist->isReadOnly()) {
      // we can't transfer the items from the old control.
      // Report these items as removed.
      if (oldPlaylist->mediaCount() > 0) {
         *removedStart = 0;
         *removedEnd = oldPlaylist->mediaCount() - 1;
      }
      // The new control might have some items that can't be cleared.
      // Report these as inserted.
      if (newPlaylist->mediaCount() > 0) {
         *insertedStart = 0;
         *insertedEnd = newPlaylist->mediaCount() - 1;
      }

   } else {
      const int oldPlaylistSize = oldPlaylist->mediaCount();

      newPlaylist->clear();
      for (int i = 0; i < oldPlaylistSize; ++i) {
         newPlaylist->addMedia(oldPlaylist->media(i));
      }
   }

   newControl->setPlaybackMode(oldControl->playbackMode());
   newControl->setCurrentIndex(oldControl->currentIndex());
}

void QMediaPlaylist::load(const QNetworkRequest &request, const char *format)
{
   Q_D(QMediaPlaylist);

   d->error = NoError;
   d->errorString.clear();

   if (d->playlist()->load(request, format)) {
      return;
   }

   if (isReadOnly()) {
      d->error = AccessDeniedError;
      d->errorString = tr("Could not add items to read only playlist.");
      emit loadFailed();

      return;
   }

   QFactoryLoader *factoryObj = loader();

   // what keys are available
   const QSet<QString> keySet = factoryObj->keySet();

   for (QString const &key : keySet) {
      QMediaPlaylistIOInterface *plugin = dynamic_cast<QMediaPlaylistIOInterface *>(factoryObj->instance(key));

      if (plugin && plugin->canRead(request.url(), format)) {
         QMediaPlaylistReader *reader = plugin->createReader(request.url(), QByteArray(format));

         if (reader && d->readItems(reader)) {
            delete reader;
            emit loaded();
            return;
         }

         delete reader;
      }
   }

   d->error = FormatNotSupportedError;
   d->errorString = tr("Playlist format is not supported");
   emit loadFailed();

   return;
}

void QMediaPlaylist::load(const QUrl &location, const char *format)
{
   load(QNetworkRequest(location), format);
}

void QMediaPlaylist::load(QIODevice *device, const char *format)
{
   Q_D(QMediaPlaylist);

   d->error = NoError;
   d->errorString.clear();

   if (d->playlist()->load(device, format)) {
      return;
   }

   if (isReadOnly()) {
      d->error = AccessDeniedError;
      d->errorString = tr("Could not add items to read only playlist.");
      emit loadFailed();
      return;
   }

   QFactoryLoader *factoryObj = loader();

   // what keys are available
   const QSet<QString> keySet = factoryObj->keySet();

   for (QString const &key : keySet) {
      QMediaPlaylistIOInterface *plugin = dynamic_cast<QMediaPlaylistIOInterface *>(factoryObj->instance(key));

      if (plugin && plugin->canRead(device, format)) {
         QMediaPlaylistReader *reader = plugin->createReader(device, QByteArray(format));

         if (reader && d->readItems(reader)) {
            delete reader;
            emit loaded();
            return;
         }
         delete reader;
      }
   }

   d->error = FormatNotSupportedError;
   d->errorString = tr("Playlist format is not supported");
   emit loadFailed();

   return;
}

bool QMediaPlaylist::save(const QUrl &location, const char *format)
{
   Q_D(QMediaPlaylist);

   d->error = NoError;
   d->errorString.clear();

   if (d->playlist()->save(location, format)) {
      return true;
   }

   QFile file(location.toLocalFile());

   if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      d->error = AccessDeniedError;
      d->errorString = tr("The file could not be accessed.");
      return false;
   }

   return save(&file, format);
}

bool QMediaPlaylist::save(QIODevice *device, const char *format)
{
   Q_D(QMediaPlaylist);

   d->error = NoError;
   d->errorString.clear();

   if (d->playlist()->save(device, format)) {
      return true;
   }

   QFactoryLoader *factoryObj = loader();

   // what keys are available
   const QSet<QString> keySet = factoryObj->keySet();

   for (QString const &key : keySet) {
      QMediaPlaylistIOInterface *plugin = dynamic_cast<QMediaPlaylistIOInterface *>(factoryObj->instance(key));

      if (plugin && plugin->canWrite(device, format)) {
         QMediaPlaylistWriter *writer = plugin->createWriter(device, QByteArray(format));

         if (writer && d->writeItems(writer)) {
            delete writer;
            return true;
         }
         delete writer;
      }
   }

   d->error = FormatNotSupportedError;
   d->errorString = tr("Playlist format is not supported.");

   return false;
}

QMediaPlaylist::Error QMediaPlaylist::error() const
{
   return d_func()->error;
}

QString QMediaPlaylist::errorString() const
{
   return d_func()->errorString;
}

void QMediaPlaylist::shuffle()
{
   d_func()->playlist()->shuffle();
}

void QMediaPlaylist::next()
{
   d_func()->control->next();
}

void QMediaPlaylist::previous()
{
   d_func()->control->previous();
}

void QMediaPlaylist::setCurrentIndex(int playlistPosition)
{
   d_func()->control->setCurrentIndex(playlistPosition);
}

void QMediaPlaylist::_q_loadFailed(QMediaPlaylist::Error playlistError, const QString &errorMsg)
{
   Q_D(QMediaPlaylist);
   d->_q_loadFailed(playlistError, errorMsg);
}
