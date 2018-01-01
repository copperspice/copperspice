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

#include "iodevicestream_p.h"
#include "abstractmediastream_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{

class IODeviceStreamPrivate : public AbstractMediaStreamPrivate
{
    Q_DECLARE_PUBLIC(IODeviceStream)

    protected:
        IODeviceStreamPrivate(QIODevice *_ioDevice)
            : ioDevice(_ioDevice)
        {
            if (!ioDevice->isOpen()) {
                ioDevice->open(QIODevice::ReadOnly);
            }
            Q_ASSERT(ioDevice->isOpen());
            Q_ASSERT(ioDevice->isReadable());
            streamSize = ioDevice->size();
            streamSeekable = !ioDevice->isSequential();
        }

    private:
        QIODevice *ioDevice;
};

IODeviceStream::IODeviceStream(QIODevice *ioDevice, QObject *parent)
    : AbstractMediaStream(*new IODeviceStreamPrivate(ioDevice), parent)
{
    Q_D(IODeviceStream);
    d->ioDevice->reset();
}

IODeviceStream::~IODeviceStream()
{
}

void IODeviceStream::reset()
{
    Q_D(IODeviceStream);
    d->ioDevice->reset();
    //resetDone();
}

void IODeviceStream::needData()
{
    quint32 size = 4096;
    Q_D(IODeviceStream);
    const QByteArray data = d->ioDevice->read(size);
    if (data.isEmpty() && !d->ioDevice->atEnd()) {
        error(Phonon::NormalError, d->ioDevice->errorString());
    }
    writeData(data);
    if (d->ioDevice->atEnd()) {
        endOfData();
    }
}

void IODeviceStream::seekStream(qint64 offset)
{
    Q_D(IODeviceStream);
    d->ioDevice->seek(offset);
    //seekStreamDone();
}

} // namespace Phonon

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE
