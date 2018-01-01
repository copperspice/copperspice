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

#include "streaminterface.h"
#include "streaminterface_p.h"
#include "abstractmediastream.h"
#include "abstractmediastream_p.h"
#include "mediasource_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{

StreamInterface::StreamInterface()
    : d(new StreamInterfacePrivate)
{
    d->q = this;
}

StreamInterface::~StreamInterface()
{
    if (d->connected) {
        AbstractMediaStreamPrivate *dd = d->mediaSource.stream()->d_func();
        dd->setStreamInterface(0);
    }
    delete d;
}

void StreamInterface::connectToSource(const MediaSource &mediaSource)
{
    Q_ASSERT(!d->connected);
    d->connected = true;
    d->mediaSource = mediaSource;
    Q_ASSERT(d->mediaSource.type() == MediaSource::Stream);
    Q_ASSERT(d->mediaSource.stream());
    AbstractMediaStreamPrivate *dd = d->mediaSource.stream()->d_func();
    dd->setStreamInterface(this);
    d->mediaSource.stream()->reset();
}

void StreamInterfacePrivate::disconnectMediaStream()
{
    Q_ASSERT(connected);
    connected = false;

    // if mediaSource has autoDelete set then it will delete the AbstractMediaStream again who's
    // destructor is calling us right now
    mediaSource.setAutoDelete(false);

    mediaSource = MediaSource();
    q->endOfData();
    q->setStreamSeekable(false);
}

void StreamInterface::needData()
{
    if (d->mediaSource.type() == MediaSource::Stream) {
        d->mediaSource.stream()->needData();
    }
}

void StreamInterface::enoughData()
{
    Q_ASSERT(d->connected);
    if (d->mediaSource.type() == MediaSource::Stream) {
        d->mediaSource.stream()->enoughData();
    }
}

void StreamInterface::seekStream(qint64 offset)
{
    Q_ASSERT(d->connected);
    if (d->mediaSource.type() == MediaSource::Stream) {
        d->mediaSource.stream()->seekStream(offset);
    }
}

void StreamInterface::reset()
{
    Q_ASSERT(d->connected);
    if (d->mediaSource.type() == MediaSource::Stream) {
        d->mediaSource.stream()->reset();
    }
}

} // namespace Phonon

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE


