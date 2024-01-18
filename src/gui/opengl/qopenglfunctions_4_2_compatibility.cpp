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

#include "qopenglfunctions_4_2_compatibility.h"
#include "qopenglcontext.h"

QOpenGLFunctions_4_2_Compatibility::QOpenGLFunctions_4_2_Compatibility()
 : QAbstractOpenGLFunctions()
 , d_1_0_Core(nullptr)
 , d_1_1_Core(nullptr)
 , d_1_2_Core(nullptr)
 , d_1_3_Core(nullptr)
 , d_1_4_Core(nullptr)
 , d_1_5_Core(nullptr)
 , d_2_0_Core(nullptr)
 , d_2_1_Core(nullptr)
 , d_3_0_Core(nullptr)
 , d_3_1_Core(nullptr)
 , d_3_2_Core(nullptr)
 , d_3_3_Core(nullptr)
 , d_4_0_Core(nullptr)
 , d_4_1_Core(nullptr)
 , d_4_2_Core(nullptr)
 , d_1_0_Deprecated(nullptr)
 , d_1_1_Deprecated(nullptr)
 , d_1_2_Deprecated(nullptr)
 , d_1_3_Deprecated(nullptr)
 , d_1_4_Deprecated(nullptr)
 , m_reserved_2_0_Deprecated(nullptr)
 , d_3_3_Deprecated(nullptr)
{
}

QOpenGLFunctions_4_2_Compatibility::~QOpenGLFunctions_4_2_Compatibility()
{
    if (d_1_0_Core && !d_1_0_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_0_Core->context, QOpenGLFunctions_1_0_CoreBackend::versionStatus());
        delete d_1_0_Core;
    }
    if (d_1_1_Core && !d_1_1_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_1_Core->context, QOpenGLFunctions_1_1_CoreBackend::versionStatus());
        delete d_1_1_Core;
    }
    if (d_1_2_Core && !d_1_2_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_2_Core->context, QOpenGLFunctions_1_2_CoreBackend::versionStatus());
        delete d_1_2_Core;
    }
    if (d_1_3_Core && !d_1_3_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_3_Core->context, QOpenGLFunctions_1_3_CoreBackend::versionStatus());
        delete d_1_3_Core;
    }
    if (d_1_4_Core && !d_1_4_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_4_Core->context, QOpenGLFunctions_1_4_CoreBackend::versionStatus());
        delete d_1_4_Core;
    }
    if (d_1_5_Core && !d_1_5_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_5_Core->context, QOpenGLFunctions_1_5_CoreBackend::versionStatus());
        delete d_1_5_Core;
    }
    if (d_2_0_Core && !d_2_0_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_2_0_Core->context, QOpenGLFunctions_2_0_CoreBackend::versionStatus());
        delete d_2_0_Core;
    }
    if (d_2_1_Core && !d_2_1_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_2_1_Core->context, QOpenGLFunctions_2_1_CoreBackend::versionStatus());
        delete d_2_1_Core;
    }
    if (d_3_0_Core && !d_3_0_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_3_0_Core->context, QOpenGLFunctions_3_0_CoreBackend::versionStatus());
        delete d_3_0_Core;
    }
    if (d_3_1_Core && !d_3_1_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_3_1_Core->context, QOpenGLFunctions_3_1_CoreBackend::versionStatus());
        delete d_3_1_Core;
    }
    if (d_3_2_Core && !d_3_2_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_3_2_Core->context, QOpenGLFunctions_3_2_CoreBackend::versionStatus());
        delete d_3_2_Core;
    }
    if (d_3_3_Core && !d_3_3_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_3_3_Core->context, QOpenGLFunctions_3_3_CoreBackend::versionStatus());
        delete d_3_3_Core;
    }
    if (d_4_0_Core && !d_4_0_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_4_0_Core->context, QOpenGLFunctions_4_0_CoreBackend::versionStatus());
        delete d_4_0_Core;
    }
    if (d_4_1_Core && !d_4_1_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_4_1_Core->context, QOpenGLFunctions_4_1_CoreBackend::versionStatus());
        delete d_4_1_Core;
    }
    if (d_4_2_Core && !d_4_2_Core->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_4_2_Core->context, QOpenGLFunctions_4_2_CoreBackend::versionStatus());
        delete d_4_2_Core;
    }
    if (d_1_0_Deprecated && !d_1_0_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_0_Deprecated->context, QOpenGLFunctions_1_0_DeprecatedBackend::versionStatus());
        delete d_1_0_Deprecated;
    }
    if (d_1_1_Deprecated && !d_1_1_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_1_Deprecated->context, QOpenGLFunctions_1_1_DeprecatedBackend::versionStatus());
        delete d_1_1_Deprecated;
    }
    if (d_1_2_Deprecated && !d_1_2_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_2_Deprecated->context, QOpenGLFunctions_1_2_DeprecatedBackend::versionStatus());
        delete d_1_2_Deprecated;
    }
    if (d_1_3_Deprecated && !d_1_3_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_3_Deprecated->context, QOpenGLFunctions_1_3_DeprecatedBackend::versionStatus());
        delete d_1_3_Deprecated;
    }
    if (d_1_4_Deprecated && !d_1_4_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_1_4_Deprecated->context, QOpenGLFunctions_1_4_DeprecatedBackend::versionStatus());
        delete d_1_4_Deprecated;
    }
    if (d_3_3_Deprecated && !d_3_3_Deprecated->refs.deref()) {
        QAbstractOpenGLFunctionsPrivate::removeFunctionsBackend(d_3_3_Deprecated->context, QOpenGLFunctions_3_3_DeprecatedBackend::versionStatus());
        delete d_3_3_Deprecated;
    }
}

