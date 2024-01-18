/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QVIDEOOUTPUTORIENTATIONHANDLER_P_H
#define QVIDEOOUTPUTORIENTATIONHANDLER_P_H

#include <qobject.h>

class Q_MULTIMEDIA_EXPORT QVideoOutputOrientationHandler : public QObject
{
    MULTI_CS_OBJECT(QVideoOutputOrientationHandler)

public:
    explicit QVideoOutputOrientationHandler(QObject *parent = nullptr);

    int currentOrientation() const;

public:
    MULTI_CS_SIGNAL_1(Public, void orientationChanged(int angle))
    MULTI_CS_SIGNAL_2(orientationChanged,angle)

private:
    MULTI_CS_SLOT_1(Private, void screenOrientationChanged(Qt::ScreenOrientation orientation))
    MULTI_CS_SLOT_2(screenOrientationChanged)

    int m_currentOrientation;
};

#endif
