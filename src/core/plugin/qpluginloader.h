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

#ifndef QPLUGINLOADER_H
#define QPLUGINLOADER_H

#include <QtCore/qlibrary.h>

QT_BEGIN_NAMESPACE

class QLibraryPrivate;

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
   ~QPluginLoader();

   QObject *instance();

   static QObjectList staticInstances();

   bool load();
   bool unload();
   bool isLoaded() const;

   void setFileName(const QString &fileName);
   QString fileName() const;

   QString errorString() const;

   void setLoadHints(QLibrary::LoadHints loadHints);
   QLibrary::LoadHints loadHints() const;

 private:
   QLibraryPrivate *d;
   bool did_load;
   Q_DISABLE_COPY(QPluginLoader)
};

QT_END_NAMESPACE

#endif //QPLUGINLOADER_H
