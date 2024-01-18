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

#ifndef QDECLARATIVETYPELOADER_P_H
#define QDECLARATIVETYPELOADER_P_H

#include <QtCore/qobject.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtDeclarative/qdeclarativeerror.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <qdeclarativescriptparser_p.h>
#include <qdeclarativedirparser_p.h>
#include <qdeclarativeimport_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeScriptData;
class QDeclarativeQmldirData;
class QDeclarativeTypeLoader;
class QDeclarativeCompiledData;
class QDeclarativeComponentPrivate;
class QDeclarativeTypeData;
class QDeclarativeDataLoader;

class QDeclarativeDataBlob : public QDeclarativeRefCount
{
 public:
   enum Status {
      Null,                    // Prior to QDeclarativeDataLoader::load()
      Loading,                 // Prior to data being received and dataReceived() being called
      WaitingForDependencies,  // While there are outstanding addDependency()s
      Complete,                // Finished
      Error                    // Error
   };

   enum Type {
      QmlFile,
      JavaScriptFile,
      QmldirFile
   };

   QDeclarativeDataBlob(const QUrl &, Type);
   virtual ~QDeclarativeDataBlob();

   Type type() const;

   Status status() const;
   bool isNull() const;
   bool isLoading() const;
   bool isWaiting() const;
   bool isComplete() const;
   bool isError() const;
   bool isCompleteOrError() const;

   qreal progress() const;

   QUrl url() const;
   QUrl finalUrl() const;

   QList<QDeclarativeError> errors() const;

   void setError(const QDeclarativeError &);
   void setError(const QList<QDeclarativeError> &errors);

   void addDependency(QDeclarativeDataBlob *);

 protected:
   virtual void dataReceived(const QByteArray &) = 0;

   virtual void done();
   virtual void networkError(QNetworkReply::NetworkError);

   virtual void dependencyError(QDeclarativeDataBlob *);
   virtual void dependencyComplete(QDeclarativeDataBlob *);
   virtual void allDependenciesDone();

   virtual void downloadProgressChanged(qreal);

 private:
   friend class QDeclarativeDataLoader;
   void tryDone();
   void cancelAllWaitingFor();
   void notifyAllWaitingOnMe();
   void notifyComplete(QDeclarativeDataBlob *);

   Type m_type;
   Status m_status;
   qreal m_progress;

   QUrl m_url;
   QUrl m_finalUrl;

   // List of QDeclarativeDataBlob's that are waiting for me to complete.
   QList<QDeclarativeDataBlob *> m_waitingOnMe;

   // List of QDeclarativeDataBlob's that I am waiting for to complete.
   QList<QDeclarativeDataBlob *> m_waitingFor;

   // Manager that is currently fetching data for me
   QDeclarativeDataLoader *m_manager;
   int m_redirectCount: 30;
   bool m_inCallback: 1;
   bool m_isDone: 1;

   QList<QDeclarativeError> m_errors;
};

class QDeclarativeDataLoader : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDataLoader)

 public:
   QDeclarativeDataLoader(QDeclarativeEngine *);
   ~QDeclarativeDataLoader();

   void load(QDeclarativeDataBlob *);
   void loadWithStaticData(QDeclarativeDataBlob *, const QByteArray &);

   QDeclarativeEngine *engine() const;

 private :
   DECL_CS_SLOT_1(Private, void networkReplyFinished())
   DECL_CS_SLOT_OVERLOAD(networkReplyFinished, ())

   DECL_CS_SLOT_1(Private, void networkReplyProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   DECL_CS_SLOT_OVERLOAD(networkReplyProgress, (qint64, qint64))

   void setData(QDeclarativeDataBlob *, const QByteArray &);
   void networkReplyFinished(QNetworkReply *);
   void networkReplyProgress(QNetworkReply *, qint64, qint64);

   QDeclarativeEngine *m_engine;
   typedef QHash<QNetworkReply *, QDeclarativeDataBlob *> NetworkReplies;
   NetworkReplies m_networkReplies;
};


class QDeclarativeTypeLoader : public QDeclarativeDataLoader
{
   DECL_CS_OBJECT(QDeclarativeTypeLoader)

 public:
   QDeclarativeTypeLoader(QDeclarativeEngine *);
   ~QDeclarativeTypeLoader();

