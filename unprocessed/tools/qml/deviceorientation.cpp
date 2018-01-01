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

#include "deviceorientation.h"

QT_USE_NAMESPACE

class DefaultDeviceOrientation : public DeviceOrientation
{
    Q_OBJECT
public:
    DefaultDeviceOrientation() : DeviceOrientation(), m_orientation(DeviceOrientation::Portrait) {}

    Orientation orientation() const {
        return m_orientation;
    }

    void pauseListening() {
    }
    void resumeListening() {
    }

    void setOrientation(Orientation o) {
        if (o != m_orientation) {
            m_orientation = o;
            emit orientationChanged();
        }
    }

    Orientation m_orientation;
};

DeviceOrientation* DeviceOrientation::instance()
{
    static DefaultDeviceOrientation *o = 0;
    if (!o)
        o = new DefaultDeviceOrientation;
    return o;
}

#include "deviceorientation.moc"

