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

#include "mediaobject.h"
#include "mediaobject_p.h"
#include "factory_p.h"
#include "mediaobjectinterface.h"
#include "audiooutput.h"
#include "phonondefs_p.h"
#include "abstractmediastream.h"
#include "abstractmediastream_p.h"
#include "frontendinterface_p.h"
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QTimer>
#include "phononnamespace_p.h"
#include "platform_p.h"

#define PHONON_CLASSNAME MediaObject
#define PHONON_INTERFACENAME MediaObjectInterface

QT_BEGIN_NAMESPACE

namespace Phonon
{
PHONON_OBJECT_IMPL

MediaObject::~MediaObject()
{
    K_D(MediaObject);
    if (d->m_backendObject) {
        switch (state()) {

        case PlayingState:
        case BufferingState:
        case PausedState:
            stop();
            break;

        case ErrorState:
        case StoppedState:
        case LoadingState:
            break;
        }
    }
}

Phonon::State MediaObject::state() const
{
    K_D(const MediaObject);

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    if (d->errorOverride) {
        return d->state;
    }
    if (d->ignoreLoadingToBufferingStateChange) {
        return BufferingState;
    }
    if (d->ignoreErrorToLoadingStateChange) {
        return LoadingState;
    }
#endif

    if (!d->m_backendObject) {
        return d->state;
    }
    return INTERFACE_CALL(state());
}

PHONON_INTERFACE_SETTER(setTickInterval, tickInterval, qint32)
PHONON_INTERFACE_GETTER(qint32, tickInterval, d->tickInterval)
PHONON_INTERFACE_GETTER(bool, hasVideo, false)
PHONON_INTERFACE_GETTER(bool, isSeekable, false)
PHONON_INTERFACE_GETTER(qint64, currentTime, d->currentTime)

static inline bool isPlayable(const MediaSource::Type t)
{
    return t != MediaSource::Invalid && t != MediaSource::Empty;
}

void MediaObject::play()
{
    K_D(MediaObject);
    if (d->backendObject() && isPlayable(d->mediaSource.type())) {
        INTERFACE_CALL(play());
    }
}

void MediaObject::pause()
{
    K_D(MediaObject);
    if (d->backendObject() && isPlayable(d->mediaSource.type())) {
        INTERFACE_CALL(pause());
    }
}

void MediaObject::stop()
{
    K_D(MediaObject);
    if (d->backendObject() && isPlayable(d->mediaSource.type())) {
        INTERFACE_CALL(stop());
    }
}

void MediaObject::seek(qint64 time)
{
    K_D(MediaObject);
    if (d->backendObject() && isPlayable(d->mediaSource.type())) {
        INTERFACE_CALL(seek(time));
    }
}

QString MediaObject::errorString() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const MediaObject);
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        if (d->errorOverride) {
            return d->errorString;
        }
#endif // QT_NO_PHONON_ABSTRACTMEDIASTREAM
        return INTERFACE_CALL(errorString());
    }
    return QString();
}

ErrorType MediaObject::errorType() const
{
    if (state() == Phonon::ErrorState) {
        K_D(const MediaObject);
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        if (d->errorOverride) {
            return d->errorType;
        }
#endif // QT_NO_PHONON_ABSTRACTMEDIASTREAM
        return INTERFACE_CALL(errorType());
    }
    return Phonon::NoError;
}

QStringList MediaObject::metaData(Phonon::MetaData f) const
{
    switch (f) {
    case ArtistMetaData:
        return metaData(QLatin1String("ARTIST"     ));
    case AlbumMetaData:
        return metaData(QLatin1String("ALBUM"      ));
    case TitleMetaData:
        return metaData(QLatin1String("TITLE"      ));
    case DateMetaData:
        return metaData(QLatin1String("DATE"       ));
    case GenreMetaData:
        return metaData(QLatin1String("GENRE"      ));
    case TracknumberMetaData:
        return metaData(QLatin1String("TRACKNUMBER"));
    case DescriptionMetaData:
        return metaData(QLatin1String("DESCRIPTION"));
    case MusicBrainzDiscIdMetaData:
        return metaData(QLatin1String("MUSICBRAINZ_DISCID"));
    }
    return QStringList();
}

QStringList MediaObject::metaData(const QString &key) const
{
    K_D(const MediaObject);
    return d->metaData.values(key);
}

QMultiMap<QString, QString> MediaObject::metaData() const
{
    K_D(const MediaObject);
    return d->metaData;
}

