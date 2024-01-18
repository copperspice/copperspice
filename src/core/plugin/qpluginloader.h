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

#ifndef QPLUGINLOADER_H
#define QPLUGINLOADER_H

#include <qlibrary.h>
#include <qplugin.h>

class QLibraryHandle;

class Q_CORE_EXPORT QPluginLoader : public QObject
{
   CORE_CS_OBJECT(QPluginLoader)

   CORE_CS_PROPERTY_READ(fileName, fileName)
   CORE_CS_PROPERTY_WRITE(fileName, setFileName)

   CORE_CS_PROPERTY_READ(loadHints, loadHints)
   CORE_CS_PROPERTY_WRITE(loadHints, setLoadHints)

 public:
   explicit QPluginLoader(QObject *parent = nullptr);
   explicit QPluginLoader(const QString &fileName, QObject *parent = nullptr);

   QPluginLoader(const QPluginLoader &) = delete;
   QPluginLoader &operator=(const QPluginLoader &) = delete;

   ~QPluginLoader();

   QObject *instance();

   static QObjectList staticInstances();
   static QVector<QMetaObject *> staticPlugins();

   bool load();
   bool unload();
   bool isLoaded() const;

   void setFileName(const QString &fileName);
   QString fileName() const;

   QString errorString() const;

   void setLoadHints(QLibrary::LoadHints loadHints);
   QLibrary::LoadHints loadHints() const;

 private:
   QLibraryHandle *mp_handle;
   bool mp_loaded;
};

#endif
