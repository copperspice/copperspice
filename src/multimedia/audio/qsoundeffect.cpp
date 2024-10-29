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

#include <qsoundeffect.h>

#if defined(QT_MULTIMEDIA_PULSEAUDIO)
#include <qsoundeffect_pulse_p.h>

#else
#include <qsoundeffect_qaudio_p.h>

#endif

QSoundEffect::QSoundEffect(QObject *parent)
   : QObject(parent)
{
   d = new QSoundEffectPrivate(this);

   connect(d, &QSoundEffectPrivate::loopsRemainingChanged, this, &QSoundEffect::loopsRemainingChanged);
   connect(d, &QSoundEffectPrivate::volumeChanged,         this, &QSoundEffect::volumeChanged);
   connect(d, &QSoundEffectPrivate::mutedChanged,          this, &QSoundEffect::mutedChanged);
   connect(d, &QSoundEffectPrivate::loadedChanged,         this, &QSoundEffect::loadedChanged);
   connect(d, &QSoundEffectPrivate::playingChanged,        this, &QSoundEffect::playingChanged);
   connect(d, &QSoundEffectPrivate::statusChanged,         this, &QSoundEffect::statusChanged);
   connect(d, &QSoundEffectPrivate::categoryChanged,       this, &QSoundEffect::categoryChanged);
}

QSoundEffect::~QSoundEffect()
{
   d->release();
}

QStringList QSoundEffect::supportedMimeTypes()
{
   return QSoundEffectPrivate::supportedMimeTypes();
}

QUrl QSoundEffect::source() const
{
   return d->source();
}

void QSoundEffect::setSource(const QUrl &url)
{
   if (d->source() == url) {
      return;
   }

   d->setSource(url);

   emit sourceChanged();
}

int QSoundEffect::loopCount() const
{
   return d->loopCount();
}

void QSoundEffect::setLoopCount(int loopCount)
{
   if (loopCount < 0 && loopCount != Infinite) {
      qWarning("SoundEffect: loops should be SoundEffect.Infinite, 0 or positive integer");
      return;
   }
   if (loopCount == 0) {
      loopCount = 1;
   }
   if (d->loopCount() == loopCount) {
      return;
   }

   d->setLoopCount(loopCount);
   emit loopCountChanged();
}

int QSoundEffect::loopsRemaining() const
{
   return d->loopsRemaining();
}

qreal QSoundEffect::volume() const
{
   return d->volume();
}

void QSoundEffect::setVolume(qreal volume)
{
   volume = qBound(qreal(0.0), volume, qreal(1.0));
   if (qFuzzyCompare(d->volume(), volume)) {
      return;
   }

   d->setVolume(volume);
}

bool QSoundEffect::isMuted() const
{
   return d->isMuted();
}

void QSoundEffect::setMuted(bool muted)
{
   if (d->isMuted() == muted) {
      return;
   }

   d->setMuted(muted);
}

bool QSoundEffect::isLoaded() const
{
   return d->isLoaded();
}

void QSoundEffect::play()
{
   d->play();
}

bool QSoundEffect::isPlaying() const
{
   return d->isPlaying();
}

QSoundEffect::Status QSoundEffect::status() const
{
   return d->status();
}

QString QSoundEffect::category() const
{
   return d->category();
}

void QSoundEffect::setCategory(const QString &category)
{
   d->setCategory(category);
}

void QSoundEffect::stop()
{
   d->stop();
}
