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

#include <qmediaplayer.h>

#include <qaudiorolecontrol.h>
#include <qmediaservice.h>
#include <qmediaplayercontrol.h>
#include <qmediaplaylist.h>

#include <qmediaobject_p.h>
#include <qmediaserviceprovider_p.h>
#include <qvideosurfaceoutput_p.h>
#include <qmediaplaylistcontrol_p.h>
#include <qmediaplaylistsourcecontrol_p.h>
#include <qmedianetworkaccesscontrol.h>

#include <qvideowidget.h>
#include <qgraphicsvideoitem.h>

#include <qcoreevent.h>
#include <qmetaobject.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qpointer.h>
#include <qfileinfo.h>
#include <qtemporaryfile.h>

static int qRegisterMediaPlayerMetaTypes()
{
   qRegisterMetaType<QMediaPlayer::State>("QMediaPlayer::State");
   qRegisterMetaType<QMediaPlayer::MediaStatus>("QMediaPlayer::MediaStatus");
   qRegisterMetaType<QMediaPlayer::Error>("QMediaPlayer::Error");

   return 0;
}

Q_CONSTRUCTOR_FUNCTION(qRegisterMediaPlayerMetaTypes)

#define MAX_NESTED_PLAYLISTS 16

class QMediaPlayerPrivate : public QMediaObjectPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QMediaPlayer)

 public:
   QMediaPlayerPrivate()
      : provider(0)
      , control(0)
      , audioRoleControl(0)
      , playlist(0)
      , networkAccessControl(0)
      , state(QMediaPlayer::StoppedState)
      , status(QMediaPlayer::UnknownMediaStatus)
      , error(QMediaPlayer::NoError)
      , ignoreNextStatusChange(-1)
      , nestedPlaylists(0)
      , hasStreamPlaybackFeature(false)
   {}

   QMediaServiceProvider *provider;
   QMediaPlayerControl *control;
   QAudioRoleControl *audioRoleControl;
   QString errorString;

   QPointer<QObject> videoOutput;
   QMediaPlaylist *playlist;
   QMediaNetworkAccessControl *networkAccessControl;
   QVideoSurfaceOutput surfaceOutput;
   QMediaContent qrcMedia;
   QScopedPointer<QFile> qrcFile;

   QMediaContent rootMedia;
   QMediaContent pendingPlaylist;
   QMediaPlayer::State state;
   QMediaPlayer::MediaStatus status;
   QMediaPlayer::Error error;
   int ignoreNextStatusChange;
   int nestedPlaylists;
   bool hasStreamPlaybackFeature;

   QMediaPlaylist *parentPlaylist(QMediaPlaylist *pls);
   bool isInChain(QUrl url);

   void setMedia(const QMediaContent &media, QIODevice *stream = 0);

   void setPlaylist(QMediaPlaylist *playlist);
   void setPlaylistMedia();
   void loadPlaylist();
   void disconnectPlaylist();
   void connectPlaylist();

   void _q_stateChanged(QMediaPlayer::State state);
   void _q_mediaStatusChanged(QMediaPlayer::MediaStatus status);
   void _q_error(int error, const QString &errorString);
   void _q_updateMedia(const QMediaContent &);
   void _q_playlistDestroyed();
   void _q_handleMediaChanged(const QMediaContent &);
   void _q_handlePlaylistLoaded();
   void _q_handlePlaylistLoadFailed();
};

QMediaPlaylist *QMediaPlayerPrivate::parentPlaylist(QMediaPlaylist *pls)
{
   // This function finds a parent playlist for an item in the active chain of playlists.
   // Every item in the chain comes from currentMedia() of its parent.
   // We don't need to travers the whole tree of playlists,
   // but only the subtree of active ones.
   for (QMediaPlaylist *current = rootMedia.playlist(); current && current != pls; current = current->currentMedia().playlist())
      if (current->currentMedia().playlist() == pls) {
         return current;
      }
   return 0;
}

