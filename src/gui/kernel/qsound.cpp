/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qsound.h>

#ifndef QT_NO_SOUND

#include <qlist.h>
#include <qsound_p.h>

QT_BEGIN_NAMESPACE

static QList<QAuServer *> *servers = 0;

QAuServer::QAuServer(QObject *parent)
   : QObject(parent)
{
   if (!servers) {
      servers = new QList<QAuServer *>;
   }
   servers->prepend(this);
}

QAuServer::~QAuServer()
{
   servers->removeAll(this);
   if (servers->count() == 0) {
      delete servers;
      servers = 0;
   }
}

void QAuServer::play(const QString &filename)
{
   QSound s(filename);
   play(&s);
}

extern QAuServer *qt_new_audio_server();

static QAuServer &server()
{
   if (!servers) {
      qt_new_audio_server();
   }
   return *servers->first();
}

class QSoundPrivate
{
 public:
   QSoundPrivate(const QString &fname)
      : filename(fname), bucket(0), looprem(0), looptotal(1) {
   }

   virtual ~QSoundPrivate() {
      delete bucket;
   }

   QString filename;
   QAuBucket *bucket;
   int looprem;
   int looptotal;
};


void QSound::play(const QString &filename)
{
   server().play(filename);
}

/*!
    Constructs a QSound object from the file specified by the given \a
    filename and with the given \a parent.

    This may use more memory than the static play() function, but it
    may also play more immediately (depending on the underlying
    platform audio facilities).

    \sa play()
*/
QSound::QSound(const QString &filename, QObject *parent)
   : QObject(parent), d_ptr(new QSoundPrivate(filename))
{
   server().init(this);
}

/*!
    Destroys this sound object. If the sound is not finished playing,
    the stop() function is called before the sound object is
    destructed.

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

    \warning On Windows this function always returns true for unlooped sounds.
*/
bool QSound::isFinished() const
{
   Q_D(const QSound);
   return d->looprem == 0;
}

/*!
    \overload

    Starts playing the sound specified by this QSound object.

    The function returns immediately.  Depending on the platform audio
    facilities, other sounds may stop or be mixed with the new
    sound. The sound can be played again at any time, possibly mixing
    or replacing previous plays of the sound.

    \sa fileName()
*/
void QSound::play()
{
   Q_D(QSound);
   d->looprem = d->looptotal;
   server().play(this);
}

/*!
    Returns the number of times the sound will play.

    \sa loopsRemaining(), setLoops()
*/
int QSound::loops() const
{
   Q_D(const QSound);
   return d->looptotal;
}

/*!
    Returns the remaining number of times the sound will loop (this
    value decreases each time the sound is played).

    \sa loops(), isFinished()
*/
int QSound::loopsRemaining() const
{
   Q_D(const QSound);
   return d->looprem;
}

/*!
    \fn void QSound::setLoops(int number)

    Sets the sound to repeat the given \a number of times when it is
    played.

    Note that passing the value -1 will cause the sound to loop
    indefinitely.

    \sa loops()
*/
void QSound::setLoops(int n)
{
   Q_D(QSound);
   d->looptotal = n;
}

/*!
    Returns the filename associated with this QSound object.

    \sa QSound()
*/
QString QSound::fileName() const
{
   Q_D(const QSound);
   return d->filename;
}

/*!
    Stops the sound playing.

    Note that on Windows the current loop will finish if a sound is
    played in a loop.

    \sa play()
*/
void QSound::stop()
{
   Q_D(QSound);
   server().stop(this);
   d->looprem = 0;
}


/*!
    Returns true if sound facilities exist on the platform; otherwise
    returns false.

    If no sound is available, all QSound operations work silently and
    quickly. An application may choose either to notify the user if
    sound is crucial to the application or to operate silently without
    bothering the user.

    Note: On Windows this always returns true because some sound card
    drivers do not implement a way to find out whether it is available
    or not.
*/
bool QSound::isAvailable()
{
   return server().okay();
}

/*!
    Sets the internal bucket record of sound \a s to \a b, deleting
    any previous setting.
*/
void QAuServer::setBucket(QSound *s, QAuBucket *b)
{
   delete s->d_func()->bucket;
   s->d_func()->bucket = b;
}

/*!
    Returns the internal bucket record of sound \a s.
*/
QAuBucket *QAuServer::bucket(QSound *s)
{
   return s->d_func()->bucket;
}

/*!
    Decrements the QSound::loopRemaining() value for sound \a s,
    returning the result.
*/
int QAuServer::decLoop(QSound *s)
{
   if (s->d_func()->looprem > 0) {
      --s->d_func()->looprem;
   }
   return s->d_func()->looprem;
}

/*!
    Initializes the sound. The default implementation does nothing.
*/
void QAuServer::init(QSound *)
{
}

QAuBucket::~QAuBucket()
{
}
/*!
    \fn bool QSound::available()

    Use the isAvailable() function instead.
*/

QT_END_NAMESPACE

#endif // QT_NO_SOUND
