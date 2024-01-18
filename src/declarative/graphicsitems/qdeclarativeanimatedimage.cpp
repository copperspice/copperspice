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

#include <qdeclarativeanimatedimage_p.h>
#include <qdeclarativeanimatedimage_p_p.h>

#ifndef QT_NO_MOVIE

#include <qdeclarativeinfo.h>
#include <private/qdeclarativeengine_p.h>

#include <QMovie>
#include <QNetworkRequest>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

QDeclarativeAnimatedImage::QDeclarativeAnimatedImage(QDeclarativeItem *parent)
   : QDeclarativeImage(*(new QDeclarativeAnimatedImagePrivate), parent)
{
}

QDeclarativeAnimatedImage::~QDeclarativeAnimatedImage()
{
   Q_D(QDeclarativeAnimatedImage);
   delete d->_movie;
}

/*!
  \qmlproperty bool AnimatedImage::paused
  This property holds whether the animated image is paused.

  By default, this property is false. Set it to true when you want to pause
  the animation.
*/
bool QDeclarativeAnimatedImage::isPaused() const
{
   Q_D(const QDeclarativeAnimatedImage);
   if (!d->_movie) {
      return false;
   }
   return d->_movie->state() == QMovie::Paused;
}

void QDeclarativeAnimatedImage::setPaused(bool pause)
{
   Q_D(QDeclarativeAnimatedImage);
   if (pause == d->paused) {
      return;
   }
   d->paused = pause;
   if (!d->_movie) {
      return;
   }
   d->_movie->setPaused(pause);
}
/*!
  \qmlproperty bool AnimatedImage::playing
  This property holds whether the animated image is playing.

  By default, this property is true, meaning that the animation
  will start playing immediately.
*/
bool QDeclarativeAnimatedImage::isPlaying() const
{
   Q_D(const QDeclarativeAnimatedImage);
   if (!d->_movie) {
      return false;
   }
   return d->_movie->state() != QMovie::NotRunning;
}

void QDeclarativeAnimatedImage::setPlaying(bool play)
{
   Q_D(QDeclarativeAnimatedImage);
   if (play == d->playing) {
      return;
   }
   d->playing = play;
   if (!d->_movie) {
      return;
   }
   if (play) {
      d->_movie->start();
   } else {
      d->_movie->stop();
   }
}

/*!
  \qmlproperty int AnimatedImage::currentFrame
  \qmlproperty int AnimatedImage::frameCount

  currentFrame is the frame that is currently visible. By monitoring this property
  for changes, you can animate other items at the same time as the image.

  frameCount is the number of frames in the animation. For some animation formats,
  frameCount is unknown and has a value of zero.
*/
int QDeclarativeAnimatedImage::currentFrame() const
{
   Q_D(const QDeclarativeAnimatedImage);
   if (!d->_movie) {
      return d->preset_currentframe;
   }
   return d->_movie->currentFrameNumber();
}

void QDeclarativeAnimatedImage::setCurrentFrame(int frame)
{
   Q_D(QDeclarativeAnimatedImage);
   if (!d->_movie) {
      d->preset_currentframe = frame;
      return;
   }
   d->_movie->jumpToFrame(frame);
}

int QDeclarativeAnimatedImage::frameCount() const
{
   Q_D(const QDeclarativeAnimatedImage);
   if (!d->_movie) {
      return 0;
   }
   return d->_movie->frameCount();
}

void QDeclarativeAnimatedImage::setSource(const QUrl &url)
{
   Q_D(QDeclarativeAnimatedImage);
   if (url == d->url) {
      return;
   }

   delete d->_movie;
   d->_movie = 0;

   if (d->reply) {
      d->reply->deleteLater();
      d->reply = 0;
   }

   d->url = url;
   emit sourceChanged(d->url);

   if (isComponentComplete()) {
      load();
   }
}