bool QMediaPlayerPrivate::isInChain(QUrl url)
{
   // Check whether a URL is already in the chain of playlists.
   // Also see a comment in parentPlaylist().
   for (QMediaPlaylist *current = rootMedia.playlist(); current && current != playlist; current = current->currentMedia().playlist())
      if (current->currentMedia().canonicalUrl() == url) {
         return true;
      }
   return false;
}

void QMediaPlayerPrivate::_q_stateChanged(QMediaPlayer::State ps)
{
   Q_Q(QMediaPlayer);

   // Backend switches into stopped state every time new media is about to be loaded.
   // If media player has a playlist loaded make sure player does not stop.

   if (playlist && playlist->currentIndex() != -1 && ps != state && ps == QMediaPlayer::StoppedState) {
      if (control->mediaStatus() == QMediaPlayer::EndOfMedia ||
         control->mediaStatus() == QMediaPlayer::InvalidMedia) {
         // if media player is not stopped, and
         // we have finished playback for the current media,
         // advance to the next item in the playlist

         Q_ASSERT(state != QMediaPlayer::StoppedState);
         playlist->next();
         return;

      } else if (control->mediaStatus() == QMediaPlayer::LoadingMedia) {
         return;
      }
   }

   if (ps != state) {
      state = ps;

      if (ps == QMediaPlayer::PlayingState) {
         q->addPropertyWatch<qint64>("position");

      } else {
         q->removePropertyWatch("position");
      }

      emit q->stateChanged(ps);
   }
}

void QMediaPlayerPrivate::_q_mediaStatusChanged(QMediaPlayer::MediaStatus s)
{
   Q_Q(QMediaPlayer);

   if (int(s) == ignoreNextStatusChange) {
      ignoreNextStatusChange = -1;
      return;
   }

   if (s != status) {
      status = s;

      switch (s) {
         case QMediaPlayer::StalledMedia:
         case QMediaPlayer::BufferingMedia:
            q->addPropertyWatch<int>("bufferStatus");
            break;

         default:
            q->removePropertyWatch("bufferStatus");
            break;
      }

      emit q->mediaStatusChanged(s);
   }
}

void QMediaPlayerPrivate::_q_error(int error, const QString &errorString)
{
   Q_Q(QMediaPlayer);

   if (error == int(QMediaPlayer::MediaIsPlaylist)) {
      loadPlaylist();

   } else {
      this->error = QMediaPlayer::Error(error);
      this->errorString = errorString;
      emit q->error(this->error);

      if (playlist) {
         playlist->next();
      }
   }
}

void QMediaPlayerPrivate::_q_updateMedia(const QMediaContent &media)
{
   Q_Q(QMediaPlayer);

   if (!control) {
      return;
   }

   // check if the current playlist is a top-level playlist
   Q_ASSERT(playlist);
   if (media.isNull() && playlist != rootMedia.playlist()) {
      // switch back to parent playlist
      QMediaPlaylist *pls = parentPlaylist(playlist);
      Q_ASSERT(pls);
      disconnectPlaylist();
      playlist = pls;
      connectPlaylist();

      Q_ASSERT(!pendingPlaylist.playlist());
      nestedPlaylists--;
      Q_ASSERT(nestedPlaylists >= 0);

      playlist->next();
      return;
   }

   if (media.playlist()) {
      if (nestedPlaylists < MAX_NESTED_PLAYLISTS) {
         nestedPlaylists++;
         Q_ASSERT(!pendingPlaylist.playlist());

         // disconnect current playlist
         disconnectPlaylist();
         // new playlist signals are connected
         // in the call to setPlaylist() in _q_handlePlaylistLoaded()
         playlist = media.playlist();
         emit q->currentMediaChanged(media);
         _q_handlePlaylistLoaded();
         return;
      } else if (playlist) {
         playlist->next();
      }
      return;
   }

   const QMediaPlayer::State currentState = state;

   setMedia(media, 0);

   if (!media.isNull()) {
      switch (currentState) {
         case QMediaPlayer::PlayingState:
            control->play();
            break;
         case QMediaPlayer::PausedState:
            control->pause();
            break;
         default:
            break;
      }
   }

   _q_stateChanged(control->state());
}

