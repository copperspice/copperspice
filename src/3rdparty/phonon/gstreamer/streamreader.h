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

#ifndef GSTREAMER_StreamReader_H
#define GSTREAMER_StreamReader_H

#include <phonon/mediasource.h>
#include <phonon/streaminterface.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{
    class MediaSource;
    namespace Gstreamer
    {
        class StreamReader : public Phonon::StreamInterface
        {
        public:

           StreamReader(const Phonon::MediaSource &source)
            :  m_pos(0)
             , m_size(0)
             , m_seekable(false)
            {
                connectToSource(source);
            }

            int currentBufferSize() const
            {
                return m_buffer.size();
            }

            void writeData(const QByteArray &data) override {
                m_pos += data.size();
                m_buffer += data;
            }

            void setCurrentPos(qint64 pos)
            {
                m_pos = pos;
                seekStream(pos);
                m_buffer.clear();
            }
            
            quint64 currentPos() const
            {
                return m_pos;
            }

            bool read(quint64 offset, int length, char * buffer);

            void endOfData() override {}

            void setStreamSize(qint64 newSize) override {
                m_size = newSize;
            }

            qint64 streamSize() const {
                return m_size;
            }

            void setStreamSeekable(bool s) override {
                m_seekable = s;
            }

            bool streamSeekable() const {
                return m_seekable;
            }

   private:
            QByteArray m_buffer;
            quint64 m_pos;
            quint64 m_size;
            bool m_seekable;
        };
    }
}

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif
