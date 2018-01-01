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

#ifndef Q_GLX_CONTEXT_H
#define Q_GLX_CONTEXT_H

#include "qxlibwindow.h"

#include <QtGui/QPlatformGLContext>
#include <QtGui/QPlatformWindowFormat>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#define Status int
#include <GL/glx.h>
#undef Status

QT_BEGIN_NAMESPACE

class QGLXContext : public QPlatformGLContext
{
public:
    QGLXContext(Window window, QXlibScreen *xd, const QPlatformWindowFormat &format);
    ~QGLXContext();

    virtual void makeCurrent();
    virtual void doneCurrent();
    virtual void swapBuffers();
    virtual void* getProcAddress(const QString& procName);

    GLXContext glxContext() const {return m_context;}

    QPlatformWindowFormat platformWindowFormat() const;

private:
    QXlibScreen  *m_screen;
    Drawable    m_drawable;
    GLXContext  m_context;
    QPlatformWindowFormat m_windowFormat;

    QGLXContext (QXlibScreen *screen, Drawable drawable, GLXContext context);
};

QT_END_NAMESPACE

#endif //!defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#endif
