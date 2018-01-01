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

#ifndef PHONON_IODEVICESTREAM_P_H
#define PHONON_IODEVICESTREAM_P_H

#include "abstractmediastream.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

class QIODevice;

namespace Phonon
{

class IODeviceStreamPrivate;
class IODeviceStream : public AbstractMediaStream
{
    PHN_CS_OBJECT(IODeviceStream)
    Q_DECLARE_PRIVATE(IODeviceStream)

    public:
        explicit IODeviceStream(QIODevice *ioDevice, QObject *parent = nullptr);
        ~IODeviceStream();

        void reset() override;
        void needData() override;
        void seekStream(qint64) override;
};
} // namespace Phonon

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif // PHONON_IODEVICESTREAM_P_H