PHONON_INTERFACE_GETTER(qint32, prefinishMark, d->prefinishMark)
PHONON_INTERFACE_SETTER(setPrefinishMark, prefinishMark, qint32)

PHONON_INTERFACE_GETTER(qint32, transitionTime, d->transitionTime)
PHONON_INTERFACE_SETTER(setTransitionTime, transitionTime, qint32)

qint64 MediaObject::totalTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject) {
        return -1;
    }
    return INTERFACE_CALL(totalTime());
}

qint64 MediaObject::remainingTime() const
{
    K_D(const MediaObject);
    if (!d->m_backendObject) {
        return -1;
    }
    qint64 ret = INTERFACE_CALL(remainingTime());
    if (ret < 0) {
        return -1;
    }
    return ret;
}

MediaSource MediaObject::currentSource() const
{
    K_D(const MediaObject);
    return d->mediaSource;
}

void MediaObject::setCurrentSource(const MediaSource &newSource)
{
    K_D(MediaObject);

    if (! k_ptr->backendObject()) {  
        d->mediaSource = newSource;
        return;
    }

    pDebug() << Q_FUNC_INFO << newSource.url();

    // first call stop as that often is the expected state for setting a new URL
    stop(); 

    d->mediaSource = newSource;

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    d->kiofallback = 0; // kiofallback auto-deletes
#endif

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    if (d->mediaSource.type() == MediaSource::Stream) {
        Q_ASSERT(d->mediaSource.stream());
        d->mediaSource.stream()->d_func()->setMediaObjectPrivate(d);
    } 
#endif 

    INTERFACE_CALL(setSource(d->mediaSource));
}

void MediaObject::clear()
{
    K_D(MediaObject);
    d->sourceQueue.clear();
    setCurrentSource(MediaSource());
}

QList<MediaSource> MediaObject::queue() const
{
    K_D(const MediaObject);
    return d->sourceQueue;
}

void MediaObject::setQueue(const QList<MediaSource> &sources)
{
    K_D(MediaObject);
    d->sourceQueue.clear();
    enqueue(sources);
}

void MediaObject::setQueue(const QList<QUrl> &urls)
{
    K_D(MediaObject);
    d->sourceQueue.clear();
    enqueue(urls);
}

void MediaObject::enqueue(const MediaSource &source)
{
    K_D(MediaObject);
    if (!isPlayable(d->mediaSource.type())) {
        // the current source is nothing valid so this source needs to become the current one
        setCurrentSource(source);
    } else {
        d->sourceQueue << source;
    }
}

void MediaObject::enqueue(const QList<MediaSource> &sources)
{
    for (int i = 0; i < sources.count(); ++i) {
        enqueue(sources.at(i));
    }
}

void MediaObject::enqueue(const QList<QUrl> &urls)
{
    for (int i = 0; i < urls.count(); ++i) {
        enqueue(urls.at(i));
    }
}

void MediaObject::clearQueue()
{
    K_D(MediaObject);
    d->sourceQueue.clear();
}

bool MediaObjectPrivate::aboutToDeleteBackendObject()
{
    //pDebug() << Q_FUNC_INFO;
    prefinishMark = pINTERFACE_CALL(prefinishMark());
    transitionTime = pINTERFACE_CALL(transitionTime());
    //pDebug() << Q_FUNC_INFO;
    if (m_backendObject) {
        state = pINTERFACE_CALL(state());
        currentTime = pINTERFACE_CALL(currentTime());
        tickInterval = pINTERFACE_CALL(tickInterval());
    }
    return true;
}

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
void MediaObjectPrivate::streamError(Phonon::ErrorType type, const QString &text)
{
    Q_Q(MediaObject);
    State lastState = q->state();
    errorOverride = true;
    errorType = type;
    errorString = text;
    state = ErrorState;

    QMetaObject::invokeMethod(q, "stateChanged", Qt::QueuedConnection, Q_ARG(Phonon::State, Phonon::ErrorState), Q_ARG(Phonon::State, lastState));   
}

