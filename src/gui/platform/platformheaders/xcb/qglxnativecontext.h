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

#ifndef QGLXNATIVECONTEXT_H
#define QGLXNATIVECONTEXT_H

#include <X11/Xlib.h>
#include <GL/glx.h>

struct QGLXNativeContext
{
   QGLXNativeContext()
      : m_context(nullptr), m_display(nullptr), m_window(0), m_visualId(0)
   {
   }

   QGLXNativeContext(GLXContext ctx, Display *dpy = nullptr, Window wnd = 0, VisualID vid = 0)
      : m_context(ctx), m_display(dpy), m_window(wnd), m_visualId(vid)
   {
   }

   GLXContext context() const { return m_context; }
   Display *display() const { return m_display; }
   Window window() const { return m_window; }
   VisualID visualId() const { return m_visualId; }

private:
   GLXContext m_context;
   Display *m_display;
   Window m_window;
   VisualID m_visualId;
};

CS_DECLARE_METATYPE(QGLXNativeContext)

#endif
