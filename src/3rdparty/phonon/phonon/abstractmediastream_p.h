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

#ifndef PHONON_ABSTRACTMEDIASTREAM_P_H
#define PHONON_ABSTRACTMEDIASTREAM_P_H

#include "phonon_export.h"
#include "abstractmediastream.h"
#include "mediaobject_p.h"
#include "streaminterface.h"
#include "medianodedestructionhandler_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

class MediaObjectPrivate;

namespace Phonon
{
class PHONON_EXPORT AbstractMediaStreamPrivate : private MediaNodeDestructionHandler
{
    friend class MediaObject;
    Q_DECLARE_PUBLIC(AbstractMediaStream)
    public:
        void setStreamInterface(StreamInterface *);
        void setMediaObjectPrivate(MediaObjectPrivate *);
        ~AbstractMediaStreamPrivate();

    protected:
        AbstractMediaStreamPrivate()
            : streamSize(0),
            streamSeekable(false),
            ignoreWrites(false),
            streamInterface(0),
            mediaObjectPrivate(0),
            errorType(NoError)
        {
        }

        virtual void setStreamSize(qint64 newSize);
        virtual void setStreamSeekable(bool s);
        virtual void writeData(const QByteArray &data);
        virtual void endOfData();
        void phononObjectDestroyed(MediaNodePrivate *) override;

        AbstractMediaStream *q_ptr;
        qint64 streamSize;
        bool streamSeekable;
        bool ignoreWrites;
        StreamInterface *streamInterface;
        MediaObjectPrivate *mediaObjectPrivate;
        Phonon::ErrorType errorType;
        QString errorText;
};
} // namespace Phonon

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif // ABSTRACTMEDIASTREAM_P_H
