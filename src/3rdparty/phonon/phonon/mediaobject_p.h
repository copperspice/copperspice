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

#ifndef PHONON_MEDIAOBJECT_P_H
#define PHONON_MEDIAOBJECT_P_H

#include "mediaobject.h"
#include "medianode_p.h"
#include <QtCore/QString>
#include "medianodedestructionhandler_p.h"
#include "mediasource.h"
#include <QtCore/QQueue>

QT_BEGIN_NAMESPACE

namespace Phonon
{
class KioFallback;
class KioFallbackImpl;
class FrontendInterfacePrivate;

class MediaObjectPrivate : public MediaNodePrivate, private MediaNodeDestructionHandler
{
    friend class KioFallbackImpl;
    friend class AbstractMediaStream;
    friend class AbstractMediaStreamPrivate;
    Q_DECLARE_PUBLIC(MediaObject)

    public:
        QObject *qObject() override { return q_func(); }
        QList<FrontendInterfacePrivate *> interfaceList;

    protected:
        bool aboutToDeleteBackendObject() override;
        void createBackendObject() override;
        void phononObjectDestroyed(MediaNodePrivate *) override;

        PHONON_EXPORT void setupBackendObject();

        void _k_resumePlay();
        void _k_resumePause();
        void _k_metaDataChanged(const QMultiMap<QString, QString> &);
        void _k_aboutToFinish();
        void _k_currentSourceChanged(const MediaSource &);

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        void streamError(Phonon::ErrorType, const QString &);
        PHONON_EXPORT void _k_stateChanged(Phonon::State, Phonon::State);
#endif

        MediaObjectPrivate()
            : currentTime(0),
            tickInterval(0),
            metaData(),
            errorString(),
            prefinishMark(0),
            transitionTime(0), // gapless playback

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            kiofallback(0),
#endif
            state(Phonon::LoadingState)

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            , errorType(Phonon::NormalError),
            errorOverride(false),
            ignoreLoadingToBufferingStateChange(false),
            ignoreErrorToLoadingStateChange(false)
#endif
        {
        }

        qint64 currentTime;
        qint32 tickInterval;
        QMultiMap<QString, QString> metaData;
        QString errorString;
        qint32 prefinishMark;
        qint32 transitionTime;
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        AbstractMediaStream *kiofallback;
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
        State state
#ifdef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            ;
#else
            : 8;
        ErrorType errorType : 4;
        bool errorOverride : 1;
        bool ignoreLoadingToBufferingStateChange : 1;
        bool ignoreErrorToLoadingStateChange : 1;
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
        MediaSource mediaSource;
        QQueue<MediaSource> sourceQueue;
};
}

QT_END_NAMESPACE

#endif // MEDIAOBJECT_P_H

