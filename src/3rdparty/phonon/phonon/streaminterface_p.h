/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef PHONON_STREAMINTERFACE_P_H
#define PHONON_STREAMINTERFACE_P_H

#include "streaminterface.h"
#include "mediasource.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{
class StreamInterfacePrivate
{
    friend class StreamInterface;
    public:
        void disconnectMediaStream();

    protected:
        inline StreamInterfacePrivate()
            : connected(false)
        {
        }

        StreamInterface *q;
        MediaSource mediaSource;
        bool connected;
};

} // namespace Phonon

#endif // QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif // STREAMINTERFACE_P_H

