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

#ifndef QGLXCONVENIENCE_H
#define QGLXCONVENIENCE_H

#include <QPlatformWindowFormat>

#include <X11/Xlib.h>
#include <GL/glx.h>

XVisualInfo *qglx_findVisualInfo(Display *display, int screen, const QPlatformWindowFormat &format);
GLXFBConfig qglx_findConfig(Display *display, int screen, const QPlatformWindowFormat &format, int drawableBit = GLX_WINDOW_BIT);
QPlatformWindowFormat qglx_platformWindowFromGLXFBConfig(Display *display, GLXFBConfig config, GLXContext context);
QVector<int> qglx_buildSpec(const QPlatformWindowFormat &format, int drawableBit = GLX_WINDOW_BIT);
QPlatformWindowFormat qglx_reducePlatformWindowFormat(const QPlatformWindowFormat &format, bool *reduced);

#endif // QGLXCONVENIENCE_H