void QMediaPlayerPrivate::_q_playlistDestroyed()
{
   playlist = 0;
   setMedia(QMediaContent(), 0);
}

void QMediaPlayerPrivate::setMedia(const QMediaContent &media, QIODevice *stream)
{
   Q_Q(QMediaPlayer);

   if (!control) {
      return;
   }

   QScopedPointer<QFile> file;

   // Backends can't play qrc files directly.
   // If the backend supports StreamPlayback, we pass a QFile for that resource.
   // If it doesn't, we copy the data to a temporary file and pass its path.
   if (!media.isNull() && !stream && media.canonicalUrl().scheme() == QLatin1String("qrc")) {
      qrcMedia = media;

      file.reset(new QFile(QLatin1Char(':') + media.canonicalUrl().path()));
      if (!file->open(QFile::ReadOnly)) {
         QMetaObject::invokeMethod(q, "_q_error", Qt::QueuedConnection,
            Q_ARG(int, QMediaPlayer::ResourceError),
            Q_ARG(QString, QMediaPlayer::tr("Attempting to play invalid resource")));

         QMetaObject::invokeMethod(q, "_q_mediaStatusChanged", Qt::QueuedConnection,
            Q_ARG(QMediaPlayer::MediaStatus, QMediaPlayer::InvalidMedia));

         file.reset();
         // Ignore the next NoMedia status change, we just want to clear the current media
         // on the backend side since we can't load the new one and we want to be in the
         // InvalidMedia status.
         ignoreNextStatusChange = QMediaPlayer::NoMedia;
         control->setMedia(QMediaContent(), 0);

      } else if (hasStreamPlaybackFeature) {
         control->setMedia(media, file.data());
      } else {
         QTemporaryFile *tempFile = new QTemporaryFile;

         // Preserve original file extension, some backends might not load the file if it doesn't
         // have an extension.
         const QString suffix = QFileInfo(*file).suffix();
         if (!suffix.isEmpty()) {
            tempFile->setFileTemplate(tempFile->fileTemplate() + QLatin1Char('.') + suffix);
         }

         // Copy the qrc data into the temporary file
         tempFile->open();
         char buffer[4096];
         while (true) {
            qint64 len = file->read(buffer, sizeof(buffer));
            if (len < 1) {
               break;
            }
            tempFile->write(buffer, len);
         }
         tempFile->close();

         file.reset(tempFile);
         control->setMedia(QMediaContent(QUrl::fromLocalFile(file->fileName())), 0);
      }
   } else {
      qrcMedia = QMediaContent();
      control->setMedia(media, stream);
   }

   qrcFile.swap(file); // Cleans up any previous file
}

void QMediaPlayerPrivate::_q_handleMediaChanged(const QMediaContent &media)
{
   Q_Q(QMediaPlayer);

   emit q->currentMediaChanged(qrcMedia.isNull() ? media : qrcMedia);
}

void QMediaPlayerPrivate::setPlaylist(QMediaPlaylist *pls)
{
   disconnectPlaylist();
   playlist = pls;

   setPlaylistMedia();
}

void QMediaPlayerPrivate::setPlaylistMedia()
{
   // This function loads current playlist media into backend.
   // If current media is a playlist, the function recursively
   // loads media from the playlist.
   // It also makes sure the correct playlist signals are connected.
   Q_Q(QMediaPlayer);

   if (playlist) {
      connectPlaylist();
      if (playlist->currentMedia().playlist()) {
         if (nestedPlaylists < MAX_NESTED_PLAYLISTS) {
            emit q->currentMediaChanged(playlist->currentMedia());
            // rewind nested playlist to start
            playlist->currentMedia().playlist()->setCurrentIndex(0);
            nestedPlaylists++;
            setPlaylist(playlist->currentMedia().playlist());
         } else {
            playlist->next();
         }
         return;
      } else {
         // If we've just switched to a new playlist,
         // then last emitted currentMediaChanged was a playlist.
         // Make sure we emit currentMediaChanged if new playlist has
         // the same media as the previous one:
         // sample.m3u
         //      test.wav     -- processed by backend
         //      nested.m3u   -- processed by frontend
         //          test.wav -- processed by backend,
         //                      media is not changed,
         //                      frontend needs to emit currentMediaChanged
         bool isSameMedia = (q->currentMedia() == playlist->currentMedia());
         setMedia(playlist->currentMedia(), 0);
         if (isSameMedia) {
            emit q->currentMediaChanged(q->currentMedia());
         }
      }
   } else {
      setMedia(QMediaContent(), 0);
   }
}

