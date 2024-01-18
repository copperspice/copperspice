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

#ifndef QWINDOWSWINDOWFUNCTIONS_H
#define QWINDOWSWINDOWFUNCTIONS_H

#include <QByteArray>
#include <QGuiApplication>

class QWindow;

class QWindowsWindowFunctions
{
public:
    enum TouchWindowTouchType {
        NormalTouch   = 0x00000000,
        FineTouch     = 0x00000001,
        WantPalmTouch = 0x00000002
    };

    using TouchWindowTouchTypes = QFlags<TouchWindowTouchType>;

    typedef void (*SetTouchWindowTouchType)(QWindow *window, QWindowsWindowFunctions::TouchWindowTouchTypes touchType);
    static const QByteArray setTouchWindowTouchTypeIdentifier() { return QByteArray("WindowsSetTouchWindowTouchType"); }

    static void setTouchWindowTouchType(QWindow *window, TouchWindowTouchTypes type)
    {
        SetTouchWindowTouchType func =  reinterpret_cast<SetTouchWindowTouchType>(
            QGuiApplication::platformFunction(setTouchWindowTouchTypeIdentifier()));

        if (func) {
           func(window, type);
        }
    }

    typedef void (*SetHasBorderInFullScreen)(QWindow *window, bool border);
    static const QByteArray setHasBorderInFullScreenIdentifier() { return QByteArray("WindowsSetHasBorderInFullScreen"); }

    static void setHasBorderInFullScreen(QWindow *window, bool border)
    {
        SetHasBorderInFullScreen func = reinterpret_cast<SetHasBorderInFullScreen>(
            QGuiApplication::platformFunction(setHasBorderInFullScreenIdentifier()));

        if (func) {
           func(window, border);
        }
    }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWindowsWindowFunctions::TouchWindowTouchTypes)

#endif