   enum Option {
      None,
      PreserveParser
   };
   using Options = QFlags<Option>;

   QDeclarativeTypeData *get(const QUrl &url);
   QDeclarativeTypeData *get(const QByteArray &, const QUrl &url, Options = None);
   void clearCache();

   QDeclarativeScriptData *getScript(const QUrl &);
   QDeclarativeQmldirData *getQmldir(const QUrl &);

   QString absoluteFilePath(const QString &path);
   const QDeclarativeDirParser *qmlDirParser(const QString &absoluteFilePath);

 private:
   typedef QHash<QUrl, QDeclarativeTypeData *> TypeCache;
   typedef QHash<QUrl, QDeclarativeScriptData *> ScriptCache;
   typedef QHash<QUrl, QDeclarativeQmldirData *> QmldirCache;
   typedef QSet<QString> StringSet;
   typedef QHash<QString, StringSet *> ImportDirCache;
   typedef QHash<QString, QDeclarativeDirParser *> ImportQmlDirCache;

   TypeCache m_typeCache;
   ScriptCache m_scriptCache;
   QmldirCache m_qmldirCache;
   ImportDirCache m_importDirCache;
   ImportQmlDirCache m_importQmlDirCache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeTypeLoader::Options)

class QDeclarativeTypeData : public QDeclarativeDataBlob
{
 public:
   struct TypeReference {
      TypeReference() : type(0), majorVersion(0), minorVersion(0), typeData(0) {}

      QDeclarativeParser::Location location;
      QDeclarativeType *type;
      int majorVersion;
      int minorVersion;
      QDeclarativeTypeData *typeData;
   };

   struct ScriptReference {
      ScriptReference() : script(0) {}

      QDeclarativeParser::Location location;
      QString qualifier;
      QDeclarativeScriptData *script;
   };

   QDeclarativeTypeData(const QUrl &, QDeclarativeTypeLoader::Options, QDeclarativeTypeLoader *);
   ~QDeclarativeTypeData();

   QDeclarativeTypeLoader *typeLoader() const;

   const QDeclarativeImports &imports() const;
   const QDeclarativeScriptParser &parser() const;

   const QList<TypeReference> &resolvedTypes() const;
   const QList<ScriptReference> &resolvedScripts() const;

   QDeclarativeCompiledData *compiledData() const;

   // Used by QDeclarativeComponent to get notifications
   struct TypeDataCallback {
      ~TypeDataCallback() {}
      virtual void typeDataProgress(QDeclarativeTypeData *, qreal) {}
      virtual void typeDataReady(QDeclarativeTypeData *) {}
   };
   void registerCallback(TypeDataCallback *);
   void unregisterCallback(TypeDataCallback *);

 protected:
   virtual void done();
   virtual void dataReceived(const QByteArray &);
   virtual void allDependenciesDone();
   virtual void downloadProgressChanged(qreal);

 private:
   void resolveTypes();
   void compile();

   QDeclarativeTypeLoader::Options m_options;

   QDeclarativeQmldirData *qmldirForUrl(const QUrl &);

   QDeclarativeScriptParser scriptParser;
   QDeclarativeImports m_imports;

   QList<ScriptReference> m_scripts;
   QList<QDeclarativeQmldirData *> m_qmldirs;

   QList<TypeReference> m_types;
   bool m_typesResolved: 1;

   QDeclarativeCompiledData *m_compiledData;

   QList<TypeDataCallback *> m_callbacks;

   QDeclarativeTypeLoader *m_typeLoader;
};

class QDeclarativeScriptData : public QDeclarativeDataBlob
{
 public:
   QDeclarativeScriptData(const QUrl &);

   QDeclarativeParser::Object::ScriptBlock::Pragmas pragmas() const;
   QString scriptSource() const;

 protected:
   virtual void dataReceived(const QByteArray &);

 private:
   QDeclarativeParser::Object::ScriptBlock::Pragmas m_pragmas;
   QString m_source;
};

class QDeclarativeQmldirData : public QDeclarativeDataBlob
{
 public:
   QDeclarativeQmldirData(const QUrl &);

   const QDeclarativeDirComponents &dirComponents() const;

 protected:
   virtual void dataReceived(const QByteArray &);

 private:
   QDeclarativeDirComponents m_components;

};

QT_END_NAMESPACE

#endif // QDECLARATIVETYPELOADER_P_H