void QMediaPlayerPrivate::loadPlaylist()
{
   Q_Q(QMediaPlayer);
   Q_ASSERT(pendingPlaylist.isNull());

   // Do not load a playlist if there are more than MAX_NESTED_PLAYLISTS in the chain already,
   // or if the playlist URL is already in the chain, i.e. do not allow recursive playlists and loops.

   if (nestedPlaylists < MAX_NESTED_PLAYLISTS && !q->currentMedia().canonicalUrl().isEmpty()
      && ! isInChain(q->currentMedia().canonicalUrl())) {

      pendingPlaylist = QMediaContent(new QMediaPlaylist, q->currentMedia().canonicalUrl(), true);
      QObject::connect(pendingPlaylist.playlist(), SIGNAL(loaded()),     q, SLOT(_q_handlePlaylistLoaded()));
      QObject::connect(pendingPlaylist.playlist(), SIGNAL(loadFailed()), q, SLOT(_q_handlePlaylistLoadFailed()));

      pendingPlaylist.playlist()->load(pendingPlaylist.canonicalRequest());
   } else if (playlist) {
      playlist->next();
   }
}

void QMediaPlayerPrivate::disconnectPlaylist()
{
   Q_Q(QMediaPlayer);

   if (playlist) {
      QObject::disconnect(playlist, SIGNAL(currentMediaChanged(QMediaContent)),
         q, SLOT(_q_updateMedia(QMediaContent)));

      QObject::disconnect(playlist, SIGNAL(destroyed()), q, SLOT(_q_playlistDestroyed()));
      q->unbind(playlist);
   }
}

void QMediaPlayerPrivate::connectPlaylist()
{
   Q_Q(QMediaPlayer);
   if (playlist) {
      q->bind(playlist);
      QObject::connect(playlist, SIGNAL(currentMediaChanged(QMediaContent)),
         q, SLOT(_q_updateMedia(QMediaContent)));

      QObject::connect(playlist, SIGNAL(destroyed()), q, SLOT(_q_playlistDestroyed()));
   }
}

void QMediaPlayerPrivate::_q_handlePlaylistLoaded()
{
   Q_Q(QMediaPlayer);

   if (pendingPlaylist.playlist()) {
      Q_ASSERT(!q->currentMedia().playlist());

      // if there is an active playlist
      if (playlist) {
         Q_ASSERT(playlist->currentIndex() >= 0);
         disconnectPlaylist();
         playlist->insertMedia(playlist->currentIndex() + 1, pendingPlaylist);
         playlist->removeMedia(playlist->currentIndex());
         nestedPlaylists++;
      } else {
         Q_ASSERT(!rootMedia.playlist());
         rootMedia = pendingPlaylist;
         emit q->mediaChanged(rootMedia);
      }

      playlist = pendingPlaylist.playlist();
      emit q->currentMediaChanged(pendingPlaylist);
   }
   pendingPlaylist = QMediaContent();

   playlist->next();
   setPlaylistMedia();

   switch (state) {
      case QMediaPlayer::PausedState:
         control->pause();
         break;
      case QMediaPlayer::PlayingState:
         control->play();
         break;
      case QMediaPlayer::StoppedState:
         break;
   }
}

void QMediaPlayerPrivate::_q_handlePlaylistLoadFailed()
{
   pendingPlaylist = QMediaContent();

   if (!control) {
      return;
   }

   if (playlist) {
      playlist->next();
   } else {
      setMedia(QMediaContent(), 0);
   }
}

