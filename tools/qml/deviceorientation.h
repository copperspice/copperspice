/***********************************************************************
*
* Copyright (c) 2012-2013 Barbara Geller
* Copyright (c) 2012-2013 Ansel Sermersheim
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <QObject>

QT_BEGIN_NAMESPACE

class DeviceOrientationPrivate;
class DeviceOrientation : public QObject
{
    Q_OBJECT
    Q_ENUMS(Orientation)
public:
    enum Orientation {
        UnknownOrientation,
        Portrait,
        Landscape,
        PortraitInverted,
        LandscapeInverted
    };

    virtual Orientation orientation() const = 0;
    virtual void setOrientation(Orientation) = 0;

    virtual void pauseListening() = 0;
    virtual void resumeListening() = 0;

    static DeviceOrientation *instance();

signals:
    void orientationChanged();

protected:
    DeviceOrientation() {}

private:
    DeviceOrientationPrivate *d_ptr;
    friend class DeviceOrientationPrivate;
};

QT_END_NAMESPACE

#endif
