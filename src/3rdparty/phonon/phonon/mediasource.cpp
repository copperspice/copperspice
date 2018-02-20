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

#include "mediasource.h"
#include "mediasource_p.h"
#include "iodevicestream_p.h"
#include "abstractmediastream_p.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QFSFileEngine>

QT_BEGIN_NAMESPACE

namespace Phonon
{

MediaSource::MediaSource(MediaSourcePrivate &dd)
    : d(&dd)
{
}

MediaSource::MediaSource()
    : d(new MediaSourcePrivate(Empty))
{
}

MediaSource::MediaSource(const QString &filename)
    : d(new MediaSourcePrivate(LocalFile))
{
    const QFileInfo fileInfo(filename);
    if (fileInfo.exists()) {

        bool localFs = QAbstractFileEngine::LocalDiskFlag &
                     QFSFileEngine(filename).fileFlags(QAbstractFileEngine::LocalDiskFlag);

        if (localFs && ! filename.startsWith(QLatin1String(":/")) && ! filename.startsWith(QLatin1String("qrc://"))) {
            d->url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());

        } else {

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            // it's a Qt resource -> use QFile
            d->type = Stream;
            d->ioDevice = new QFile(filename);
            d->setStream(new IODeviceStream(d->ioDevice, d->ioDevice));
            d->url =  QUrl::fromLocalFile(fileInfo.absoluteFilePath());
#else
            d->type = Invalid;
#endif
        }

    } else {
        d->url = QUrl(filename);

        if (d->url.isValid()) {
            d->type = Url;
        } else {
            d->type = Invalid;
        }
    }
}

MediaSource::MediaSource(const QUrl &url)
    : d(new MediaSourcePrivate(Url))
{
    if (url.isValid()) {
        d->url = url;
    } else {
        d->type = Invalid;
    }
}

MediaSource::MediaSource(Phonon::DiscType dt, const QString &deviceName)
    : d(new MediaSourcePrivate(Disc))
{
    if (dt == NoDisc) {
        d->type = Invalid;
        return;
    }
    d->discType = dt;
    d->deviceName = deviceName;
}

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
MediaSource::MediaSource(AbstractMediaStream *stream)
    : d(new MediaSourcePrivate(Stream))
{
    if (stream) {
        d->setStream(stream);
    } else {
        d->type = Invalid;
    }
}

MediaSource::MediaSource(QIODevice *ioDevice)
    : d(new MediaSourcePrivate(Stream))
{
    if (ioDevice) {
        d->setStream(new IODeviceStream(ioDevice, ioDevice));
        d->ioDevice = ioDevice;
    } else {
        d->type = Invalid;
    }
}
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

/* post 4.0
MediaSource::MediaSource(const QList<MediaSource> &mediaList)
    : d(new MediaSourcePrivate(Link))
{
    d->linkedSources = mediaList;
    for (MediaSource ms : mediaList) {
        Q_ASSERT(ms.type() != Link);
    }
}

QList<MediaSource> MediaSource::substreams() const
{
    return d->linkedSources;
}
*/

MediaSource::~MediaSource()
{
}

MediaSourcePrivate::~MediaSourcePrivate()
{
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    if (autoDelete) {
        //here we use deleteLater because this object
        //might be destroyed from another thread
        if (stream)
            stream->deleteLater();
        if (ioDevice)
            ioDevice->deleteLater();
    }
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
}

MediaSource::MediaSource(const MediaSource &rhs)
    : d(rhs.d)
{
}

MediaSource &MediaSource::operator=(const MediaSource &rhs)
{
    d = rhs.d;
    return *this;
}

bool MediaSource::operator==(const MediaSource &rhs) const
{
    return d == rhs.d;
}

void MediaSource::setAutoDelete(bool autoDelete)
{
    d->autoDelete = autoDelete;
}

bool MediaSource::autoDelete() const
{
    return d->autoDelete;
}

MediaSource::Type MediaSource::type() const
{
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
    if (d->type == Stream && d->stream == 0) {
        return Invalid;
    }
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM
    return d->type;
}

QString MediaSource::fileName() const
{
    return d->url.toLocalFile();
}

QUrl MediaSource::url() const
{
    return d->url;
}

Phonon::DiscType MediaSource::discType() const
{
    return d->discType;
}

QString MediaSource::deviceName() const
{
    return d->deviceName;
}

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
AbstractMediaStream *MediaSource::stream() const
{
    return d->stream;
}

void MediaSourcePrivate::setStream(AbstractMediaStream *s)
{
    stream = s;
}
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM


//X AudioCaptureDevice MediaSource::audioCaptureDevice() const
//X {
//X     return d->audioCaptureDevice;
//X }
//X
//X VideoCaptureDevice MediaSource::videoCaptureDevice() const
//X {
//X     return d->videoCaptureDevice;
//X }

} // namespace Phonon

QT_END_NAMESPACE


