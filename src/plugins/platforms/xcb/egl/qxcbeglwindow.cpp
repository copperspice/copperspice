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

#include "qxcbeglwindow.h"

#include "qxcbeglintegration.h"

#include <qeglconvenience_p.h>
#include <qxlibeglintegration_p.h>

QXcbEglWindow::QXcbEglWindow(QWindow *window, QXcbEglIntegration *glIntegration)
   : QXcbWindow(window)
   , m_glIntegration(glIntegration)
   , m_config(nullptr)
   , m_surface(EGL_NO_SURFACE)
{
}

QXcbEglWindow::~QXcbEglWindow()
{
   eglDestroySurface(m_glIntegration->eglDisplay(), m_surface);
}

void QXcbEglWindow::resolveFormat()
{
   m_config = q_configFromGLFormat(m_glIntegration->eglDisplay(), window()->requestedFormat(), true);
   m_format = q_glFormatFromConfig(m_glIntegration->eglDisplay(), m_config, m_format);
}

void *QXcbEglWindow::createVisual()
{
#ifdef XCB_USE_XLIB
   Display *xdpy = static_cast<Display *>(m_glIntegration->xlib_display());
   VisualID id = QXlibEglIntegration::getCompatibleVisualId(xdpy, m_glIntegration->eglDisplay(), m_config);

   XVisualInfo visualInfoTemplate;
   memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
   visualInfoTemplate.visualid = id;

   XVisualInfo *visualInfo;
   int matchingCount = 0;
   visualInfo = XGetVisualInfo(xdpy, VisualIDMask, &visualInfoTemplate, &matchingCount);
   return visualInfo;
#else
   return QXcbWindow::createVisual();
#endif
}

void QXcbEglWindow::create()
{
   QXcbWindow::create();

   m_surface = eglCreateWindowSurface(m_glIntegration->eglDisplay(), m_config, m_window, 0);
}

