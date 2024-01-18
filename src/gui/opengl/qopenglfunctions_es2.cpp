/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#include <qopenglfunctions_es2.h>
#include <qopenglcontext.h>

#if defined(QT_OPENGL_ES_2)

QOpenGLFunctions_ES2::QOpenGLFunctions_ES2()
 : QAbstractOpenGLFunctions()
 , d_es2(0)
{
}

QOpenGLFunctions_ES2::~QOpenGLFunctions_ES2()
{
}

bool QOpenGLFunctions_ES2::initializeOpenGLFunctions()
{
    if ( isInitialized() )
        return true;

    QOpenGLContext* context = QOpenGLContext::currentContext();

    // If owned by a context object make sure it is current.
    // Also check that current context is compatible
    if (((owningContext() && owningContext() == context) || !owningContext())
        && QOpenGLFunctions_ES2::isContextCompatible(context))
    {
        // Nothing to do, just flag that we are initialized
        QAbstractOpenGLFunctions::initializeOpenGLFunctions();
    }
    return isInitialized();
}

bool QOpenGLFunctions_ES2::isContextCompatible(QOpenGLContext *context)
{
    Q_ASSERT(context);
    QSurfaceFormat f = context->format();
    const QPair<int, int> v = qMakePair(f.majorVersion(), f.minorVersion());
    if (v < qMakePair(2, 0))
        return false;
    if (f.renderableType() != QSurfaceFormat::OpenGLES)
        return false;

    return true;
}

QOpenGLVersionProfile QOpenGLFunctions_ES2::versionProfile()
{
    QOpenGLVersionProfile v;
    return v;
}

#endif