static QMediaService *playerService(QMediaPlayer::Flags flags)
{
   QMediaServiceProvider *provider = QMediaServiceProvider::defaultServiceProvider();

   if (flags) {
      QMediaServiceProviderHint::Features features = 0;

      if (flags & QMediaPlayer::LowLatency) {
         features |= QMediaServiceProviderHint::LowLatencyPlayback;
      }

      if (flags & QMediaPlayer::StreamPlayback) {
         features |= QMediaServiceProviderHint::StreamPlayback;
      }

      if (flags & QMediaPlayer::VideoSurface) {
         features |= QMediaServiceProviderHint::VideoSurface;
      }

      return provider->requestService(QMediaPlayerControl_Key, QMediaServiceProviderHint(features));

   } else {
      return provider->requestService(QMediaPlayerControl_Key);
   }
}

QMediaPlayer::QMediaPlayer(QObject *parent, QMediaPlayer::Flags flags)
   : QMediaObject(*new QMediaPlayerPrivate, parent, playerService(flags))
{
   Q_D(QMediaPlayer);

   d->provider = QMediaServiceProvider::defaultServiceProvider();

   if (d->service == nullptr) {
      d->error = ServiceMissingError;

   } else {
      d->control = dynamic_cast<QMediaPlayerControl *>(d->service->requestControl(QMediaPlayerControl_Key));
      d->networkAccessControl = dynamic_cast<QMediaNetworkAccessControl *>(d->service->requestControl(QMediaNetworkAccessControl_iid));

      if (d->control != 0) {
         connect(d->control, &QMediaPlayerControl::mediaChanged,           this, &QMediaPlayer::_q_handleMediaChanged);
         connect(d->control, &QMediaPlayerControl::stateChanged,           this, &QMediaPlayer::_q_stateChanged);
         connect(d->control, &QMediaPlayerControl::mediaStatusChanged,     this, &QMediaPlayer::_q_mediaStatusChanged);
         connect(d->control, &QMediaPlayerControl::error,                  this, &QMediaPlayer::_q_error);

         connect(d->control, &QMediaPlayerControl::durationChanged,        this, &QMediaPlayer::durationChanged);
         connect(d->control, &QMediaPlayerControl::positionChanged,        this, &QMediaPlayer::positionChanged);
         connect(d->control, &QMediaPlayerControl::audioAvailableChanged,  this, &QMediaPlayer::audioAvailableChanged);
         connect(d->control, &QMediaPlayerControl::videoAvailableChanged,  this, &QMediaPlayer::videoAvailableChanged);
         connect(d->control, &QMediaPlayerControl::volumeChanged,          this, &QMediaPlayer::volumeChanged);
         connect(d->control, &QMediaPlayerControl::mutedChanged,           this, &QMediaPlayer::mutedChanged);
         connect(d->control, &QMediaPlayerControl::seekableChanged,        this, &QMediaPlayer::seekableChanged);
         connect(d->control, &QMediaPlayerControl::playbackRateChanged,    this, &QMediaPlayer::playbackRateChanged);
         connect(d->control, &QMediaPlayerControl::bufferStatusChanged,    this, &QMediaPlayer::bufferStatusChanged);

         d->state = d->control->state();
         d->status = d->control->mediaStatus();

         if (d->state == PlayingState) {
            addPropertyWatch<qint64>("position");
         }

         if (d->status == StalledMedia || d->status == BufferingMedia) {
            addPropertyWatch<int>("bufferStatus");
         }

         d->hasStreamPlaybackFeature = d->provider->supportedFeatures(d->service).testFlag(QMediaServiceProviderHint::StreamPlayback);

         d->audioRoleControl = qobject_cast<QAudioRoleControl *>(d->service->requestControl(QAudioRoleControl_iid));
         if (d->audioRoleControl) {
            connect(d->audioRoleControl, &QAudioRoleControl::audioRoleChanged, this, &QMediaPlayer::audioRoleChanged);
         }
      }
      if (d->networkAccessControl != 0) {
         connect(d->networkAccessControl, SIGNAL(configurationChanged(QNetworkConfiguration)),
            this, SLOT(networkConfigurationChanged(QNetworkConfiguration)));
      }
   }
}

