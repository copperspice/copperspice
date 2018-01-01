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

#include "abstractmediastream.h"
#include "abstractmediastream_p.h"
#include "mediaobjectinterface.h"
#include "mediaobject_p.h"
#include "streaminterface_p.h"

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_BEGIN_NAMESPACE

namespace Phonon
{

AbstractMediaStream::AbstractMediaStream(QObject *parent)
    : QObject(parent),
    d_ptr(new AbstractMediaStreamPrivate)
{
    d_ptr->q_ptr = this;
}

AbstractMediaStream::AbstractMediaStream(AbstractMediaStreamPrivate &dd, QObject *parent)
    : QObject(parent),
    d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

AbstractMediaStream::~AbstractMediaStream()
{
}

qint64 AbstractMediaStream::streamSize() const
{
    return d_ptr->streamSize;
}

void AbstractMediaStream::setStreamSize(qint64 newSize)
{
    d_ptr->setStreamSize(newSize);
}

void AbstractMediaStreamPrivate::setStreamSize(qint64 newSize)
{
    streamSize = newSize;
    if (streamInterface) {
        streamInterface->setStreamSize(newSize);
    }
}

bool AbstractMediaStream::streamSeekable() const
{
    return d_ptr->streamSeekable;
}

void AbstractMediaStream::setStreamSeekable(bool s)
{
    d_ptr->setStreamSeekable(s);
}

void AbstractMediaStreamPrivate::setStreamSeekable(bool s)
{
    streamSeekable = s;
    if (streamInterface) {
        streamInterface->setStreamSeekable(s);
    }
}

void AbstractMediaStream::writeData(const QByteArray &data)
{
    d_ptr->writeData(data);
}

void AbstractMediaStreamPrivate::writeData(const QByteArray &data)
{
    if (ignoreWrites) {
        return;
    }
    Q_ASSERT(streamInterface);
    streamInterface->writeData(data);
}

void AbstractMediaStream::endOfData()
{
    d_ptr->endOfData();
}

void AbstractMediaStreamPrivate::endOfData()
{
    if (streamInterface) {
        streamInterface->endOfData();
    }
}

void AbstractMediaStream::error(Phonon::ErrorType type, const QString &text)
{
    Q_D(AbstractMediaStream);
    d->errorType = type;
    d->errorText = text;
    if (d->mediaObjectPrivate) {
        // TODO: MediaObject might be in a different thread
        d->mediaObjectPrivate->streamError(type, text);
    }
}

void AbstractMediaStream::enoughData()
{
}

void AbstractMediaStream::seekStream(qint64)
{
    Q_ASSERT(!d_ptr->streamSeekable);
}

AbstractMediaStreamPrivate::~AbstractMediaStreamPrivate()
{
    if (mediaObjectPrivate) {
        // TODO: MediaObject might be in a different thread
        mediaObjectPrivate->removeDestructionHandler(this);
    }
    if (streamInterface) {
        // TODO: StreamInterface might be in a different thread
        streamInterface->d->disconnectMediaStream();
    }
}

void AbstractMediaStreamPrivate::setStreamInterface(StreamInterface *iface)
{
    Q_Q(AbstractMediaStream);
    streamInterface = iface;
    if (!iface) {
        // our subclass might be just about to call writeData, so tell it we have enoughData and
        // ignore the next writeData calls
        q->enoughData();
        ignoreWrites = true;
        return;
    }
    if (ignoreWrites) {
        ignoreWrites = false;
        // we had a StreamInterface before. The new StreamInterface expects us to start reading from
        // position 0
        q->reset();
    } else {
        iface->setStreamSize(streamSize);
        iface->setStreamSeekable(streamSeekable);
    }
}

void AbstractMediaStreamPrivate::setMediaObjectPrivate(MediaObjectPrivate *mop)
{
    // TODO: MediaObject might be in a different thread
    mediaObjectPrivate = mop;
    mediaObjectPrivate->addDestructionHandler(this);
    if (!errorText.isEmpty()) {
        mediaObjectPrivate->streamError(errorType, errorText);
    }
}

void AbstractMediaStreamPrivate::phononObjectDestroyed(MediaNodePrivate *bp)
{
    // TODO: MediaObject might be in a different thread
    Q_ASSERT(bp == mediaObjectPrivate);
    Q_UNUSED(bp);
    mediaObjectPrivate = 0;
}

} // namespace Phonon


QT_END_NAMESPACE

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM


