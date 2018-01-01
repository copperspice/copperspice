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

#ifndef PHONON_STREAMINTERFACE_H
#define PHONON_STREAMINTERFACE_H

#include "phonon_export.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{
class StreamInterfacePrivate;
class MediaSource;

class PHONON_EXPORT StreamInterface
{
    friend class StreamInterfacePrivate;
    friend class AbstractMediaStreamPrivate;
    public:
        virtual ~StreamInterface();
        virtual void writeData(const QByteArray &data) = 0;
        virtual void endOfData() = 0;
        virtual void setStreamSize(qint64 newSize) = 0;
        virtual void setStreamSeekable(bool s) = 0;
        void connectToSource(const MediaSource &mediaSource);
        void needData();
        void enoughData();
        void seekStream(qint64 seekTo);
        void reset();

    protected:
        StreamInterface();

        StreamInterfacePrivate *const d;
};
} // namespace Phonon

CS_DECLARE_INTERFACE(Phonon::StreamInterface, "StreamInterface1.phonon.kde.org")

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif // PHONON_STREAMINTERFACE_H
