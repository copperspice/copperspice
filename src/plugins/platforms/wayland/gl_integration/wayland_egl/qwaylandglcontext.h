/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QWAYLANDGLCONTEXT_H
#define QWAYLANDGLCONTEXT_H

#include "qwaylanddisplay.h"

#include <QtGui/QPlatformGLContext>

#include "qwaylandeglinclude.h"

class QWaylandWindow;
class QWaylandGLWindowSurface;

class QWaylandGLContext : public QPlatformGLContext {
public:
    QWaylandGLContext(EGLDisplay eglDisplay, const QPlatformWindowFormat &format);
    ~QWaylandGLContext();
    void makeCurrent();
    void doneCurrent();
    void swapBuffers();
    void* getProcAddress(const QString&);

    QPlatformWindowFormat platformWindowFormat() const { return mFormat; }

    void setEglSurface(EGLSurface surface);
    EGLConfig eglConfig() const;
private:
    EGLDisplay mEglDisplay;

    EGLContext mContext;
    EGLSurface mSurface;
    EGLConfig mConfig;
    QPlatformWindowFormat mFormat;

    QWaylandGLContext();

};


#endif // QWAYLANDGLCONTEXT_H
