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

#ifndef QOPENKODEGLINTEGRATION_H
#define QOPENKODEGLINTEGRATION_H

#include <QtGui/QPlatformGLContext>
#include <EGL/egl.h>

class QEGLPlatformContext : public QPlatformGLContext
{
public:
    QEGLPlatformContext(EGLDisplay display, EGLConfig config, EGLint contextAttrs[], EGLSurface surface, EGLenum eglApi, QEGLPlatformContext *shareContext = 0);
    ~QEGLPlatformContext();

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();
    void* getProcAddress(const QString& procName);

    QPlatformWindowFormat platformWindowFormat() const;

    EGLContext eglContext() const;
private:
    EGLContext m_eglContext;
    EGLDisplay m_eglDisplay;
    EGLSurface m_eglSurface;
    EGLenum m_eglApi;

    QPlatformWindowFormat m_windowFormat;
};

#endif //QOPENKODEGLINTEGRATION_H
