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

#ifndef QT7_QUICKTIMESTREAMREADER_H
#define QT7_QUICKTIMESTREAMREADER_H

#include <phonon/mediasource.h>
#include <phonon/streaminterface.h>
#include <QtCore/QReadWriteLock>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class QuickTimeStreamReader : public QObject, Phonon::StreamInterface
    {
        QT7_CS_OBJECT(QuickTimeStreamReader)
        CS_INTERFACES(Phonon::StreamInterface)

    public:
        QuickTimeStreamReader(const Phonon::MediaSource &source);
        ~QuickTimeStreamReader();

        int readData(long offset, long size, void *data);
        bool readAllData();
        QByteArray *pointerToData();
        void writeData(const QByteArray &data) override;
        void endOfData() override;
        void setStreamSize(qint64 newSize) override;
        qint64 streamSize() const;
        void setStreamSeekable(bool s) override;
        bool streamSeekable() const;
        void setCurrentPos(qint64 pos);
        qint64 currentPos() const;
        int currentBufferSize() const;        

        QByteArray m_buffer;
        mutable QReadWriteLock m_lock;
        bool m_seekable;
        qint64 m_pos;
        qint64 m_size;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_QUICKTIMESTREAMREADER_H
