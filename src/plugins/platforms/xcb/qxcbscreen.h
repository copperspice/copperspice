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

#ifndef QXCBSCREEN_H
#define QXCBSCREEN_H

#include <QtGui/QPlatformScreen>
#include <QtCore/QString>

#include <xcb/xcb.h>

#include "qxcbobject.h"

class QXcbConnection;

class QXcbScreen : public QXcbObject, public QPlatformScreen
{
public:
    QXcbScreen(QXcbConnection *connection, xcb_screen_t *screen, int number);
    ~QXcbScreen();

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;
    QSize physicalSize() const;

    int screenNumber() const;

    xcb_screen_t *screen() const { return m_screen; }
    xcb_window_t root() const { return m_screen->root; }

    QString windowManagerName() const { return m_windowManagerName; }
    bool syncRequestSupported() const { return m_syncRequestSupported; }

private:
    xcb_screen_t *m_screen;
    int m_number;
    QString m_windowManagerName;
    bool m_syncRequestSupported;
};

#endif