QMediaPlayer::~QMediaPlayer()
{
   Q_D(QMediaPlayer);

   d->disconnectPlaylist();

   if (d->service) {
      if (d->control) {
         d->service->releaseControl(d->control);
      }
      if (d->audioRoleControl) {
         d->service->releaseControl(d->audioRoleControl);
      }

      d->provider->releaseService(d->service);
   }
}

QMediaContent QMediaPlayer::media() const
{
   Q_D(const QMediaPlayer);

   return d->rootMedia;
}

const QIODevice *QMediaPlayer::mediaStream() const
{
   Q_D(const QMediaPlayer);

   // When playing a resource file, we might have passed a QFile to the backend. Hide it from the user.
   if (d->control && d->qrcMedia.isNull()) {
      return d->control->mediaStream();
   }

   return nullptr;
}

QMediaPlaylist *QMediaPlayer::playlist() const
{
   Q_D(const QMediaPlayer);

   return d->rootMedia.playlist();
}

QMediaContent QMediaPlayer::currentMedia() const
{
   Q_D(const QMediaPlayer);

   // When playing a resource file, do not return the backend's current media, which can be a temporary file.
   if (! d->qrcMedia.isNull()) {
      return d->qrcMedia;
   }

   if (d->control) {
      return d->control->media();
   }

   return QMediaContent();
}

void QMediaPlayer::setPlaylist(QMediaPlaylist *playlist)
{
   QMediaContent m(playlist, QUrl(), false);
   setMedia(m);
}

/*!
    Sets the network access points for remote media playback.
    \a configurations contains, in ascending preferential order, a list of
    configuration  that can be used for network access.

    This will invalidate the choice of previous configurations.
*/
void QMediaPlayer::setNetworkConfigurations(const QList<QNetworkConfiguration> &configurations)
{
   Q_D(QMediaPlayer);

   if (d->networkAccessControl) {
      d->networkAccessControl->setConfigurations(configurations);
   }
}

QMediaPlayer::State QMediaPlayer::state() const
{
   return d_func()->state;
}

QMediaPlayer::MediaStatus QMediaPlayer::mediaStatus() const
{
   return d_func()->status;
}

qint64 QMediaPlayer::duration() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->duration();
   }

   return -1;
}

qint64 QMediaPlayer::position() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->position();
   }

   return 0;
}

int QMediaPlayer::volume() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->volume();
   }

   return 0;
}

bool QMediaPlayer::isMuted() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->isMuted();
   }

   return false;
}

int QMediaPlayer::bufferStatus() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->bufferStatus();
   }

   return 0;
}

bool QMediaPlayer::isAudioAvailable() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->isAudioAvailable();
   }

   return false;
}

bool QMediaPlayer::isVideoAvailable() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->isVideoAvailable();
   }

   return false;
}

bool QMediaPlayer::isSeekable() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->isSeekable();
   }

   return false;
}

qreal QMediaPlayer::playbackRate() const
{
   Q_D(const QMediaPlayer);

   if (d->control != 0) {
      return d->control->playbackRate();
   }

   return 0.0;
}

QMediaPlayer::Error QMediaPlayer::error() const
{
   return d_func()->error;
}

QString QMediaPlayer::errorString() const
{
   return d_func()->errorString;
}

QNetworkConfiguration QMediaPlayer::currentNetworkConfiguration() const
{
   Q_D(const QMediaPlayer);

   if (d->networkAccessControl) {
      return d_func()->networkAccessControl->currentConfiguration();
   }

   return QNetworkConfiguration();
}

