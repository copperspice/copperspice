/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#ifndef QOPENGL_PAINTDEVICE_P_H
#define QOPENGL_PAINTDEVICE_P_H

#include <qopengl_paintdevice.h>

class QOpenGLContext;
class QPaintEngine;

class Q_GUI_EXPORT QOpenGLPaintDevicePrivate
{
public:
    QOpenGLPaintDevicePrivate(const QSize &size);
    virtual ~QOpenGLPaintDevicePrivate();

    static QOpenGLPaintDevicePrivate *get(QOpenGLPaintDevice *dev) { return dev->d_func(); }

    virtual void beginPaint() { }
    virtual void endPaint() { }

public:
    QSize size;
    QOpenGLContext *ctx;

    qreal dpmx;
    qreal dpmy;
    qreal devicePixelRatio;

    bool flipped;

    QPaintEngine *engine;
};

#endif
