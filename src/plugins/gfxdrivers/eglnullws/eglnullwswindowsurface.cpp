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

#include "eglnullwswindowsurface.h"
#include "eglnullwsscreenplugin.h"

#include <QGLWidget>

static const QWSWindowSurface::SurfaceFlags Flags
    = QWSWindowSurface::RegionReserved | QWSWindowSurface::RegionReserved;

EGLNullWSWindowSurface::EGLNullWSWindowSurface(QWidget *w)
    :
    QWSGLWindowSurface(w),
    widget(w)
{
    setSurfaceFlags(Flags);
}

EGLNullWSWindowSurface::EGLNullWSWindowSurface()
    : widget(0)
{
    setSurfaceFlags(Flags);
}

EGLNullWSWindowSurface::~EGLNullWSWindowSurface() {}

QString EGLNullWSWindowSurface::key() const
{
    return QLatin1String(PluginName);
}

QPaintDevice *EGLNullWSWindowSurface::paintDevice()
{
    return widget;
}

bool EGLNullWSWindowSurface::isValid() const
{
    return qobject_cast<QGLWidget *>(window());
}

QImage EGLNullWSWindowSurface::image() const
{
    return QImage();
}
