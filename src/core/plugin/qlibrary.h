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

#ifndef QLIBRARY_H
#define QLIBRARY_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QLibraryPrivate;

class Q_CORE_EXPORT QLibrary : public QObject
{
   CORE_CS_OBJECT(QLibrary)

   CORE_CS_PROPERTY_READ(fileName, fileName)
   CORE_CS_PROPERTY_WRITE(fileName, setFileName)

   CORE_CS_PROPERTY_READ(loadHints, loadHints)
   CORE_CS_PROPERTY_WRITE(loadHints, setLoadHints)

 public:
   enum LoadHint {
      ResolveAllSymbolsHint     = 0x01,
      ExportExternalSymbolsHint = 0x02,
      LoadArchiveMemberHint     = 0x04,
      ImprovedSearchHeuristics  = 0x08
   };
   using LoadHints = QFlags<LoadHint>;

   CORE_CS_FLAG(LoadHint, LoadHints)

   explicit QLibrary(QObject *parent = nullptr);
   explicit QLibrary(const QString &fileName, QObject *parent = nullptr);
   explicit QLibrary(const QString &fileName, int verNum, QObject *parent = nullptr);
   explicit QLibrary(const QString &fileName, const QString &version, QObject *parent = nullptr);
   ~QLibrary();

   void *resolve(const QString &symbol);
   static void *resolve(const QString &fileName, const QString &symbol);
   static void *resolve(const QString &fileName, int verNum, const QString &symbol);
   static void *resolve(const QString &fileName, const QString &version, const QString &symbol);

   bool load();
   bool unload();
   bool isLoaded() const;

   static bool isLibrary(const QString &fileName);

   void setFileName(const QString &fileName);
   QString fileName() const;

   void setFileNameAndVersion(const QString &fileName, int verNum);
   void setFileNameAndVersion(const QString &fileName, const QString &version);
   QString errorString() const;

   void setLoadHints(LoadHints hints);
   LoadHints loadHints() const;

 private:
   QLibraryPrivate *d;
   bool did_load;
   Q_DISABLE_COPY(QLibrary)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLibrary::LoadHints)

QT_END_NAMESPACE

#endif //QLIBRARY_H
