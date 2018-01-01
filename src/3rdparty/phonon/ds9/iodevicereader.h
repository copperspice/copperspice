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

#ifndef DS9_IODEVICEREADER_H
#define DS9_IODEVICEREADER_H

#include "qbasefilter.h"
#include <phonon/mediasource.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM

namespace Phonon
{
    class MediaSource;
    namespace DS9
    {
        class MediaGraph;
        class StreamReader;

        //for the streams...
        //the filter
        class IODeviceReader : public QBaseFilter
        {
        public:
            IODeviceReader(const MediaSource &source, const MediaGraph *);
            ~IODeviceReader();

        private:
            StreamReader *m_streamReader;
        };

    }
}

#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE

#endif
