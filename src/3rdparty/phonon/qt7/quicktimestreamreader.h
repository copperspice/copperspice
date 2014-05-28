/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QT7_QUICKTIMESTREAMREADER_H
#define QT7_QUICKTIMESTREAMREADER_H

#include <phonon/mediasource.h>
#include <phonon/streaminterface.h>
#include <QtCore/QReadWriteLock>
#include <QuickTime/Movies.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class QuickTimeStreamReader : public QObject, Phonon::StreamInterface
    {
        CS_OBJECT(QuickTimeStreamReader)
        CS_INTERFACES(Phonon::StreamInterface)

    public:
        QuickTimeStreamReader(const Phonon::MediaSource &source);
        ~QuickTimeStreamReader();

        int readData(long offset, long size, void *data);
        bool readAllData();
        QByteArray *pointerToData();
        void writeData(const QByteArray &data);
        void endOfData();
        void setStreamSize(qint64 newSize);
        qint64 streamSize() const;
        void setStreamSeekable(bool s);
        bool streamSeekable() const;
        void setCurrentPos(qint64 pos);
        qint64 currentPos() const;
        int currentBufferSize() const;
        Movie movieRef();

        QByteArray m_buffer;
        mutable QReadWriteLock m_lock;
        bool m_seekable;
        qint64 m_pos;
        qint64 m_size;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_QUICKTIMESTREAMREADER_H
