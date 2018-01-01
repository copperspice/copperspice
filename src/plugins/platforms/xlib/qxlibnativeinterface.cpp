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

#include <qxlibnativeinterface.h>
#include <qxlibdisplay.h>
#include <qapplication_p.h>

class QXlibResourceMap : public QMap<QByteArray, QXlibNativeInterface::ResourceType>
{
public:
    QXlibResourceMap()
        :QMap<QByteArray, QXlibNativeInterface::ResourceType>()
    {
        insert("display",QXlibNativeInterface::Display);
        insert("egldisplay",QXlibNativeInterface::EglDisplay);
        insert("connection",QXlibNativeInterface::Connection);
        insert("screen",QXlibNativeInterface::Screen);
        insert("graphicsdevice",QXlibNativeInterface::GraphicsDevice);
        insert("eglcontext",QXlibNativeInterface::EglContext);
    }
};

Q_GLOBAL_STATIC(QXlibResourceMap, qXlibResourceMap)


void * QXlibNativeInterface::nativeResourceForWidget(const QByteArray &resourceString, QWidget *widget)
{
    QByteArray lowerCaseResource = resourceString.toLower();
    ResourceType resource = qXlibResourceMap()->value(lowerCaseResource);
    void *result = 0;
    switch(resource) {
    case Display:
        result = displayForWidget(widget);
        break;
    case EglDisplay:
        result = eglDisplayForWidget(widget);
        break;
    case Connection:
        result = connectionForWidget(widget);
        break;
    case Screen:
        result = reinterpret_cast<void *>(qPlatformScreenForWidget(widget)->xScreenNumber());
        break;
    case GraphicsDevice:
        result = graphicsDeviceForWidget(widget);
        break;
    case EglContext:
        result = eglContextForWidget(widget);
        break;
    default:
        result = 0;
    }
    return result;
}

void * QXlibNativeInterface::displayForWidget(QWidget *widget)
{
    return qPlatformScreenForWidget(widget)->display()->nativeDisplay();
}

void * QXlibNativeInterface::eglDisplayForWidget(QWidget *widget)
{
    Q_UNUSED(widget);
    return 0;
}

void * QXlibNativeInterface::screenForWidget(QWidget *widget)
{
    Q_UNUSED(widget);
    return 0;
}

void * QXlibNativeInterface::graphicsDeviceForWidget(QWidget *widget)
{
    Q_UNUSED(widget);
    return 0;
}

void * QXlibNativeInterface::eglContextForWidget(QWidget *widget)
{
    Q_UNUSED(widget);
    return 0;
}

QXlibScreen * QXlibNativeInterface::qPlatformScreenForWidget(QWidget *widget)
{
    QXlibScreen *screen;
    if (widget) {
        screen = static_cast<QXlibScreen *>(QPlatformScreen::platformScreenForWidget(widget));
    }else {
        screen = static_cast<QXlibScreen *>(QApplicationPrivate::platformIntegration()->screens()[0]);
    }
    return screen;
}
