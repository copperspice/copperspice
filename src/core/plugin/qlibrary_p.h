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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

#include <qlibrary.h>
#include <qpointer.h>
#include <qstringlist.h>
#include <qplugin.h>
#include <qsharedpointer.h>
#include "QtCore/qjsonobject.h"
#include "QtCore/qjsondocument.h"
#include "QtCore/qendian.h"

#ifdef Q_OS_WIN
# include <qt_windows.h>
#endif

class QLibraryStore;

bool qt_debug_component();

class QLibraryPrivate
{
   enum {IsAPlugin, IsNotAPlugin, MightBeAPlugin } pluginState;

 public:

#ifdef Q_OS_WIN
   HINSTANCE
#else
   void *
#endif

   pHnd;

   enum UnloadFlag { UnloadSys, NoUnloadSys };

   QString fileName;
   QString qualifiedFileName;
   QString fullVersion;

   bool load();
   bool loadPlugin(); // loads and resolves instance
   bool unload(UnloadFlag flag = UnloadSys);
   void release();
   void *resolve(const QString &symbol);

   QLibrary::LoadHints loadHints() const {
      return QLibrary::LoadHints(loadHintsInt.load());
   }

   void setLoadHints(QLibrary::LoadHints lh);
   static QLibraryPrivate *findOrCreate(const QString &fileName,
      const QString &version = QString(), QLibrary::LoadHints loadHints = 0);

   static QStringList suffixes_sys(const QString &fullVersion);
   static QStringList prefixes_sys();

   QPointer<QObject> inst;
   QtPluginInstanceFunction instance;

   QJsonObject metaData;

   QString errorString;

   void updatePluginState();
   bool isPlugin();

   static inline QJsonDocument fromRawMetaData(const char *raw) {
      raw += strlen("QTMETADATA  ");

      // the size of the embedded JSON object can be found 8 bytes into the data (see qjson_p.h),
      // but does not include the size of the header (8 bytes)


      // BROOM - bad mojo !!        QByteArray json(raw, qFromLittleEndian<uint>(*(const uint *)(raw + 8)) + 8);
      QByteArray json;  // broom - must fix


      return QJsonDocument::fromJson(json);
   }

 private:
   explicit QLibraryPrivate(const QString &canonicalFileName, const QString &version, QLibrary::LoadHints loadHints);
   ~QLibraryPrivate();

   void mergeLoadHints(QLibrary::LoadHints loadHints);

   bool load_sys();
   bool unload_sys();
   void *resolve_sys(const QString &symbol);

   QAtomicInt loadHintsInt;

   // counts how many QLibrary or QPluginLoader are attached to us, plus 1 if it's loaded
   QAtomicInt libraryRefCount;

   // counts how many times load() or loadPlugin() were called
   QAtomicInt libraryUnloadCount;

   friend class QLibraryStore;
};



#endif
