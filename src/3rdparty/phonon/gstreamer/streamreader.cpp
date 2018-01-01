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
**  This file is part of the KDE project.
********************************************************/

#include "streamreader.h"
#include <QtCore/QMutex>
#include <phonon/streaminterface.h>

QT_BEGIN_NAMESPACE
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
namespace Phonon
{
namespace Gstreamer
{

bool StreamReader::read(quint64 pos, int length, char * buffer)
{
     if (currentPos() - currentBufferSize() != pos) {
         if (!streamSeekable())
             return false;
         setCurrentPos(pos);
     }

    while (currentBufferSize() < length) {
        int oldSize = currentBufferSize();
        needData();
        if (oldSize == currentBufferSize())
            return false; // We didn't get any data
    }

    memcpy(buffer, m_buffer.data(), length);
    //truncate the buffer
    m_buffer = m_buffer.mid(pos);    
    return true;
}

}
}
#endif //QT_NO_PHONON_ABSTRACTMEDIASTREAM

QT_END_NAMESPACE
