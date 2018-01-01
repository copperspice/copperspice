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

#ifndef QGLXINTEGRATION_H
#define QGLXINTEGRATION_H

#include "qxcbwindow.h"

#include <QtGui/QPlatformGLContext>
#include <QtGui/QPlatformWindowFormat>

#include <QtCore/QMutex>

#include <GL/glx.h>

class QGLXContext : public QPlatformGLContext
{
public:
    QGLXContext(Window window, QXcbScreen *xd, const QPlatformWindowFormat &format);
    ~QGLXContext();

    virtual void makeCurrent();
    virtual void doneCurrent();
    virtual void swapBuffers();
    virtual void* getProcAddress(const QString& procName);

    GLXContext glxContext() const { return m_context; }

    QPlatformWindowFormat platformWindowFormat() const;

private:
    QXcbScreen *m_screen;
    Drawable m_drawable;
    GLXContext m_context;
    QPlatformWindowFormat m_windowFormat;

    QGLXContext (QXcbScreen *screen, Drawable drawable, GLXContext context);
};

#endif
