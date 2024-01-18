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

/*! Returns the URL of the current source to play */
QUrl QSoundEffect::source() const
{
   return d->source();
}

/*! Set the current URL to play to \a url. */
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

/*!
    Sets the volume to play the sound effect at to \a volume, from 0.0 (silent) to 1.0 (maximum volume).
 */
void QSoundEffect::setVolume(qreal volume)
{
   volume = qBound(qreal(0.0), volume, qreal(1.0));
   if (qFuzzyCompare(d->volume(), volume)) {
      return;
   }

   d->setVolume(volume);
}

/*!
    \qmlproperty bool QtMultimedia::SoundEffect::muted

    This property provides a way to control muting. A value of \c true will mute this effect.
    Otherwise, playback will occur with the currently specified \l volume.
*/
/*!
    \property QSoundEffect::muted

    This property provides a way to control muting. A value of \c true will mute this effect.
*/

/*! Returns whether this sound effect is muted */
bool QSoundEffect::isMuted() const
{
   return d->isMuted();
}

/*!
    Sets whether to mute this sound effect's playback.

    If \a muted is true, playback will be muted (silenced),
    and otherwise playback will occur with the currently
    specified volume().
*/
void QSoundEffect::setMuted(bool muted)
{
   if (d->isMuted() == muted) {
      return;
   }

   d->setMuted(muted);
}

/*!
    \fn QSoundEffect::isLoaded() const

    Returns whether the sound effect has finished loading the \l source().
*/
/*!
    \qmlmethod bool QtMultimedia::SoundEffect::isLoaded()

    Returns whether the sound effect has finished loading the \l source.
*/
bool QSoundEffect::isLoaded() const
{
   return d->isLoaded();
}

/*!
    \qmlmethod QtMultimedia::SoundEffect::play()

    Start playback of the sound effect, looping the effect for the number of
    times as specified in the loops property.

    This is the default method for SoundEffect.

    \snippet multimedia-snippets/soundeffect.qml play sound on click
*/
/*!
    \fn QSoundEffect::play()

    Start playback of the sound effect, looping the effect for the number of
    times as specified in the loops property.
*/
void QSoundEffect::play()
{
   d->play();
}

/*!
    \qmlproperty bool QtMultimedia::SoundEffect::playing

    This property indicates whether the sound effect is playing or not.
*/
/*!
    \property QSoundEffect::playing

    This property indicates whether the sound effect is playing or not.
*/

/*! Returns true if the sound effect is currently playing, or false otherwise */
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

/*!
    Sets the \e category of this sound effect to \a category.

    Some platforms can perform different audio routing
    for different categories, or may allow the user to
    set different volume levels for different categories.

    This setting will be ignored on platforms that do not
    support audio categories.

    If this setting is changed while a sound effect is playing
    it will only take effect when the sound effect has stopped
    playing.

    \sa category()
 */
void QSoundEffect::setCategory(const QString &category)
{
   d->setCategory(category);
}


/*!
  \qmlmethod QtMultimedia::SoundEffect::stop()

  Stop current playback.

 */
/*!
  \fn QSoundEffect::stop()

  Stop current playback.

 */
void QSoundEffect::stop()
{
   d->stop();
}
