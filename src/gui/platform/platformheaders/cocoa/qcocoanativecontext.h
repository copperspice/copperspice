/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QCOCOANATIVECONTEXT_H
#define QCOCOANATIVECONTEXT_H

#include <QtCore/QMetaType>
#include <AppKit/NSOpenGL.h>

struct QCocoaNativeContext
{
    QCocoaNativeContext()
        : m_context(0)
    { }

    QCocoaNativeContext(NSOpenGLContext *ctx)
        : m_context(ctx)
    { }

    NSOpenGLContext *context() const { return m_context; }

private:
    NSOpenGLContext *m_context;
};

Q_DECLARE_METATYPE(QCocoaNativeContext)

#endif
