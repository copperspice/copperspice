/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

#ifdef Q_OS_WIN
# include <qt_windows.h>
#endif

#include <qlibrary.h>
#include <qpointer.h>
#include <qstringlist.h>
#include <qplugin.h>
#include <qsharedpointer.h>

QT_BEGIN_NAMESPACE

bool qt_debug_component();

class QSettings;

class QLibraryPrivate
{
 public:

#ifdef Q_OS_WIN
   HINSTANCE
#else
   void *
#endif

   pHnd;

   QString fileName;
   QString qualifiedFileName;
   QString fullVersion;

   bool load();
   bool loadPlugin(); // loads and resolves instance
   bool unload();
   void release();
   void *resolve(const QString &symbol);

   static QLibraryPrivate *findOrCreate(const QString &fileName, const QString &version = QString());

   QWeakPointer<QObject> inst;
   QtPluginInstanceFunction instance;
   uint cs_version;
   QString lastModified;

   QString errorString;
   QLibrary::LoadHints loadHints;

   bool isPlugin(QSettings *settings = 0);

 private:
   explicit QLibraryPrivate(const QString &canonicalFileName, const QString &version);
   ~QLibraryPrivate();

   bool load_sys();
   bool unload_sys();
   void *resolve_sys(const QString &symbol);

   QAtomicInt libraryRefCount;
   QAtomicInt libraryUnloadCount;

   enum {IsAPlugin, IsNotAPlugin, MightBeAPlugin } pluginState;
   friend class QLibraryPrivateHasFriends;
};

QT_END_NAMESPACE

#endif // QLIBRARY_P_H
