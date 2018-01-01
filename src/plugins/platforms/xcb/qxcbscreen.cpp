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

#include "qxcbscreen.h"

#include <stdio.h>

QXcbScreen::QXcbScreen(QXcbConnection *connection, xcb_screen_t *screen, int number)
    : QXcbObject(connection)
    , m_screen(screen)
    , m_number(number)
{
    printf ("\n");
    printf ("Information of screen %d:\n", screen->root);
    printf ("  width.........: %d\n", screen->width_in_pixels);
    printf ("  height........: %d\n", screen->height_in_pixels);
    printf ("  depth.........: %d\n", screen->root_depth);
    printf ("  white pixel...: %x\n", screen->white_pixel);
    printf ("  black pixel...: %x\n", screen->black_pixel);
    printf ("\n");

    const quint32 mask = XCB_CW_EVENT_MASK;
    const quint32 values[] = {
        // XCB_CW_EVENT_MASK
        XCB_EVENT_MASK_ENTER_WINDOW
        | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_PROPERTY_CHANGE
    };

    xcb_change_window_attributes(xcb_connection(), screen->root, mask, values);

    xcb_generic_error_t *error;

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(xcb_connection(),
            xcb_get_property(xcb_connection(), false, screen->root,
                             atom(QXcbAtom::_NET_SUPPORTING_WM_CHECK),
                             XCB_ATOM_WINDOW, 0, 1024), &error);

    if (reply && reply->format == 32 && reply->type == XCB_ATOM_WINDOW) {
        xcb_window_t windowManager = *((xcb_window_t *)xcb_get_property_value(reply));

        if (windowManager != XCB_WINDOW_NONE) {
            xcb_get_property_reply_t *windowManagerReply =
                xcb_get_property_reply(xcb_connection(),
                    xcb_get_property(xcb_connection(), false, windowManager,
                                     atom(QXcbAtom::_NET_WM_NAME),
                                     atom(QXcbAtom::UTF8_STRING), 0, 1024), &error);
            if (windowManagerReply && windowManagerReply->format == 8 && windowManagerReply->type == atom(QXcbAtom::UTF8_STRING)) {
                m_windowManagerName = QString::fromUtf8((const char *)xcb_get_property_value(windowManagerReply), xcb_get_property_value_length(windowManagerReply));
                printf("Running window manager: %s\n", qPrintable(m_windowManagerName));
            } else if (error) {
                connection->handleXcbError(error);
                free(error);
            }

            free(windowManagerReply);
        }
    } else if (error) {
        connection->handleXcbError(error);
        free(error);
    }

    free(reply);

    m_syncRequestSupported = m_windowManagerName != QLatin1String("KWin");
}

QXcbScreen::~QXcbScreen()
{
}

QRect QXcbScreen::geometry() const
{
    return QRect(0, 0, m_screen->width_in_pixels, m_screen->height_in_pixels);
}

int QXcbScreen::depth() const
{
    return m_screen->root_depth;
}

QImage::Format QXcbScreen::format() const
{
    return QImage::Format_RGB32;
}

QSize QXcbScreen::physicalSize() const
{
    return QSize(m_screen->width_in_millimeters, m_screen->height_in_millimeters);
}

int QXcbScreen::screenNumber() const
{
    return m_number;
}
