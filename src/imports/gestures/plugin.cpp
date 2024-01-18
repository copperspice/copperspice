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

#include <QtDeclarative/qdeclarativeextensionplugin.h>
#include <QtDeclarative/qdeclarative.h>

#include "qdeclarativegesturearea_p.h"

QT_BEGIN_NAMESPACE

class GestureAreaQmlPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    virtual void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Qt.labs.gestures"));
#ifndef QT_NO_GESTURES
        qmlRegisterCustomType<QDeclarativeGestureArea>(uri,1,0, "GestureArea", new QDeclarativeGestureAreaParser);

        qmlRegisterUncreatableType<QGesture>(uri, 1, 0, "Gesture", QLatin1String("Do not create objects of this type."));
        qmlRegisterUncreatableType<QPanGesture>(uri, 1, 0, "PanGesture", QLatin1String("Do not create objects of this type."));
        qmlRegisterUncreatableType<QTapGesture>(uri, 1, 0, "TapGesture", QLatin1String("Do not create objects of this type."));
        qmlRegisterUncreatableType<QTapAndHoldGesture>(uri, 1, 0, "TapAndHoldGesture", QLatin1String("Do not create objects of this type."));
        qmlRegisterUncreatableType<QPinchGesture>(uri, 1, 0, "PinchGesture", QLatin1String("Do not create objects of this type."));
        qmlRegisterUncreatableType<QSwipeGesture>(uri, 1, 0, "SwipeGesture", QLatin1String("Do not create objects of this type."));
#endif
    }
};

QT_END_NAMESPACE

Q_EXPORT_PLUGIN2(qmlgesturesplugin, QT_PREPEND_NAMESPACE(GestureAreaQmlPlugin));
