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

#ifndef QDRI2CONTEXT_H
#define QDRI2CONTEXT_H

#include <QtGui/QPlatformGLContext>

class QXcbWindow;
class QDri2ContextPrivate;

struct xcb_dri2_dri2_buffer_t;

class QDri2Context : public QPlatformGLContext
{
    Q_DECLARE_PRIVATE(QDri2Context);
public:
    QDri2Context(QXcbWindow *window);
    ~QDri2Context();

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();
    void* getProcAddress(const QString& procName);

    void resize(const QSize &size);

    QPlatformWindowFormat platformWindowFormat() const;

    void *eglContext() const;

protected:
    xcb_dri2_dri2_buffer_t *backBuffer();
    QScopedPointer<QDri2ContextPrivate> d_ptr;
private:
    Q_DISABLE_COPY(QDri2Context)
};

#endif // QDRI2CONTEXT_H
