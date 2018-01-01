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

#include "qxcbwindow.h"
#include "qxcbscreen.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include "qglxintegration.h"
#include "qglxconvenience.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
#include <dlfcn.h>
#endif

QGLXContext::QGLXContext(Window window, QXcbScreen *screen, const QPlatformWindowFormat &format)
    : QPlatformGLContext()
    , m_screen(screen)
    , m_drawable((Drawable)window)
    , m_context(0)
{
    Q_XCB_NOOP(m_screen->connection());
    const QPlatformGLContext *sharePlatformContext;
    sharePlatformContext = format.sharedGLContext();
    GLXContext shareGlxContext = 0;
    if (sharePlatformContext)
        shareGlxContext = static_cast<const QGLXContext*>(sharePlatformContext)->glxContext();

    GLXFBConfig config = qglx_findConfig(DISPLAY_FROM_XCB(screen),screen->screenNumber(),format);
    m_context = glXCreateNewContext(DISPLAY_FROM_XCB(screen), config, GLX_RGBA_TYPE, shareGlxContext, TRUE);
    m_windowFormat = qglx_platformWindowFromGLXFBConfig(DISPLAY_FROM_XCB(screen), config, m_context);
    Q_XCB_NOOP(m_screen->connection());
}

QGLXContext::QGLXContext(QXcbScreen *screen, Drawable drawable, GLXContext context)
    : QPlatformGLContext(), m_screen(screen), m_drawable(drawable), m_context(context)
{

}

QGLXContext::~QGLXContext()
{
    Q_XCB_NOOP(m_screen->connection());
    if (m_context)
        glXDestroyContext(DISPLAY_FROM_XCB(m_screen), m_context);
    Q_XCB_NOOP(m_screen->connection());
}

void QGLXContext::makeCurrent()
{
    Q_XCB_NOOP(m_screen->connection());
    QPlatformGLContext::makeCurrent();
    glXMakeCurrent(DISPLAY_FROM_XCB(m_screen), m_drawable, m_context);
    Q_XCB_NOOP(m_screen->connection());
}

void QGLXContext::doneCurrent()
{
    Q_XCB_NOOP(m_screen->connection());
    QPlatformGLContext::doneCurrent();
    glXMakeCurrent(DISPLAY_FROM_XCB(m_screen), 0, 0);
    Q_XCB_NOOP(m_screen->connection());
}

void QGLXContext::swapBuffers()
{
    Q_XCB_NOOP(m_screen->connection());
    glXSwapBuffers(DISPLAY_FROM_XCB(m_screen), m_drawable);
    Q_XCB_NOOP(m_screen->connection());
}

void* QGLXContext::getProcAddress(const QString& procName)
{
    Q_XCB_NOOP(m_screen->connection());
    typedef void *(*qt_glXGetProcAddressARB)(const GLubyte *);
    static qt_glXGetProcAddressARB glXGetProcAddressARB = 0;
    static bool resolved = false;

    if (resolved && !glXGetProcAddressARB)
        return 0;
    if (!glXGetProcAddressARB) {
        QList<QByteArray> glxExt = QByteArray(glXGetClientString(DISPLAY_FROM_XCB(m_screen), GLX_EXTENSIONS)).split(' ');
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
