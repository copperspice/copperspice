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

#ifndef QDECLARATIVEIMPORT_P_H
#define QDECLARATIVEIMPORT_P_H

#include <QtCore/qurl.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qset.h>
#include <qdeclarativedirparser_p.h>
#include <qdeclarativescriptparser_p.h>
#include <qdeclarativemetatype_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTypeNameCache;
class QDeclarativeEngine;
class QDir;
class QDeclarativeImportedNamespace;
class QDeclarativeImportsPrivate;
class QDeclarativeImportDatabase;
class QDeclarativeTypeLoader;

class QDeclarativeImports
{
 public:
   QDeclarativeImports();
   QDeclarativeImports(QDeclarativeTypeLoader *);
   QDeclarativeImports(const QDeclarativeImports &);
   ~QDeclarativeImports();
   QDeclarativeImports &operator=(const QDeclarativeImports &);

   void setBaseUrl(const QUrl &url);
   QUrl baseUrl() const;

   bool resolveType(const QByteArray &type,
                    QDeclarativeType **type_return, QUrl *url_return,
                    int *version_major, int *version_minor,
                    QDeclarativeImportedNamespace **ns_return,
                    QString *errorString = 0) const;
   bool resolveType(QDeclarativeImportedNamespace *,
                    const QByteArray &type,
                    QDeclarativeType **type_return, QUrl *url_return,
                    int *version_major, int *version_minor) const;

   bool addImport(QDeclarativeImportDatabase *,
                  const QString &uri, const QString &prefix, int vmaj, int vmin,
                  QDeclarativeScriptParser::Import::Type importType,
                  const QDeclarativeDirComponents &qmldircomponentsnetwork,
                  QString *errorString);

   void populateCache(QDeclarativeTypeNameCache *cache, QDeclarativeEngine *) const;

 private:
   friend class QDeclarativeImportDatabase;
   QDeclarativeImportsPrivate *d;
};

class QDeclarativeImportDatabase
{
   Q_DECLARE_TR_FUNCTIONS(QDeclarativeImportDatabase)
 public:
   QDeclarativeImportDatabase(QDeclarativeEngine *);
   ~QDeclarativeImportDatabase();

   bool importPlugin(const QString &filePath, const QString &uri, QString *errorString);

   QStringList importPathList() const;
   void setImportPathList(const QStringList &paths);
   void addImportPath(const QString &dir);

   QStringList pluginPathList() const;
   void setPluginPathList(const QStringList &paths);
   void addPluginPath(const QString &path);

 private:
   friend class QDeclarativeImportsPrivate;
   QString resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath,
                         const QString &baseName, const QStringList &suffixes,
                         const QString &prefix = QString());
   QString resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath,
                         const QString &baseName);


   QStringList filePluginPath;
   QStringList fileImportPath;

   QSet<QString> initializedPlugins;
   QDeclarativeEngine *engine;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEIMPORT_P_H