void MediaObjectPrivate::_k_stateChanged(Phonon::State newstate, Phonon::State oldstate)
{
    Q_Q(MediaObject);

    if (mediaSource.type() != MediaSource::Url) {
        // special handling only necessary for URLs because of the fallback
        emit q->stateChanged(newstate, oldstate);
        return;
    }

    if (errorOverride) {
        errorOverride = false;
        if (newstate == ErrorState) {
            return;
        }
        oldstate = ErrorState;
    }

    // backend MediaObject reached ErrorState, try a KioMediaSource
    if (newstate == Phonon::ErrorState && !kiofallback) {
        kiofallback = Platform::createMediaStream(mediaSource.url(), q);
        if (!kiofallback) {
            pDebug() << "backend MediaObject reached ErrorState, no KIO fallback available";
            emit q->stateChanged(newstate, oldstate);
            return;
        }
        pDebug() << "backend MediaObject reached ErrorState, trying Platform::createMediaStream now";
        ignoreLoadingToBufferingStateChange = false;
        ignoreErrorToLoadingStateChange = false;
        switch (oldstate) {
        case Phonon::BufferingState:
            // play() has already been called, we need to make sure it is called
            // on the backend with the KioMediaStream MediaSource now, too
            ignoreLoadingToBufferingStateChange = true;
            break;
        case Phonon::LoadingState:
            ignoreErrorToLoadingStateChange = true;
            // no extras
            break;
        default:
            pError() << "backend MediaObject reached ErrorState after " << oldstate
                << ". It seems a KioMediaStream will not help here, trying anyway.";
            emit q->stateChanged(Phonon::LoadingState, oldstate);
            break;
        }
        kiofallback->d_func()->setMediaObjectPrivate(this);
        MediaSource mediaSource(kiofallback);
        mediaSource.setAutoDelete(true);
        pINTERFACE_CALL(setSource(mediaSource));
        if (oldstate == Phonon::BufferingState) {
            q->play();
        }
        return;
    } else if (ignoreLoadingToBufferingStateChange &&
            kiofallback &&
            oldstate == Phonon::LoadingState) {
        if (newstate != Phonon::BufferingState) {
            emit q->stateChanged(newstate, Phonon::BufferingState);
        }
        return;
    } else if (ignoreErrorToLoadingStateChange && kiofallback && oldstate == ErrorState) {
        if (newstate != LoadingState) {
            emit q->stateChanged(newstate, Phonon::LoadingState);
        }
        return;
    }

    emit q->stateChanged(newstate, oldstate);
}
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

void MediaObjectPrivate::_k_aboutToFinish()
{
    Q_Q(MediaObject);
    pDebug() << Q_FUNC_INFO;

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    kiofallback = 0; // kiofallback auto-deletes
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

    if (sourceQueue.isEmpty()) {
        emit q->aboutToFinish();
        if (sourceQueue.isEmpty()) {
            return;
        }
    }

    mediaSource = sourceQueue.head();
    pINTERFACE_CALL(setNextSource(mediaSource));
}

void MediaObjectPrivate::_k_currentSourceChanged(const MediaSource &source)
{
    Q_Q(MediaObject);
    pDebug() << Q_FUNC_INFO;

    if (!sourceQueue.isEmpty() && sourceQueue.head() == source)
        sourceQueue.dequeue();

    emit q->currentSourceChanged(source);
}

void MediaObjectPrivate::setupBackendObject()
{
    Q_Q(MediaObject);
    Q_ASSERT(m_backendObject);
   
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), 
           q, SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
#else
    QObject::connect(m_backendObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), 
           q, SLOT(stateChanged(Phonon::State, Phonon::State)));
#endif

    QObject::connect(m_backendObject, SIGNAL(tick(qint64)),                 q, SLOT(tick(qint64)));
    QObject::connect(m_backendObject, SIGNAL(seekableChanged(bool)),        q, SLOT(seekableChanged(bool)));

#ifndef QT_NO_PHONON_VIDEO
    QObject::connect(m_backendObject, SIGNAL(hasVideoChanged(bool)),        q, SLOT(hasVideoChanged(bool)));
