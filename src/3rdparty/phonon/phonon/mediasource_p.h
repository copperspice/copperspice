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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_MEDIASOURCE_P_H
#define PHONON_MEDIASOURCE_P_H

#include "mediasource.h"
#include "objectdescription.h"
#include "abstractmediastream.h"

#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QFile;

namespace Phonon
{

class PHONON_EXPORT MediaSourcePrivate : public QSharedData
{
    public:
        MediaSourcePrivate(MediaSource::Type t)
            : type(t), discType(NoDisc),
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            stream(0),
            ioDevice(0),
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
            autoDelete(false)
        {
        }

        virtual ~MediaSourcePrivate();

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        void setStream(AbstractMediaStream *s);
#endif

        MediaSource::Type type;
        QUrl url;
        Phonon::DiscType discType;
        QString deviceName;

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
        // The AbstractMediaStream(2) may be deleted at any time by the application. If that happens
        // stream will be 0 automatically, but streamEventQueue will stay valid as we hold a
        // reference to it. This is necessary to avoid a races when setting the MediaSource while
        // another thread deletes the AbstractMediaStream2. StreamInterface(2) will then just get a
        // StreamEventQueue where nobody answers.
        QPointer<AbstractMediaStream> stream;

//        AudioCaptureDevice audioCaptureDevice;
//        VideoCaptureDevice videoCaptureDevice;
        QIODevice *ioDevice;
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
        //QList<MediaSource> linkedSources;
        bool autoDelete;
};

} // namespace Phonon

QT_END_NAMESPACE

#endif // MEDIASOURCE_P_H

