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

#ifndef QWGLNATIVECONTEXT_H
#define QWGLNATIVECONTEXT_H

#include <wingdi.h>
#include <GL/gl.h>

class QWGLNativeContext
{
public:
   QWGLNativeContext()
      : m_context(nullptr), m_window(nullptr)
   { }

   QWGLNativeContext(HGLRC ctx, HWND wnd)
      : m_context(ctx), m_window(wnd)
   {
   }

   HGLRC context() const { return m_context; }
   HWND window() const { return m_window; }

private:
   HGLRC m_context;
   HWND m_window;
};

CS_DECLARE_METATYPE(QWGLNativeContext)

#endif
