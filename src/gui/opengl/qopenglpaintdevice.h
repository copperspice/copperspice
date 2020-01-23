/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QOPENGLPAINTDEVICE_H
#define QOPENGLPAINTDEVICE_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qpaintdevice.h>
#include <qopengl.h>
#include <qopenglcontext.h>

class QOpenGLPaintDevicePrivate;

class Q_GUI_EXPORT QOpenGLPaintDevice : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QOpenGLPaintDevice)

public:
    QOpenGLPaintDevice();
    explicit QOpenGLPaintDevice(const QSize &size);
    QOpenGLPaintDevice(int width, int height);
    virtual ~QOpenGLPaintDevice();

    int devType() const { return QInternal::OpenGL; }
    QPaintEngine *paintEngine() const;

    QOpenGLContext *context() const;
    QSize size() const;
    void setSize(const QSize &size);
    void setDevicePixelRatio(qreal devicePixelRatio);

    qreal dotsPerMeterX() const;
    qreal dotsPerMeterY() const;

    void setDotsPerMeterX(qreal);
    void setDotsPerMeterY(qreal);

    void setPaintFlipped(bool flipped);
    bool paintFlipped() const;

    virtual void ensureActiveTarget();

protected:
    QOpenGLPaintDevice(QOpenGLPaintDevicePrivate &dd);
    int metric(QPaintDevice::PaintDeviceMetric metric) const;

    Q_DISABLE_COPY(QOpenGLPaintDevice)
    QScopedPointer<QOpenGLPaintDevicePrivate> d_ptr;
};

#endif // QT_NO_OPENGL

#endif
