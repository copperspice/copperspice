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

#ifndef QGLX_CONVENIENCE_H
#define QGLX_CONVENIENCE_H

#include <qsurfaceformat.h>
#include <qvector.h>

#include <X11/Xlib.h>
#include <GL/glx.h>

XVisualInfo *qglx_findVisualInfo(Display *display, int screen, QSurfaceFormat *format);
GLXFBConfig qglx_findConfig(Display *display, int screen, const QSurfaceFormat &format, int drawableBit = GLX_WINDOW_BIT);
void qglx_surfaceFormatFromGLXFBConfig(QSurfaceFormat *format, Display *display, GLXFBConfig config);
void qglx_surfaceFormatFromVisualInfo(QSurfaceFormat *format, Display *display, XVisualInfo *visualInfo);

QVector<int> qglx_buildSpec(const QSurfaceFormat &format, int drawableBit = GLX_WINDOW_BIT);
QSurfaceFormat qglx_reduceSurfaceFormat(const QSurfaceFormat &format, bool *reduced);

#endif
