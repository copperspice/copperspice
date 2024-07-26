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

#include <qwin_gui_eventdispatcher_p.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qwindowsysteminterface.h>

QWindowsGuiEventDispatcher::QWindowsGuiEventDispatcher(QObject *parent) :
    QEventDispatcherWin32(parent), m_flags(Qt::EmptyFlag)
{
    setObjectName("QWindowsGuiEventDispatcher");
    createInternalHwnd();                            // Do not delay registering timers, etc. for QtMfc.
}

bool QWindowsGuiEventDispatcher::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    const QEventLoop::ProcessEventsFlags oldFlags = m_flags;
    m_flags = flags;
    const bool rc = QEventDispatcherWin32::processEvents(flags);
    m_flags = oldFlags;
    return rc;
}

void QWindowsGuiEventDispatcher::sendPostedEvents()
{
    QEventDispatcherWin32::sendPostedEvents();
    QWindowSystemInterface::sendWindowSystemEvents(m_flags);
}

// Helpers for printing debug output for WM_* messages.
struct MessageDebugEntry
{
    UINT message;
    const char *description;
    bool interesting;
};

static const MessageDebugEntry
messageDebugEntries[] = {
    {WM_CREATE, "WM_CREATE", true},
    {WM_PAINT, "WM_PAINT", true},
    {WM_CLOSE, "WM_CLOSE", true},
    {WM_DESTROY, "WM_DESTROY", true},
    {WM_MOVE, "WM_MOVE", true},
    {WM_SIZE, "WM_SIZE", true},
    {WM_GETICON, "WM_GETICON", false},
    {WM_KEYDOWN, "WM_KEYDOWN", true},
    {WM_SYSKEYDOWN, "WM_SYSKEYDOWN", true},
    {WM_SYSCOMMAND, "WM_SYSCOMMAND", true},
    {WM_KEYUP, "WM_KEYUP", true},
    {WM_SYSKEYUP, "WM_SYSKEYUP", true},
#if defined(WM_APPCOMMAND)
    {WM_APPCOMMAND, "WM_APPCOMMAND", true},
#endif
    {WM_IME_CHAR, "WM_IMECHAR", true},
    {WM_IME_KEYDOWN, "WM_IMECHAR", true},
    {WM_CANCELMODE,  "WM_CANCELMODE", true},
    {WM_CHAR, "WM_CHAR", true},
    {WM_DEADCHAR, "WM_DEADCHAR", true},
    {WM_ACTIVATE, "WM_ACTIVATE", true},
    {WM_SETFOCUS, "WM_SETFOCUS", true},
    {WM_KILLFOCUS, "WM_KILLFOCUS", true},
    {WM_ENABLE, "WM_ENABLE", true},
    {WM_SHOWWINDOW, "WM_SHOWWINDOW", true},
    {WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED", true},
    {WM_SETCURSOR, "WM_SETCURSOR", false},
    {WM_GETFONT, "WM_GETFONT", true},
    {WM_LBUTTONDOWN, "WM_LBUTTONDOWN", true},
    {WM_LBUTTONUP, "WM_LBUTTONUP", true},
    {WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK", true},
    {WM_RBUTTONDOWN, "WM_RBUTTONDOWN", true},
    {WM_RBUTTONUP, "WM_RBUTTONUP", true},
    {WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK", true},
    {WM_MBUTTONDOWN, "WM_MBUTTONDOWN", true},
    {WM_MBUTTONUP, "WM_MBUTTONUP", true},
    {WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK", true},
    {WM_MOUSEWHEEL, "WM_MOUSEWHEEL", true},
    {WM_XBUTTONDOWN, "WM_XBUTTONDOWN", true},
    {WM_XBUTTONUP, "WM_XBUTTONUP", true},
    {WM_XBUTTONDBLCLK, "WM_XBUTTONDBLCLK", true},
    {WM_MOUSEHWHEEL, "WM_MOUSEHWHEEL", true},
    {WM_IME_SETCONTEXT, "WM_IME_SETCONTEXT", true},
    {WM_INPUTLANGCHANGE, "WM_INPUTLANGCHANGE", true},
    {WM_IME_NOTIFY, "WM_IME_NOTIFY", true},
#if defined(WM_DWMNCRENDERINGCHANGED)
    {WM_DWMNCRENDERINGCHANGED, "WM_DWMNCRENDERINGCHANGED", true},
#endif
    {WM_IME_SETCONTEXT, "WM_IME_SETCONTEXT", true},
    {WM_IME_NOTIFY, "WM_IME_NOTIFY", true},
    {WM_RENDERFORMAT, "WM_RENDERFORMAT", true},
    {WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS", true},
    {WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD", true},
    {WM_CAPTURECHANGED, "WM_CAPTURECHANGED", true},
    {WM_IME_STARTCOMPOSITION, "WM_IME_STARTCOMPOSITION", true},
    {WM_IME_COMPOSITION, "WM_IME_COMPOSITION", true},
    {WM_IME_ENDCOMPOSITION, "WM_IME_ENDCOMPOSITION", true},
    {WM_IME_NOTIFY, "WM_IME_NOTIFY", true},
    {WM_IME_REQUEST, "WM_IME_REQUEST", true},

#if ! defined(QT_NO_SESSIONMANAGER)
    {WM_QUERYENDSESSION, "WM_QUERYENDSESSION", true},
    {WM_ENDSESSION, "WM_ENDSESSION", true},
#endif

    {WM_MOUSEACTIVATE,"WM_MOUSEACTIVATE", true},
    {WM_CHILDACTIVATE, "WM_CHILDACTIVATE", true},
    {WM_PARENTNOTIFY, "WM_PARENTNOTIFY", true},
    {WM_ENTERIDLE, "WM_ENTERIDLE", false},
    {WM_GETMINMAXINFO, "WM_GETMINMAXINFO", true},
    {WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING", true},
    {WM_NCCREATE, "WM_NCCREATE", true},
    {WM_NCCALCSIZE, "WM_NCCALCSIZE", true},
    {WM_NCACTIVATE, "WM_NCACTIVATE", true},
    {WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE", true},
    {WM_NCMOUSELEAVE, "WM_NCMOUSELEAVE", true},
    {WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN", true},
    {WM_NCLBUTTONUP, "WM_NCLBUTTONUP", true},
    {WM_ACTIVATEAPP, "WM_ACTIVATEAPP", true},
    {WM_NCPAINT, "WM_NCPAINT", true},
    {WM_ERASEBKGND, "WM_ERASEBKGND", true},
    {WM_MOUSEMOVE, "WM_MOUSEMOVE", true},
    {WM_MOUSELEAVE, "WM_MOUSELEAVE", true},
    {WM_NCHITTEST, "WM_NCHITTEST", false},

#ifdef WM_TOUCH
    {WM_TOUCH, "WM_TOUCH", true},
#endif

    {WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN", true},
    {WM_DISPLAYCHANGE, "WM_DISPLAYCHANGE", true},
    {WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD", true},
    {WM_THEMECHANGED, "WM_THEMECHANGED", true}
};

static inline const MessageDebugEntry *messageDebugEntry(UINT msg)
{
    for (size_t i = 0; i < sizeof(messageDebugEntries)/sizeof(MessageDebugEntry); i++)
        if (messageDebugEntries[i].message == msg)
            return messageDebugEntries + i;

    return nullptr;
}

const char *QWindowsGuiEventDispatcher::windowsMessageName(UINT msg)
{
    if (const MessageDebugEntry *e = messageDebugEntry(msg))
        return e->description;
    return "Unknown";
}

