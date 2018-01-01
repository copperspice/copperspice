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

#ifndef QDIRECTFBGLCONTEXT_H
#define QDIRECTFBGLCONTEXT_H

#include <QPlatformGLContext>

#include "qdirectfbconvenience.h"

class QDirectFbGLContext : public QPlatformGLContext
{
public:
    explicit QDirectFbGLContext(IDirectFBGL *glContext);

    void makeCurrent();
    void doneCurrent();
    void swapBuffers();
    void *getProcAddress(const QString &procName);

    QPlatformWindowFormat platformWindowFormat() const;


private:
    IDirectFBGL *m_dfbGlContext;

    QPlatformWindowFormat m_windowFormat;

};

#endif // QDIRECTFBGLCONTEXT_H
