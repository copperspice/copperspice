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

#include "qopenglfunctions_1_1.h"
#include "qopenglcontext.h"

QOpenGLFunctions_1_1::QOpenGLFunctions_1_1()
 : QAbstractOpenGLFunctions()
 , d_1_0_Core(nullptr)
 , d_1_1_Core(nullptr)
 , d_1_0_Deprecated(nullptr)
 , d_1_1_Deprecated(nullptr)
{
}

QOpenGLFunctions_1_1::~QOpenGLFunctions_1_1()
{
    if (d_1_0_Core && !d_1_0_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_0_Core->context, QOpenGLFunctions_1_0_CoreBackend::versionStatus());
        delete d_1_0_Core;
    }
    if (d_1_1_Core && !d_1_1_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_1_Core->context, QOpenGLFunctions_1_1_CoreBackend::versionStatus());
        delete d_1_1_Core;
    }
    if (d_1_0_Deprecated && !d_1_0_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_0_Deprecated->context, QOpenGLFunctions_1_0_DeprecatedBackend::versionStatus());
        delete d_1_0_Deprecated;
    }
    if (d_1_1_Deprecated && !d_1_1_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_1_Deprecated->context, QOpenGLFunctions_1_1_DeprecatedBackend::versionStatus());
        delete d_1_1_Deprecated;
    }
}

bool QOpenGLFunctions_1_1::initializeOpenGLFunctions()
{
    if ( isInitialized() )
        return true;

    QOpenGLContext* context = QOpenGLContext::currentContext();

    // If owned by a context object make sure it is current.
    // Also check that current context is capable of resolving all needed functions
    if (((owningContext() && owningContext() == context) || !owningContext())
        && QOpenGLFunctions_1_1::isContextCompatible(context))
    {
        // Associate with private implementation, creating if necessary
        // Function pointers in the backends are resolved at creation time
        QOpenGLVersionFunctionsBackend *d = nullptr;
        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_0_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_0_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_0_CoreBackend::versionStatus(), d);
        }
        d_1_0_Core = static_cast<QOpenGLFunctions_1_0_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_1_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_1_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_1_CoreBackend::versionStatus(), d);
        }
        d_1_1_Core = static_cast<QOpenGLFunctions_1_1_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_0_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_0_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_0_DeprecatedBackend::versionStatus(), d);
        }
        d_1_0_Deprecated = static_cast<QOpenGLFunctions_1_0_DeprecatedBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_1_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_1_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_1_DeprecatedBackend::versionStatus(), d);
        }
        d_1_1_Deprecated = static_cast<QOpenGLFunctions_1_1_DeprecatedBackend*>(d);
        d->refs.ref();

        QAbstractOpenGLFunctions::initializeOpenGLFunctions();
    }
    return isInitialized();
}

bool QOpenGLFunctions_1_1::isContextCompatible(QOpenGLContext *context)
{
    Q_ASSERT(context);
    QSurfaceFormat f = context->format();
    const QPair<int, int> v = qMakePair(f.majorVersion(), f.minorVersion());
    if (v < qMakePair(1, 1))
        return false;

    if (f.profile() == QSurfaceFormat::CoreProfile)
        return false;

    return true;
}

QOpenGLVersionProfile QOpenGLFunctions_1_1::versionProfile()
{
    QOpenGLVersionProfile v;
    v.setVersion(1, 1);
    return v;
}