void QMediaPlayer::play()
{
   Q_D(QMediaPlayer);

   if (d->control == 0) {
      QMetaObject::invokeMethod(this, "_q_error", Qt::QueuedConnection,
         Q_ARG(int, QMediaPlayer::ServiceMissingError),
         Q_ARG(const QString &, tr("QMediaPlayer: No valid playback service found, verify plugins are installed")));
      return;
   }

   // if playlist control is available, the service should advance itself
   if (d->rootMedia.playlist() && ! d->rootMedia.playlist()->isEmpty()) {
      // switch to playing state

      if (d->state != QMediaPlayer::PlayingState)  {
         d->_q_stateChanged(QMediaPlayer::PlayingState);
      }

      if (d->rootMedia.playlist()->currentIndex() == -1) {
         if (d->playlist != d->rootMedia.playlist()) {
            d->setPlaylist(d->rootMedia.playlist());
         }

         Q_ASSERT(d->playlist == d->rootMedia.playlist());

         emit currentMediaChanged(d->rootMedia);
         d->playlist->setCurrentIndex(0);
      }
   }

   // Reset error conditions
   d->error = NoError;
   d->errorString = QString();

   d->control->play();
}

void QMediaPlayer::pause()
{
   Q_D(QMediaPlayer);

   if (d->control != 0) {
      d->control->pause();
   }
}

/*!
    Stop playing, and reset the play position to the beginning.
*/

void QMediaPlayer::stop()
{
   Q_D(QMediaPlayer);

   if (d->control != 0) {
      d->control->stop();
   }

   // If media player didn't stop in response to control.
   // This happens if we have an active playlist and control
   // media status is
   // QMediaPlayer::LoadingMedia, QMediaPlayer::InvalidMedia, or QMediaPlayer::EndOfMedia
   // see QMediaPlayerPrivate::_q_stateChanged()
   if (d->playlist && d->state != QMediaPlayer::StoppedState) {
      d->state = QMediaPlayer::StoppedState;
      removePropertyWatch("position");
      emit stateChanged(QMediaPlayer::StoppedState);
   }
}

void QMediaPlayer::setPosition(qint64 position)
{
   Q_D(QMediaPlayer);

   if (d->control == 0) {
      return;
   }

   d->control->setPosition(qMax(position, 0ll));
}

void QMediaPlayer::setVolume(int v)
{
   Q_D(QMediaPlayer);

   if (d->control == 0) {
      return;
   }

   int clamped = qBound(0, v, 100);
   if (clamped == volume()) {
      return;
   }

   d->control->setVolume(clamped);
}

void QMediaPlayer::setMuted(bool muted)
{
   Q_D(QMediaPlayer);

   if (d->control == 0 || muted == isMuted()) {
      return;
   }

   d->control->setMuted(muted);
}

void QMediaPlayer::setPlaybackRate(qreal rate)
{
   Q_D(QMediaPlayer);

   if (d->control != 0) {
      d->control->setPlaybackRate(rate);
   }
}

/*!
    Sets the current \a media source.

    If a \a stream is supplied; media data will be read from it instead of resolving the media
    source. In this case the media source may still be used to resolve additional information
    about the media such as mime type. The \a stream must be open and readable.

    Setting the media to a null QMediaContent will cause the player to discard all
    information relating to the current media source and to cease all I/O operations related
    to that media.

    \note This function returns immediately after recording the specified source of the media.
    It does not wait for the media to finish loading and does not check for errors. Listen for
    the mediaStatusChanged() and error() signals to be notified when the media is loaded and
    when an error occurs during loading.
*/

void QMediaPlayer::setMedia(const QMediaContent &media, QIODevice *stream)
{
   Q_D(QMediaPlayer);
   stop();

   QMediaContent oldMedia = d->rootMedia;
   d->disconnectPlaylist();
   d->playlist  = 0;
   d->rootMedia = media;
   d->nestedPlaylists = 0;

   if (oldMedia != media) {
      emit mediaChanged(d->rootMedia);
   }

   if (media.playlist()) {
      // reset playlist to the 1st item
      media.playlist()->setCurrentIndex(0);
      d->setPlaylist(media.playlist());
   } else {
      d->setMedia(media, stream);
   }
}

QMultimedia::SupportEstimate QMediaPlayer::hasSupport(const QString &mimeType,
   const QStringList &codecs, Flags flags)
{
   return QMediaServiceProvider::defaultServiceProvider()->hasSupport(QMediaPlayerControl_Key, mimeType, codecs, flags);
}

