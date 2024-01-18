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

#ifndef QXCBSCREENFUNCTIONS_H
#define QXCBSCREENFUNCTIONS_H

#include <QPlatform_HeaderHelper>

class QScreen;

class QXcbScreenFunctions
{
public:
    typedef bool (*VirtualDesktopNumber)(const QScreen *screen);
    static const QByteArray virtualDesktopNumberIdentifier() { return QByteArray("XcbVirtualDesktopNumber"); }
    static int virtualDesktopNumber(const QScreen *screen)
    {
        return QPlatformHeaderHelper::callPlatformFunction<int, VirtualDesktopNumber, const QScreen *>(virtualDesktopNumberIdentifier(), screen);
    }
};

#endif
