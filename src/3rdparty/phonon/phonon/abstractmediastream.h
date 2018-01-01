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

#ifndef PHONON_ABSTRACTMEDIASTREAM_H
#define PHONON_ABSTRACTMEDIASTREAM_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QByteArray;

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{
class MediaObject;
class AbstractMediaStreamPrivate;

class PHONON_EXPORT AbstractMediaStream : public QObject
{
    PHN_CS_OBJECT(AbstractMediaStream)
    Q_DECLARE_PRIVATE(AbstractMediaStream)
    friend class MediaObject;
    friend class MediaObjectPrivate;
    friend class StreamInterface;
    public:
        virtual ~AbstractMediaStream();

    protected:
        explicit AbstractMediaStream(QObject *parent = nullptr);

        qint64 streamSize() const;

        void setStreamSize(qint64);

        bool streamSeekable() const;

        void setStreamSeekable(bool);

        void writeData(const QByteArray &data);

        void endOfData();

        void error(Phonon::ErrorType errorType, const QString &errorString);

        virtual void reset() = 0;

        virtual void needData() = 0;

        virtual void enoughData();

        virtual void seekStream(qint64 offset);

        AbstractMediaStream(AbstractMediaStreamPrivate &dd, QObject *parent);
        QScopedPointer<AbstractMediaStreamPrivate> d_ptr;
};

} // namespace Phonon

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif // PHONON_ABSTRACTMEDIASTREAM_H
