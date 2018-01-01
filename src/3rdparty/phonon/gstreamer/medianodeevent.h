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

#ifndef GSTREAMER_MEDIANODEEVENT_H
#define GSTREAMER_MEDIANODEEVENT_H

#include "common.h"
#include <QtCore>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
class MediaNodeEvent
{
public:
    enum Type {
        VideoAvailable,
        AudioAvailable,
        SourceChanged,
        MediaObjectConnected,
        StateChanged,
        VideoSinkAdded,
        VideoSinkRemoved,
        AudioSinkAdded,
        AudioSinkRemoved,
        VideoHandleRequest,
        VideoSizeChanged
    };

    MediaNodeEvent(Type type, const void *data = 0);
    virtual ~MediaNodeEvent();

    inline Type type() const
    {
        return eventType;
    };
    inline const void* data() const
    {
        return eventData;
    };

private:
    Type eventType;
    const void *eventData;
};

}
} // namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_MEDIANODEEVENT_H
