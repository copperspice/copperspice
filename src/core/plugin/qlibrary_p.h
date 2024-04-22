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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

#include <qlibrary.h>
#include <qpointer.h>
#include <qstringlist.h>
#include <qplugin.h>
#include <qsharedpointer.h>

#ifdef Q_OS_WIN
# include <qt_windows.h>
#endif

class QLibraryStore;

class QLibraryHandle
{
 public:
   enum UnloadFlag {
      UnloadSys,
      NoUnloadSys
   };

   bool tryload();
   bool loadPlugin();                              // loads and resolves instance
   bool unload(UnloadFlag flag = UnloadSys);

   void release();
   void *resolve(const QString &symbol);

   QLibrary::LoadHints loadHints() const {
      return QLibrary::LoadHints(loadHintsInt.load());
   }

   void setLoadHints(QLibrary::LoadHints lh);

   static QLibraryHandle *findOrLoad(const QString &fileName, const QString &version = QString(),
         QLibrary::LoadHints loadHints = Qt::EmptyFlag);

   static QStringList suffixes_sys(const QString &fullVersion);
   static QStringList prefixes_sys();

   void updatePluginState();
   bool isPlugin();

#ifdef Q_OS_WIN
   HINSTANCE pHnd;
#else
   void *pHnd;
#endif

   QString fileName;
   QString qualifiedFileName;
   QString fullVersion;

   QString errorString;
   QMetaObject *m_metaObject;

   QPointer<QObject> pluginObj;

 private:
   enum PluginState {
      IsAPlugin,
      IsNotAPlugin,
      MightBeAPlugin
   };

   explicit QLibraryHandle(const QString &canonicalFileName, const QString &version, QLibrary::LoadHints loadHints);
   ~QLibraryHandle();

   void mergeLoadHints(QLibrary::LoadHints loadHints);

   bool load_sys();
   bool unload_sys();
   void *resolve_sys(const QString &symbol);

   PluginState pluginState;

   QAtomicInt loadHintsInt;

   // counts how many QLibrary or QPluginLoader are attached to us, plus 1 if it's loaded
   QAtomicInt libraryRefCount;

   // counts how many times load() or loadPlugin() were called
   QAtomicInt libraryUnloadCount;

   friend class QLibraryStore;
};

#endif