#endif

    QObject::connect(m_backendObject, SIGNAL(bufferStatus(int)),            q, SLOT(bufferStatus(int)));
    QObject::connect(m_backendObject, SIGNAL(finished()),                   q, SLOT(finished()));
    QObject::connect(m_backendObject, SIGNAL(aboutToFinish()),              q, SLOT(_k_aboutToFinish()));
    QObject::connect(m_backendObject, SIGNAL(prefinishMarkReached(qint32)), q, SLOT(prefinishMarkReached(qint32)));
    QObject::connect(m_backendObject, SIGNAL(totalTimeChanged(qint64)),     q, SLOT(totalTimeChanged(qint64)));
     
    QObject::connect(m_backendObject, SIGNAL(metaDataChanged(const QMultiMap<QString, QString> &)),
            q, SLOT(_k_metaDataChanged(const QMultiMap<QString, QString> &)));

    QObject::connect(m_backendObject, SIGNAL(currentSourceChanged(const MediaSource &)), 
            q, SLOT(_k_currentSourceChanged(const MediaSource &)));

    // set up attributes
    pINTERFACE_CALL(setTickInterval(tickInterval));
    pINTERFACE_CALL(setPrefinishMark(prefinishMark));
    pINTERFACE_CALL(setTransitionTime(transitionTime));

    switch(state)
    {
    case LoadingState:
    case StoppedState:
    case ErrorState:
        break;

    case PlayingState:
    case BufferingState:
        QTimer::singleShot(0, q, SLOT(_k_resumePlay()));
        break;

    case PausedState:
        QTimer::singleShot(0, q, SLOT(_k_resumePause()));
        break;
    }
    const State backendState = pINTERFACE_CALL(state());
    if (state != backendState && state != ErrorState) {
        // careful: if state is ErrorState we might be switching from a
        // MediaObject to a ByteStream for KIO fallback. In that case the state
        // change to ErrorState was already suppressed.
        pDebug() << "emitting a state change because the backend object has been replaced";
        emit q->stateChanged(backendState, state);
        state = backendState;
    }

#ifndef QT_NO_PHONON_MEDIACONTROLLER
    for (int i = 0 ; i < interfaceList.count(); ++i) {
        interfaceList.at(i)->_backendObjectChanged();
    }
#endif

    // set up attributes
    if (isPlayable(mediaSource.type())) {

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        if (mediaSource.type() == MediaSource::Stream) {
            Q_ASSERT(mediaSource.stream());
            mediaSource.stream()->d_func()->setMediaObjectPrivate(this);
        }
#endif 
        pINTERFACE_CALL(setSource(mediaSource));
    }
}

void MediaObjectPrivate::_k_resumePlay()
{
    qobject_cast<MediaObjectInterface *>(m_backendObject)->play();
    if (currentTime > 0) {
        qobject_cast<MediaObjectInterface *>(m_backendObject)->seek(currentTime);
    }
}

void MediaObjectPrivate::_k_resumePause()
{
    pINTERFACE_CALL(pause());
    if (currentTime > 0) {
        qobject_cast<MediaObjectInterface *>(m_backendObject)->seek(currentTime);
    }
}

void MediaObjectPrivate::_k_metaDataChanged(const QMultiMap<QString, QString> &newMetaData)
{
    metaData = newMetaData;
    emit q_func()->metaDataChanged();
}

void MediaObjectPrivate::phononObjectDestroyed(MediaNodePrivate *bp)
{
    // this method is called from Phonon::Base::~Base(), meaning the AudioPath
    // dtor has already been called, also virtual functions don't work anymore
    // (therefore qobject_cast can only downcast from Base)
    Q_ASSERT(bp);
    Q_UNUSED(bp);
}

MediaObject *createPlayer(Phonon::Category category, const MediaSource &source)
{
    MediaObject *mo = new MediaObject;
    AudioOutput *ao = new AudioOutput(category, mo);
    createPath(mo, ao);
    if (isPlayable(source.type())) {
        mo->setCurrentSource(source);
    }
    return mo;
}

void MediaObject::_k_resumePlay()
{
	K_D(MediaObject);
	d->_k_resumePlay();
}

void MediaObject::_k_resumePause()
{
	K_D(MediaObject);
	d->_k_resumePause();
}

void MediaObject::_k_metaDataChanged(const QMultiMap<QString,QString> &un_named_arg1)
{
	K_D(MediaObject);
	d->_k_metaDataChanged(un_named_arg1);
}


#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
void MediaObject::_k_stateChanged(Phonon::State un_named_arg1,Phonon::State un_named_arg2)
{
	K_D(MediaObject);
	d->_k_stateChanged(un_named_arg1,un_named_arg2);
}
#endif

void MediaObject::_k_aboutToFinish()
{
	K_D(MediaObject);
	d->_k_aboutToFinish();
}

void MediaObject::_k_currentSourceChanged(const MediaSource &un_named_arg1)
{
	K_D(MediaObject);
	d->_k_currentSourceChanged(un_named_arg1);
}

} //namespace Phonon

QT_END_NAMESPACE

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME

