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

#ifndef QXCBWINDOWFUNCTIONS_H
#define QXCBWINDOWFUNCTIONS_H

#include <QPlatform_HeaderHelper>

class QWindow;

class QXcbWindowFunctions
{
public:
    enum WmWindowType {
        Normal       = 0x000001,
        Desktop      = 0x000002,
        Dock         = 0x000004,
        Toolbar      = 0x000008,
        Menu         = 0x000010,
        Utility      = 0x000020,
        Splash       = 0x000040,
        Dialog       = 0x000080,
        DropDownMenu = 0x000100,
        PopupMenu    = 0x000200,
        Tooltip      = 0x000400,
        Notification = 0x000800,
        Combo        = 0x001000,
        Dnd          = 0x002000,
        KdeOverride  = 0x004000
    };
    using WmWindowTypes = QFlags<WmWindowType>;

    typedef void (*SetWmWindowType)(QWindow *window, QXcbWindowFunctions::WmWindowTypes windowType);
    static const QByteArray setWmWindowTypeIdentifier() { return QByteArray("XcbSetWmWindowType"); }
    static void setWmWindowType(QWindow *window, WmWindowType type)
    {
        return QPlatformHeaderHelper::callPlatformFunction<void, SetWmWindowType, QWindow *,
            WmWindowType>(setWmWindowTypeIdentifier(), window, type);
    }

    typedef void (*SetWmWindowRole)(QWindow *window, const QByteArray &role);
    static const QByteArray setWmWindowRoleIdentifier() { return QByteArray("XcbSetWmWindowRole"); }

    static void setWmWindowRole(QWindow *window, const QByteArray &role)
    {
        return QPlatformHeaderHelper::callPlatformFunction<void, SetWmWindowRole, QWindow *, const QByteArray &>(setWmWindowRoleIdentifier(), window, role);
    }

    typedef void (*SetWmWindowIconText)(QWindow *window, const QString &text);
    static const QByteArray setWmWindowIconTextIdentifier() { return QByteArray("XcbSetWmWindowIconText"); }
    static void setWmWindowIconText(QWindow *window, const QString &text)
    {
        return QPlatformHeaderHelper::callPlatformFunction<void, SetWmWindowIconText, QWindow *, const QString &>(setWmWindowIconTextIdentifier(), window, text);
    }

    typedef void (*SetParentRelativeBackPixmap)(const QWindow *window);
    static const QByteArray setParentRelativeBackPixmapIdentifier() { return QByteArray("XcbSetParentRelativeBackPixmap"); }
    static void setParentRelativeBackPixmap(const QWindow *window)
    {
        return QPlatformHeaderHelper::callPlatformFunction<void, SetParentRelativeBackPixmap, const QWindow *>(setParentRelativeBackPixmapIdentifier(), window);
    }

    typedef bool (*RequestSystemTrayWindowDock)(const QWindow *window);
    static const QByteArray requestSystemTrayWindowDockIdentifier() { return QByteArray("XcbRequestSystemTrayWindowDockIdentifier"); }
    static bool requestSystemTrayWindowDock(const QWindow *window)
    {
        return QPlatformHeaderHelper::callPlatformFunction<bool, RequestSystemTrayWindowDock, const QWindow *>(requestSystemTrayWindowDockIdentifier(), window);
    }

    typedef QRect (*SystemTrayWindowGlobalGeometry)(const QWindow *window);
    static const QByteArray systemTrayWindowGlobalGeometryIdentifier() { return QByteArray("XcbSystemTrayWindowGlobalGeometryIdentifier"); }
    static QRect systemTrayWindowGlobalGeometry(const QWindow *window)
    {
        return QPlatformHeaderHelper::callPlatformFunction<QRect, SystemTrayWindowGlobalGeometry, const QWindow *>(systemTrayWindowGlobalGeometryIdentifier(), window);
    }

    typedef uint (*VisualId)(QWindow *window);
    static const QByteArray visualIdIdentifier() { return QByteArray("XcbVisualId"); }

    static uint visualId(QWindow *window)
    {
        QXcbWindowFunctions::VisualId func = reinterpret_cast<VisualId>(QGuiApplication::platformFunction(visualIdIdentifier()));
        if (func)
            return func(window);
        return UINT_MAX;
    }
};

#endif