void QDeclarativeAnimatedImage::load()
{
   Q_D(QDeclarativeAnimatedImage);

   QDeclarativeImageBase::Status oldStatus = d->status;
   qreal oldProgress = d->progress;

   if (d->url.isEmpty()) {
      delete d->_movie;
      d->setPixmap(QPixmap());
      d->progress = 0;
      d->status = Null;
      if (d->status != oldStatus) {
         emit statusChanged(d->status);
      }
      if (d->progress != oldProgress) {
         emit progressChanged(d->progress);
      }
   } else {
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
      QString lf = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(d->url);
      if (!lf.isEmpty()) {
         //### should be unified with movieRequestFinished
         d->_movie = new QMovie(lf);
         if (!d->_movie->isValid()) {
            qmlInfo(this) << "Error Reading Animated Image File " << d->url.toString();
            delete d->_movie;
            d->_movie = 0;
            d->status = Error;
            if (d->status != oldStatus) {
               emit statusChanged(d->status);
            }
            return;
         }
         connect(d->_movie, SIGNAL(stateChanged(QMovie::MovieState)),
                 this, SLOT(playingStatusChanged()));
         connect(d->_movie, SIGNAL(frameChanged(int)),
                 this, SLOT(movieUpdate()));
         d->_movie->setCacheMode(QMovie::CacheAll);
         if (d->playing) {
            d->_movie->start();
         } else {
            d->_movie->jumpToFrame(0);
         }
         if (d->paused) {
            d->_movie->setPaused(true);
         }
         d->setPixmap(d->_movie->currentPixmap());
         d->status = Ready;
         d->progress = 1.0;
         if (d->status != oldStatus) {
            emit statusChanged(d->status);
         }
         if (d->progress != oldProgress) {
            emit progressChanged(d->progress);
         }
         return;
      }
#endif
      d->status = Loading;
      d->progress = 0;
      emit statusChanged(d->status);
      emit progressChanged(d->progress);
      QNetworkRequest req(d->url);
      req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
      d->reply = qmlEngine(this)->networkAccessManager()->get(req);
      QObject::connect(d->reply, SIGNAL(finished()),
                       this, SLOT(movieRequestFinished()));
      QObject::connect(d->reply, SIGNAL(downloadProgress(qint64, qint64)),
                       this, SLOT(requestProgress(qint64, qint64)));
   }
}

#define ANIMATEDIMAGE_MAXIMUM_REDIRECT_RECURSION 16

void QDeclarativeAnimatedImage::movieRequestFinished()
{
   Q_D(QDeclarativeAnimatedImage);

   d->redirectCount++;
   if (d->redirectCount < ANIMATEDIMAGE_MAXIMUM_REDIRECT_RECURSION) {
      QVariant redirect = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      if (redirect.isValid()) {
         QUrl url = d->reply->url().resolved(redirect.toUrl());
         d->reply->deleteLater();
         d->reply = 0;
         setSource(url);
         return;
      }
   }
   d->redirectCount = 0;

   d->_movie = new QMovie(d->reply);

   if (! d->_movie->isValid()) {
      qmlInfo(this) << "Error Reading Animated Image File " << d->url;

      delete d->_movie;
      d->_movie = 0;
      d->status = Error;
      emit statusChanged(d->status);
      return;
   }

   connect(d->_movie, SIGNAL(stateChanged(QMovie::MovieState)), this, SLOT(playingStatusChanged()));
   connect(d->_movie, SIGNAL(frameChanged(int)), this, SLOT(movieUpdate()));
   d->_movie->setCacheMode(QMovie::CacheAll);

   if (d->playing) {
      d->_movie->start();
   }

   if (d->paused || !d->playing) {
      d->_movie->jumpToFrame(d->preset_currentframe);
      d->preset_currentframe = 0;
   }
   if (d->paused) {
      d->_movie->setPaused(true);
   }
   d->setPixmap(d->_movie->currentPixmap());
   d->status = Ready;
   emit statusChanged(d->status);
}

void QDeclarativeAnimatedImage::movieUpdate()
{
   Q_D(QDeclarativeAnimatedImage);
   d->setPixmap(d->_movie->currentPixmap());
   emit frameChanged();
}

void QDeclarativeAnimatedImage::playingStatusChanged()
{
   Q_D(QDeclarativeAnimatedImage);
   if ((d->_movie->state() != QMovie::NotRunning) != d->playing) {
      d->playing = (d->_movie->state() != QMovie::NotRunning);
      emit playingChanged();
   }
   if ((d->_movie->state() == QMovie::Paused) != d->paused) {
      d->playing = (d->_movie->state() == QMovie::Paused);
      emit pausedChanged();
   }
}

void QDeclarativeAnimatedImage::componentComplete()
{
   Q_D(QDeclarativeAnimatedImage);
   QDeclarativeItem::componentComplete(); // NOT QDeclarativeImage
   if (d->url.isValid()) {
      load();
   }
   if (!d->reply) {
      setCurrentFrame(d->preset_currentframe);
      d->preset_currentframe = 0;
   }
}

QT_END_NAMESPACE

#endif // QT_NO_MOVIE
