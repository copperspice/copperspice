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

#ifndef QDECLARATIVEENGINE_H
#define QDECLARATIVEENGINE_H

#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtScript/qscriptvalue.h>
#include <QtDeclarative/qdeclarativeerror.h>
#include <QtDeclarative/qdeclarativedebug.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeComponent;
class QDeclarativeEnginePrivate;
class QDeclarativeImportsPrivate;
class QDeclarativeExpression;
class QDeclarativeContext;
class QDeclarativeType;
class QUrl;
class QScriptEngine;
class QScriptContext;
class QDeclarativeImageProvider;
class QNetworkAccessManager;
class QDeclarativeNetworkAccessManagerFactory;

class Q_DECLARATIVE_EXPORT QDeclarativeEngine : public QObject
{
   DECL_CS_OBJECT(QDeclarativeEngine)

   DECL_CS_PROPERTY_READ(offlineStoragePath, offlineStoragePath)
   DECL_CS_PROPERTY_WRITE(offlineStoragePath, setOfflineStoragePath)

 public:
   QDeclarativeEngine(QObject *p = 0);
   virtual ~QDeclarativeEngine();

   QDeclarativeContext *rootContext() const;

   void clearComponentCache();

   QStringList importPathList() const;
   void setImportPathList(const QStringList &paths);
   void addImportPath(const QString &dir);

   QStringList pluginPathList() const;
   void setPluginPathList(const QStringList &paths);
   void addPluginPath(const QString &dir);

   bool importPlugin(const QString &filePath, const QString &uri, QString *errorString);

   void setNetworkAccessManagerFactory(QDeclarativeNetworkAccessManagerFactory *);
   QDeclarativeNetworkAccessManagerFactory *networkAccessManagerFactory() const;

   QNetworkAccessManager *networkAccessManager() const;

   void addImageProvider(const QString &id, QDeclarativeImageProvider *);
   QDeclarativeImageProvider *imageProvider(const QString &id) const;
   void removeImageProvider(const QString &id);

   void setOfflineStoragePath(const QString &dir);
   QString offlineStoragePath() const;

   QUrl baseUrl() const;
   void setBaseUrl(const QUrl &);

   bool outputWarningsToStandardError() const;
   void setOutputWarningsToStandardError(bool);

   static QDeclarativeContext *contextForObject(const QObject *);
   static void setContextForObject(QObject *, QDeclarativeContext *);

   enum ObjectOwnership { CppOwnership, JavaScriptOwnership };
   static void setObjectOwnership(QObject *, ObjectOwnership);
   static ObjectOwnership objectOwnership(QObject *);

 public:
   DECL_CS_SIGNAL_1(Public, void quit())
   DECL_CS_SIGNAL_2(quit)
   DECL_CS_SIGNAL_1(Public, void warnings(const QList <QDeclarativeError> &warnings))
   DECL_CS_SIGNAL_2(warnings, warnings)

 private:
   Q_DISABLE_COPY(QDeclarativeEngine)
   Q_DECLARE_PRIVATE(QDeclarativeEngine)

 protected:
   QScopedPointer<QDeclarativeEnginePrivate> d_ptr;

};

QT_END_NAMESPACE

#endif // QDECLARATIVEENGINE_H
