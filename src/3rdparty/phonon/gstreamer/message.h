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

#ifndef GSTREAMER_MESSAGE_H
#define GSTREAMER_MESSAGE_H

#include "common.h"
#include <QtCore/QMetaType>
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

class MediaObject;
class Message
{
public:
    Message();
    Message(GstMessage* message, MediaObject *source);
    ~Message();

    GstMessage* rawMessage() const;
    MediaObject *source() const;
    Message(const Message &other);

private:
    GstMessage* m_message;
    MediaObject *m_source;
};

}   // ns gstreamer
}   // ns phonon

QT_END_NAMESPACE

Q_DECLARE_METATYPE(Phonon::Gstreamer::Message)

#endif // Phonon_GSTREAMER_MESSAGE_H
