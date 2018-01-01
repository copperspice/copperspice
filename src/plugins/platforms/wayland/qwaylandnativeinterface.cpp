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

#include <qwaylandnativeinterface.h>
#include <qwaylanddisplay.h>
#include <qwaylandwindow.h>
#include <qapplication_p.h>

void *QWaylandNativeInterface::nativeResourceForWidget(const QByteArray &resourceString, QWidget *widget)
{
    QByteArray lowerCaseResource = resourceString.toLower();

    if (lowerCaseResource == "display")
	return qPlatformScreenForWidget(widget)->display()->wl_display();
    if (lowerCaseResource == "surface") {
	return ((QWaylandWindow *) widget->platformWindow())->wl_surface();
    }

    return NULL;
}


QWaylandScreen * QWaylandNativeInterface::qPlatformScreenForWidget(QWidget *widget)
{
    QWaylandScreen *screen;

    if (widget) {
        screen = static_cast<QWaylandScreen *>(QPlatformScreen::platformScreenForWidget(widget));
    } else {
        screen = static_cast<QWaylandScreen *>(QApplicationPrivate::platformIntegration()->screens()[0]);
    }
    return screen;
}
