/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <QtCore/qglobal.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qmutex.h>
#include <qmutexpool_p.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

void *qdbus_resolve_me(const char *name);

#if !defined QT_LINKED_LIBDBUS

static QLibrary *qdbus_libdbus = 0;

void qdbus_unloadLibDBus()
{
    delete qdbus_libdbus;
    qdbus_libdbus = 0;
}

bool qdbus_loadLibDBus()
{
    static volatile bool triedToLoadLibrary = false;
    QMutexLocker locker(QMutexPool::globalInstanceGet((void *)&qdbus_resolve_me));

    QLibrary *&lib = qdbus_libdbus;
    if (triedToLoadLibrary)
        return lib && lib->isLoaded();

    lib = new QLibrary;
    triedToLoadLibrary = true;

    static int majorversions[] = { 3, 2, -1 };
    lib->unload();
    lib->setFileName(QLatin1String("dbus-1"));
    lib->setLoadHints(QLibrary::ImprovedSearchHeuristics);
    for (uint i = 0; i < sizeof(majorversions) / sizeof(majorversions[0]); ++i) {
        lib->setFileNameAndVersion(lib->fileName(), majorversions[i]);
        if (lib->load() && lib->resolve("dbus_connection_open_private"))
            return true;

        lib->unload();
    }

    delete lib;
    lib = 0;
    return false;
}

void *qdbus_resolve_conditionally(const char *name)
{
    if (qdbus_loadLibDBus())
        return qdbus_libdbus->resolve(name);
    return 0;
}

void *qdbus_resolve_me(const char *name)
{
    void *ptr = 0;
    if (!qdbus_loadLibDBus())
        qFatal("Cannot find libdbus-1 in your system to resolve symbol '%s'.", name);

    ptr = qdbus_libdbus->resolve(name);
    if (!ptr)
        qFatal("Cannot resolve '%s' in your libdbus-1.", name);

    return ptr;
}

Q_DESTRUCTOR_FUNCTION(qdbus_unloadLibDBus)

#endif // QT_LINKED_LIBDBUS

QT_END_NAMESPACE

#endif // QT_NO_DBUS
