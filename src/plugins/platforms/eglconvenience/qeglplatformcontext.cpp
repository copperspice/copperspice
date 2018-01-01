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

#include "qeglplatformcontext.h"


#include <QtGui/QPlatformWindow>

#include "qeglconvenience.h"

#include <EGL/egl.h>

QEGLPlatformContext::QEGLPlatformContext(EGLDisplay display, EGLConfig config, EGLint contextAttrs[], EGLSurface surface, EGLenum eglApi, QEGLPlatformContext *shareContext)
    : QPlatformGLContext()
    , m_eglDisplay(display)
    , m_eglSurface(surface)
    , m_eglApi(eglApi)
{
    if (m_eglSurface == EGL_NO_SURFACE) {
        qWarning("Createing QEGLPlatformContext with no surface");
    }

    eglBindAPI(m_eglApi);
    EGLContext shareEglContext = shareContext ? shareContext->eglContext() : 0;
    m_eglContext = eglCreateContext(m_eglDisplay,config, shareEglContext, contextAttrs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        qWarning("Could not create the egl context\n");
        eglTerminate(m_eglDisplay);
        qFatal("EGL error");
    }

    m_windowFormat = qt_qPlatformWindowFormatFromConfig(display,config);
}

QEGLPlatformContext::~QEGLPlatformContext()
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglContext::~QEglContext(): %p\n",this);
#endif
    if (m_eglSurface != EGL_NO_SURFACE) {
        doneCurrent();
        eglDestroySurface(m_eglDisplay, m_eglSurface);
        m_eglSurface = EGL_NO_SURFACE;
    }

    if (m_eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(m_eglDisplay, m_eglContext);
        m_eglContext = EGL_NO_CONTEXT;
    }
}

void QEGLPlatformContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglContext::makeCurrent: %p\n",this);
#endif
    eglBindAPI(m_eglApi);
    bool ok = eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    if (!ok)
        qWarning("QEGLPlatformContext::makeCurrent: eglError: %d, this: %p \n", eglGetError(), this);
#ifdef QEGL_EXTRA_DEBUG
    static bool showDebug = true;
    if (showDebug) {
        showDebug = false;
        const char *str = (const char*)glGetString(GL_VENDOR);
        qWarning("Vendor %s\n", str);
        str = (const char*)glGetString(GL_RENDERER);
        qWarning("Renderer %s\n", str);
        str = (const char*)glGetString(GL_VERSION);
        qWarning("Version %s\n", str);

        str = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        qWarning("Extensions %s\n",str);

        str = (const char*)glGetString(GL_EXTENSIONS);
        qWarning("Extensions %s\n", str);

    }
#endif
}
void QEGLPlatformContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglContext::doneCurrent:%p\n",this);
#endif
    eglBindAPI(m_eglApi);
    bool ok = eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (!ok)
        qWarning("QEGLPlatformContext::doneCurrent(): eglError: %d, this: %p \n", eglGetError(), this);
}
void QEGLPlatformContext::swapBuffers()
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglContext::swapBuffers:%p\n",this);
#endif
    eglBindAPI(m_eglApi);
    bool ok = eglSwapBuffers(m_eglDisplay, m_eglSurface);
    if (!ok)
        qWarning("QEGLPlatformContext::swapBuffers(): eglError: %d, this: %p \n", eglGetError(), this);
}
void* QEGLPlatformContext::getProcAddress(const QString& procName)
{
#ifdef QEGL_EXTRA_DEBUG
    qWarning("QEglContext::getProcAddress%p\n",this);
#endif
    eglBindAPI(m_eglApi);
    return (void *)eglGetProcAddress(qPrintable(procName));
}

QPlatformWindowFormat QEGLPlatformContext::platformWindowFormat() const
{
    return m_windowFormat;
}

EGLContext QEGLPlatformContext::eglContext() const
{
    return m_eglContext;
}