QStringList QMediaPlayer::supportedMimeTypes(Flags flags)
{
   return QMediaServiceProvider::defaultServiceProvider()->supportedMimeTypes(QByteArray(QMediaPlayerControl_Key), flags);
}

void QMediaPlayer::setVideoOutput(QVideoWidget *output)
{
   Q_D(QMediaPlayer);

   if (d->videoOutput) {
      unbind(d->videoOutput);
   }

   d->videoOutput = output && bind(output) ? output : nullptr;
}

void QMediaPlayer::setVideoOutput(QGraphicsVideoItem *output)
{
   Q_D(QMediaPlayer);

   if (d->videoOutput) {
      unbind(d->videoOutput);
   }

   d->videoOutput = output && bind(output) ? output : nullptr;
}

void QMediaPlayer::setVideoOutput(QAbstractVideoSurface *surface)
{
   Q_D(QMediaPlayer);

   d->surfaceOutput.setVideoSurface(surface);

   if (d->videoOutput != &d->surfaceOutput) {
      if (d->videoOutput) {
         unbind(d->videoOutput);
      }

      d->videoOutput = 0;

      if (surface && bind(&d->surfaceOutput)) {
         d->videoOutput =  &d->surfaceOutput;
      }

   }  else if (!surface) {
      //unbind the surfaceOutput if null surface is set
      unbind(&d->surfaceOutput);
      d->videoOutput = 0;
   }
}

/*! \reimp */
QMultimedia::AvailabilityStatus QMediaPlayer::availability() const
{
   Q_D(const QMediaPlayer);

   if (! d->control) {
      return QMultimedia::ServiceMissing;
   }

   return QMediaObject::availability();
}

QAudio::Role QMediaPlayer::audioRole() const
{
   Q_D(const QMediaPlayer);

   if (d->audioRoleControl != NULL) {
      return d->audioRoleControl->audioRole();
   }

   return QAudio::UnknownRole;
}

void QMediaPlayer::setAudioRole(QAudio::Role audioRole)
{
   Q_D(QMediaPlayer);

   if (d->audioRoleControl) {
      d->audioRoleControl->setAudioRole(audioRole);
   }
}

QList<QAudio::Role> QMediaPlayer::supportedAudioRoles() const
{
   Q_D(const QMediaPlayer);

   if (d->audioRoleControl) {
      return d->audioRoleControl->supportedAudioRoles();
   }

   return QList<QAudio::Role>();
}

void QMediaPlayer::_q_stateChanged(QMediaPlayer::State un_named_arg1)
{
   Q_D(QMediaPlayer);
   d->_q_stateChanged(un_named_arg1);
}

void QMediaPlayer::_q_mediaStatusChanged(QMediaPlayer::MediaStatus un_named_arg1)
{
   Q_D(QMediaPlayer);
   d->_q_mediaStatusChanged(un_named_arg1);
}

void QMediaPlayer::_q_error(int un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QMediaPlayer);
   d->_q_error(un_named_arg1, un_named_arg2);
}

void QMediaPlayer::_q_updateMedia(const QMediaContent &un_named_arg1)
{
   Q_D(QMediaPlayer);
   d->_q_updateMedia(un_named_arg1);
}

void QMediaPlayer::_q_playlistDestroyed()
{
   Q_D(QMediaPlayer);
   d->_q_playlistDestroyed();
}

void QMediaPlayer::_q_handleMediaChanged(const QMediaContent &un_named_arg1)
{
   Q_D(QMediaPlayer);
   d->_q_handleMediaChanged(un_named_arg1);
}

void QMediaPlayer::_q_handlePlaylistLoaded()
{
   Q_D(QMediaPlayer);
   d->_q_handlePlaylistLoaded();
}

void QMediaPlayer::_q_handlePlaylistLoadFailed()
{
   Q_D(QMediaPlayer);
   d->_q_handlePlaylistLoadFailed();
}

