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

#include <QDebug>
#include <QLibrary>
#include <QGLFormat>

#include "qxlibwindow.h"
#include "qxlibscreen.h"
#include "qxlibdisplay.h"
#include "qxlibstatic.h"

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#include "qglxintegration.h"

#include "qglxconvenience.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
#include <dlfcn.h>
#endif

QT_BEGIN_NAMESPACE

QGLXContext::QGLXContext(Window window, QXlibScreen *screen, const QPlatformWindowFormat &format)
    : QPlatformGLContext()
    , m_screen(screen)
    , m_drawable((Drawable)window)
    , m_context(0)
{

    const QPlatformGLContext *sharePlatformContext;
    sharePlatformContext = format.sharedGLContext();
    GLXContext shareGlxContext = 0;
    if (sharePlatformContext)
        shareGlxContext = static_cast<const QGLXContext*>(sharePlatformContext)->glxContext();

    GLXFBConfig config = qglx_findConfig(screen->display()->nativeDisplay(),screen->xScreenNumber(),format);
    m_context = glXCreateNewContext(screen->display()->nativeDisplay(),config,GLX_RGBA_TYPE,shareGlxContext,TRUE);
    m_windowFormat = qglx_platformWindowFromGLXFBConfig(screen->display()->nativeDisplay(),config,m_context);

#ifdef MYX11_DEBUG
    qDebug() << "QGLXGLContext::create context" << m_context;
#endif
}

QGLXContext::QGLXContext(QXlibScreen *screen, Drawable drawable, GLXContext context)
    : QPlatformGLContext(), m_screen(screen), m_drawable(drawable), m_context(context)
{

}

QGLXContext::~QGLXContext()
{
    if (m_context) {
        qDebug("Destroying GLX context 0x%p", m_context);
        glXDestroyContext(m_screen->display()->nativeDisplay(), m_context);
    }
}

void QGLXContext::makeCurrent()
{
    QPlatformGLContext::makeCurrent();
#ifdef MYX11_DEBUG
    qDebug("QGLXGLContext::makeCurrent(window=0x%x, ctx=0x%x)", m_drawable, m_context);
#endif
    glXMakeCurrent(m_screen->display()->nativeDisplay(), m_drawable, m_context);
}

void QGLXContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();
    glXMakeCurrent(m_screen->display()->nativeDisplay(), 0, 0);
}

void QGLXContext::swapBuffers()
{
    glXSwapBuffers(m_screen->display()->nativeDisplay(), m_drawable);
}

void* QGLXContext::getProcAddress(const QString& procName)
{
    typedef void *(*qt_glXGetProcAddressARB)(const GLubyte *);
    static qt_glXGetProcAddressARB glXGetProcAddressARB = 0;
    static bool resolved = false;

    if (resolved && !glXGetProcAddressARB)
        return 0;
    if (!glXGetProcAddressARB) {
        QList<QByteArray> glxExt = QByteArray(glXGetClientString(m_screen->display()->nativeDisplay(), GLX_EXTENSIONS)).split(' ');
        if (glxExt.contains("GLX_ARB_get_proc_address")) {
#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
            void *handle = dlopen(NULL, RTLD_LAZY);
            if (handle) {
                glXGetProcAddressARB = (qt_glXGetProcAddressARB) dlsym(handle, "glXGetProcAddressARB");
                dlclose(handle);
            }
            if (!glXGetProcAddressARB)
#endif
            {
                extern const QString qt_gl_library_name();
//                QLibrary lib(qt_gl_library_name());
                QLibrary lib(QLatin1String("GL"));
                glXGetProcAddressARB = (qt_glXGetProcAddressARB) lib.resolve("glXGetProcAddressARB");
            }
        }
        resolved = true;
    }
    if (!glXGetProcAddressARB)
        return 0;
    return glXGetProcAddressARB(reinterpret_cast<const GLubyte *>(procName.toLatin1().data()));
}

QPlatformWindowFormat QGLXContext::platformWindowFormat() const
{
    return m_windowFormat;
}

QT_END_NAMESPACE

#endif //!defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
