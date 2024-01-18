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

#include <qsound.h>
#include <qsoundeffect.h>
#include <qcoreapplication.h>

void QSound::play(const QString &filename)
{
   // Object destruction is generaly handled via deleteOnComplete
   // Unexpected cases will be handled via parenting of QSound objects to qApp
   QSound *sound = new QSound(filename, qApp);
   sound->connect(sound->m_soundEffect, &QSoundEffect::playingChanged, sound, &QSound::deleteOnComplete);
   sound->play();
}

/*!
    Constructs a QSound object from the file specified by the given \a
    filename and with the given \a parent.

    \sa play()
*/
QSound::QSound(const QString &filename, QObject *parent)
   : QObject(parent)
{
   m_soundEffect = new QSoundEffect(this);
   m_soundEffect->setSource(QUrl::fromLocalFile(filename));
}

/*!
    Destroys this sound object. If the sound is not finished playing,
    the stop() function is called before the sound object is
    destroyed.

    \sa stop(), isFinished()
*/
QSound::~QSound()
{
   if (!isFinished()) {
      stop();
   }
}

/*!
    Returns true if the sound has finished playing; otherwise returns false.
*/
bool QSound::isFinished() const
{
   return !m_soundEffect->isPlaying();
}

void QSound::play()
{
   m_soundEffect->play();
}

/*!
    Returns the number of times the sound will play.
    Return value of \c QSound::Infinite indicates infinite number of loops

    \sa loopsRemaining(), setLoops()
*/
int QSound::loops() const
{
   // retain old API value for infite loops
   int loopCount = m_soundEffect->loopCount();
   if (loopCount == QSoundEffect::Infinite) {
      loopCount = Infinite;
   }

   return loopCount;
}

/*!
    Returns the remaining number of times the sound will loop (for all
    positive values this value decreases each time the sound is played).
    Return value of \c QSound::Infinite indicates infinite number of loops

    \sa loops(), isFinished()
*/
int QSound::loopsRemaining() const
{
   // retain old API value for infite loops
   int loopsRemaining = m_soundEffect->loopsRemaining();
   if (loopsRemaining == QSoundEffect::Infinite) {
      loopsRemaining = Infinite;
   }

   return loopsRemaining;
}

/*!
    \fn void QSound::setLoops(int number)

    Sets the sound to repeat the given \a number of times when it is
    played.

    Note that passing the value \c QSound::Infinite will cause the sound to loop
    indefinitely.

    \sa loops()
*/
void QSound::setLoops(int n)
{
   if (n == Infinite) {
      n = QSoundEffect::Infinite;
   }

   m_soundEffect->setLoopCount(n);
}

/*!
    Returns the filename associated with this QSound object.

    \sa QSound()
*/
QString QSound::fileName() const
{
   return m_soundEffect->source().toLocalFile();
}

/*!
    Stops the sound playing.

    \sa play()
*/
void QSound::stop()
{
   m_soundEffect->stop();
}

/*!
    \internal
*/
void QSound::deleteOnComplete()
{
   if (!m_soundEffect->isPlaying()) {
      deleteLater();
   }
}