bool QOpenGLFunctions_4_2_Compatibility::initializeOpenGLFunctions()
{
    if ( isInitialized() )
        return true;

    QOpenGLContext* context = QOpenGLContext::currentContext();

    // If owned by a context object make sure it is current.
    // Also check that current context is capable of resolving all needed functions
    if (((owningContext() && owningContext() == context) || !owningContext())
        && QOpenGLFunctions_4_2_Compatibility::isContextCompatible(context))
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

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_2_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_2_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_2_CoreBackend::versionStatus(), d);
        }
        d_1_2_Core = static_cast<QOpenGLFunctions_1_2_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_3_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_3_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_3_CoreBackend::versionStatus(), d);
        }
        d_1_3_Core = static_cast<QOpenGLFunctions_1_3_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_4_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_4_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_4_CoreBackend::versionStatus(), d);
        }
        d_1_4_Core = static_cast<QOpenGLFunctions_1_4_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_5_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_5_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_5_CoreBackend::versionStatus(), d);
        }
        d_1_5_Core = static_cast<QOpenGLFunctions_1_5_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_2_0_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_2_0_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_2_0_CoreBackend::versionStatus(), d);
        }
        d_2_0_Core = static_cast<QOpenGLFunctions_2_0_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_2_1_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_2_1_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_2_1_CoreBackend::versionStatus(), d);
        }
        d_2_1_Core = static_cast<QOpenGLFunctions_2_1_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_3_0_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_3_0_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_3_0_CoreBackend::versionStatus(), d);
        }
        d_3_0_Core = static_cast<QOpenGLFunctions_3_0_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_3_1_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_3_1_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_3_1_CoreBackend::versionStatus(), d);
        }
        d_3_1_Core = static_cast<QOpenGLFunctions_3_1_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_3_2_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_3_2_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_3_2_CoreBackend::versionStatus(), d);
        }
        d_3_2_Core = static_cast<QOpenGLFunctions_3_2_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_3_3_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_3_3_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_3_3_CoreBackend::versionStatus(), d);
        }
        d_3_3_Core = static_cast<QOpenGLFunctions_3_3_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_4_0_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_4_0_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_4_0_CoreBackend::versionStatus(), d);
        }
        d_4_0_Core = static_cast<QOpenGLFunctions_4_0_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_4_1_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_4_1_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_4_1_CoreBackend::versionStatus(), d);
        }
        d_4_1_Core = static_cast<QOpenGLFunctions_4_1_CoreBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_4_2_CoreBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_4_2_CoreBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_4_2_CoreBackend::versionStatus(), d);
        }
        d_4_2_Core = static_cast<QOpenGLFunctions_4_2_CoreBackend*>(d);
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

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_2_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_2_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_2_DeprecatedBackend::versionStatus(), d);
        }
        d_1_2_Deprecated = static_cast<QOpenGLFunctions_1_2_DeprecatedBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_3_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_3_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_3_DeprecatedBackend::versionStatus(), d);
        }
        d_1_3_Deprecated = static_cast<QOpenGLFunctions_1_3_DeprecatedBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_1_4_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_1_4_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_1_4_DeprecatedBackend::versionStatus(), d);
        }
        d_1_4_Deprecated = static_cast<QOpenGLFunctions_1_4_DeprecatedBackend*>(d);
        d->refs.ref();

        d = QAbstractOpenGLFunctionsPrivate::functionsBackend(context, QOpenGLFunctions_3_3_DeprecatedBackend::versionStatus());
        if (!d) {
            d = new QOpenGLFunctions_3_3_DeprecatedBackend(context);
            QAbstractOpenGLFunctionsPrivate::insertFunctionsBackend(context, QOpenGLFunctions_3_3_DeprecatedBackend::versionStatus(), d);
        }
        d_3_3_Deprecated = static_cast<QOpenGLFunctions_3_3_DeprecatedBackend*>(d);
        d->refs.ref();

        QAbstractOpenGLFunctions::initializeOpenGLFunctions();
    }
    return isInitialized();
}

bool QOpenGLFunctions_4_2_Compatibility::isContextCompatible(QOpenGLContext *context)
{
    Q_ASSERT(context);
    QSurfaceFormat f = context->format();
    const QPair<int, int> v = qMakePair(f.majorVersion(), f.minorVersion());
    if (v < qMakePair(4, 2))
        return false;

    if (f.profile() == QSurfaceFormat::CoreProfile)
        return false;

    return true;
}

QOpenGLVersionProfile QOpenGLFunctions_4_2_Compatibility::versionProfile()
{
    QOpenGLVersionProfile v;
    v.setVersion(4, 2);
    v.setProfile(QSurfaceFormat::CompatibilityProfile);
    return v;
}
