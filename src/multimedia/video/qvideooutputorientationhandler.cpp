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

#include <qvideooutputorientationhandler_p.h>

#include <QGuiApplication>
#include <QScreen>

QVideoOutputOrientationHandler::QVideoOutputOrientationHandler(QObject *parent)
    : QObject(parent), m_currentOrientation(0)
{
    QScreen *screen = QGuiApplication::primaryScreen();

    // we want to be informed about all orientation changes
    screen->setOrientationUpdateMask(Qt::PortraitOrientation|Qt::LandscapeOrientation
                  | Qt::InvertedPortraitOrientation|Qt::InvertedLandscapeOrientation);

    connect(screen, &QScreen::orientationChanged, this, &QVideoOutputOrientationHandler::screenOrientationChanged);

    screenOrientationChanged(screen->orientation());
}

int QVideoOutputOrientationHandler::currentOrientation() const
{
    return m_currentOrientation;
}

void QVideoOutputOrientationHandler::screenOrientationChanged(Qt::ScreenOrientation orientation)
{
    const QScreen *screen = QGuiApplication::primaryScreen();

    const int angle = (360 - screen->angleBetween(screen->nativeOrientation(), orientation)) % 360;

    if (angle == m_currentOrientation)
        return;

    m_currentOrientation = angle;
    emit orientationChanged(m_currentOrientation);
}

