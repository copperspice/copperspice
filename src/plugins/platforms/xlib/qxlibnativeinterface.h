/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QXLIBNATIVEINTERFACE_H
#define QXLIBNATIVEINTERFACE_H

#include "qxlibscreen.h"

#include <QtGui/QPlatformNativeInterface>

class QXlibNativeInterface : public QPlatformNativeInterface
{
public:
    enum ResourceType {
        Display,
        EglDisplay,
        Connection,
        Screen,
        GraphicsDevice,
        EglContext
    };

    void *nativeResourceForWidget(const QByteArray &resourceString, QWidget *widget);

    void *displayForWidget(QWidget *widget);
    void *eglDisplayForWidget(QWidget *widget);
    void *connectionForWidget(QWidget *widget);
    void *screenForWidget(QWidget *widget);
    void *graphicsDeviceForWidget(QWidget *widget);
    void *eglContextForWidget(QWidget *widget);

private:
    static QXlibScreen *qPlatformScreenForWidget(QWidget *widget);
};


#endif // QXLIBNATIVEINTERFACE_H
