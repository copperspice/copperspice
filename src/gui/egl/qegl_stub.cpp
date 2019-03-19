/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <QtGui/qpaintdevice.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qwidget.h>
#include <QtCore/qdebug.h>

#include <qegl_p.h>
#include <qeglcontext_p.h>

QT_BEGIN_NAMESPACE

static void noegl(const char *fn)
{
   qWarning() << fn << " called, but Qt configured without EGL" << endl;
}

#define NOEGL noegl(__FUNCTION__);

QEglContext::QEglContext()
   : apiType(QEgl::OpenGL)
   , ctx(0)
   , cfg(QEGL_NO_CONFIG)
   , currentSurface(0)
   , current(false)
   , ownsContext(true)
   , sharing(false)
{
   NOEGL
}

QEglContext::~QEglContext()
{
   NOEGL
}

bool QEglContext::isValid() const
{
   NOEGL
   return false;
}

bool QEglContext::isCurrent() const
{
   NOEGL
   return false;
}

EGLConfig QEgl::defaultConfig(int devType, API api, ConfigOptions options)
{
   Q_UNUSED(devType)
   Q_UNUSED(api)
   Q_UNUSED(options)
   NOEGL
   return QEGL_NO_CONFIG;
}


// Choose a configuration that matches "properties".
EGLConfig QEgl::chooseConfig(const QEglProperties *properties, QEgl::PixelFormatMatch match)
{
   Q_UNUSED(properties)
   Q_UNUSED(match)
   NOEGL
   return QEGL_NO_CONFIG;
}

bool QEglContext::chooseConfig(const QEglProperties &properties, QEgl::PixelFormatMatch match)
{
   Q_UNUSED(properties)
   Q_UNUSED(match)
   NOEGL
   return false;
}

EGLSurface QEglContext::createSurface(QPaintDevice *device, const QEglProperties *properties)
{
   Q_UNUSED(device)
   Q_UNUSED(properties)
   NOEGL
   return 0;
}


// Create the EGLContext.
bool QEglContext::createContext(QEglContext *shareContext, const QEglProperties *properties)
{
   Q_UNUSED(shareContext)
   Q_UNUSED(properties)
   NOEGL
   return false;
}

// Destroy an EGL surface object.  If it was current on this context
// then call doneCurrent() for it first.
void QEglContext::destroySurface(EGLSurface surface)
{
   Q_UNUSED(surface)
   NOEGL
}

// Destroy the context.  Note: this does not destroy the surface.
void QEglContext::destroyContext()
{
   NOEGL
}

bool QEglContext::makeCurrent(EGLSurface surface)
{
   Q_UNUSED(surface)
   NOEGL
   return false;
}

bool QEglContext::doneCurrent()
{
   NOEGL
   return false;
}

// Act as though doneCurrent() was called, but keep the context
// and the surface active for the moment.  This allows makeCurrent()
// to skip a call to eglMakeCurrent() if we are using the same
// surface as the last set of painting operations.  We leave the
// currentContext() pointer as-is for now.
bool QEglContext::lazyDoneCurrent()
{
   NOEGL
   return false;
}

bool QEglContext::swapBuffers(EGLSurface surface)
{
   Q_UNUSED(surface)
   NOEGL
   return false;
}

bool QEglContext::swapBuffersRegion2NOK(EGLSurface surface, const QRegion *region)
{
   Q_UNUSED(surface)
   Q_UNUSED(region)
   NOEGL
   return false;
}

int QEglContext::configAttrib(int name) const
{
   Q_UNUSED(name)
   NOEGL
   return 0;
}

EGLDisplay QEgl::display()
{
   NOEGL
   return 0;
}

EGLImageKHR QEgl::eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
                                    const EGLint *attrib_list)
{
   Q_UNUSED(dpy)
   Q_UNUSED(ctx)
   Q_UNUSED(target)
   Q_UNUSED(buffer)
   Q_UNUSED(attrib_list)
   NOEGL
   return 0;
}

EGLBoolean QEgl::eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img)
{
   Q_UNUSED(dpy)
   Q_UNUSED(img)
   NOEGL
   return 0;
}

EGLBoolean QEgl::eglSwapBuffersRegion2NOK(EGLDisplay dpy, EGLSurface surface, EGLint count, const EGLint *rects)
{
   Q_UNUSED(dpy);
   Q_UNUSED(surface);
   Q_UNUSED(count);
   Q_UNUSED(rects);
   NOEGL
   return 0;
}

#ifndef Q_WS_X11
EGLSurface QEgl::createSurface(QPaintDevice *device, EGLConfig cfg, const QEglProperties *properties)
{
   Q_UNUSED(device)
   Q_UNUSED(cfg)
   Q_UNUSED(properties)
   NOEGL
   return 0;
}
#endif


// Return the error string associated with a specific code.
QString QEgl::errorString(EGLint code)
{
   Q_UNUSED(code)
   NOEGL
   return QString();
}

// Dump all of the EGL configurations supported by the system.
void QEgl::dumpAllConfigs()
{
   NOEGL
}

QString QEgl::extensions()
{
   NOEGL
   return QString();
}

bool QEgl::hasExtension(const char *extensionName)
{
   Q_UNUSED(extensionName)
   NOEGL
   return false;
}

QEglContext *QEglContext::currentContext(QEgl::API api)
{
   Q_UNUSED(api)
   NOEGL
   return false;
}

void QEglContext::setCurrentContext(QEgl::API api, QEglContext *context)
{
   Q_UNUSED(api)
   Q_UNUSED(context)
   NOEGL
}

EGLNativeDisplayType QEgl::nativeDisplay()
{
   NOEGL
   return 0;
}

EGLNativeWindowType QEgl::nativeWindow(QWidget *widget)
{
   Q_UNUSED(widget)
   NOEGL
   return (EGLNativeWindowType)0;
}

EGLNativePixmapType QEgl::nativePixmap(QPixmap *)
{
   NOEGL
   return (EGLNativePixmapType)0;
}

QT_END_NAMESPACE
