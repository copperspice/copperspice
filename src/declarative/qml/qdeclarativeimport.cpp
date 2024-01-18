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

#include <algorithm>
#include <qdeclarativeimport_p.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qlibraryinfo.h>
#include <QtDeclarative/qdeclarativeextensioninterface.h>
#include <private/qdeclarativeglobal_p.h>
#include <private/qdeclarativetypenamecache_p.h>
#include <private/qdeclarativeengine_p.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(qmlImportTrace, QML_IMPORT_TRACE)
DEFINE_BOOL_CONFIG_OPTION(qmlCheckTypes, QML_CHECK_TYPES)

static bool greaterThan(const QString &s1, const QString &s2)
{
   return s1 > s2;
}

typedef QMap<QString, QString> StringStringMap;
Q_GLOBAL_STATIC(StringStringMap, qmlEnginePluginsWithRegisteredTypes); // stores the uri

class QDeclarativeImportedNamespace
{
 public:
   QStringList uris;
   QStringList urls;
   QList<int> majversions;
   QList<int> minversions;
   QList<bool> isLibrary;
   QList<QDeclarativeDirComponents> qmlDirComponents;


   bool find_helper(QDeclarativeTypeLoader *typeLoader, int i, const QByteArray &type, int *vmajor, int *vminor,
                    QDeclarativeType **type_return, QUrl *url_return,
                    QUrl *base = 0, bool *typeRecursionDetected = 0);
   bool find(QDeclarativeTypeLoader *typeLoader, const QByteArray &type, int *vmajor, int *vminor,
             QDeclarativeType **type_return,
             QUrl *url_return, QUrl *base = 0, QString *errorString = 0);
};

class QDeclarativeImportsPrivate
{
 public:
   QDeclarativeImportsPrivate(QDeclarativeTypeLoader *loader);
   ~QDeclarativeImportsPrivate();

   bool importExtension(const QString &absoluteFilePath, const QString &uri,
                        QDeclarativeImportDatabase *database, QDeclarativeDirComponents *components,
                        QString *errorString);

   QString resolvedUri(const QString &dir_arg, QDeclarativeImportDatabase *database);
   bool add(const QDeclarativeDirComponents &qmldircomponentsnetwork,
            const QString &uri_arg, const QString &prefix,
            int vmaj, int vmin, QDeclarativeScriptParser::Import::Type importType,
            QDeclarativeImportDatabase *database, QString *errorString);
   bool find(const QByteArray &type, int *vmajor, int *vminor,
             QDeclarativeType **type_return, QUrl *url_return, QString *errorString);

   QDeclarativeImportedNamespace *findNamespace(const QString &type);

   QUrl base;
   int ref;

   QSet<QString> qmlDirFilesForWhichPluginsHaveBeenLoaded;
   QDeclarativeImportedNamespace unqualifiedset;
   QHash<QString, QDeclarativeImportedNamespace * > set;
   QDeclarativeTypeLoader *typeLoader;
};

/*!
\class QDeclarativeImports
\brief The QDeclarativeImports class encapsulates one QML document's import statements.
\internal
*/
QDeclarativeImports::QDeclarativeImports(const QDeclarativeImports &copy)
   : d(copy.d)
{
   ++d->ref;
}

QDeclarativeImports &
QDeclarativeImports::operator =(const QDeclarativeImports &copy)
{
   ++copy.d->ref;
   if (--d->ref == 0) {
      delete d;
   }
   d = copy.d;
   return *this;
}

QDeclarativeImports::QDeclarativeImports()
   : d(new QDeclarativeImportsPrivate(0))
{
}

QDeclarativeImports::QDeclarativeImports(QDeclarativeTypeLoader *typeLoader)
   : d(new QDeclarativeImportsPrivate(typeLoader))
{
}

QDeclarativeImports::~QDeclarativeImports()
{
   if (--d->ref == 0) {
      delete d;
   }
}

/*!
  Sets the base URL to be used for all relative file imports added.
*/
void QDeclarativeImports::setBaseUrl(const QUrl &url)
{
   d->base = url;
}

/*!
  Returns the base URL to be used for all relative file imports added.
*/
QUrl QDeclarativeImports::baseUrl() const
{
   return d->base;
}

static QDeclarativeTypeNameCache *
cacheForNamespace(QDeclarativeEngine *engine, const QDeclarativeImportedNamespace &set,
                  QDeclarativeTypeNameCache *cache)
{
   if (!cache) {
      cache = new QDeclarativeTypeNameCache(engine);
   }

   QList<QDeclarativeType *> types = QDeclarativeMetaType::qmlTypes();

   for (int ii = 0; ii < set.uris.count(); ++ii) {
      QByteArray base = set.uris.at(ii).toUtf8() + '/';
      int major = set.majversions.at(ii);
      int minor = set.minversions.at(ii);

      foreach (QDeclarativeType * type, types) {
         if (type->qmlTypeName().startsWith(base) &&
               type->qmlTypeName().lastIndexOf('/') == (base.length() - 1) &&
               type->availableInVersion(major, minor)) {
            QString name = QString::fromUtf8(type->qmlTypeName().mid(base.length()));

            cache->add(name, type);
         }
      }
   }

   return cache;
}

void QDeclarativeImports::populateCache(QDeclarativeTypeNameCache *cache, QDeclarativeEngine *engine) const
{
   const QDeclarativeImportedNamespace &set = d->unqualifiedset;

   for (QHash<QString, QDeclarativeImportedNamespace * >::ConstIterator iter = d->set.begin();
         iter != d->set.end(); ++iter) {

      QDeclarativeTypeNameCache::Data *d = cache->data(iter.key());
      if (d) {
         if (!d->typeNamespace) {
            cacheForNamespace(engine, *(*iter), d->typeNamespace);
         }
      } else {
         QDeclarativeTypeNameCache *nc = cacheForNamespace(engine, *(*iter), 0);
         cache->add(iter.key(), nc);
         nc->release();
      }
   }

   cacheForNamespace(engine, set, cache);
}

/*!
  \internal

  The given (namespace qualified) \a type is resolved to either
  \list
  \o a QDeclarativeImportedNamespace stored at \a ns_return,
  \o a QDeclarativeType stored at \a type_return, or
  \o a component located at \a url_return.
  \endlist

  If any return pointer is 0, the corresponding search is not done.

  \sa addImport()
*/
bool QDeclarativeImports::resolveType(const QByteArray &type,
                                      QDeclarativeType **type_return, QUrl *url_return, int *vmaj, int *vmin,
                                      QDeclarativeImportedNamespace **ns_return, QString *errorString) const
{
   QDeclarativeImportedNamespace *ns = d->findNamespace(QString::fromUtf8(type));
   if (ns) {
      if (ns_return) {
         *ns_return = ns;
      }
      return true;
   }
   if (type_return || url_return) {
      if (d->find(type, vmaj, vmin, type_return, url_return, errorString)) {
         if (qmlImportTrace()) {
            if (type_return && *type_return && url_return && !url_return->isEmpty())
               qDebug().nospace() << "QDeclarativeImports(" << qPrintable(baseUrl().toString()) << ")" << "::resolveType: "
                                  << type << " => " << (*type_return)->typeName() << " " << *url_return;
            if (type_return && *type_return)
               qDebug().nospace() << "QDeclarativeImports(" << qPrintable(baseUrl().toString()) << ")" << "::resolveType: "
                                  << type << " => " << (*type_return)->typeName();
            if (url_return && !url_return->isEmpty())
               qDebug().nospace() << "QDeclarativeImports(" << qPrintable(baseUrl().toString()) << ")" << "::resolveType: "
                                  << type << " => " << *url_return;
         }
         return true;
      }
   }
   return false;
}

/*!
  \internal

  Searching \e only in the namespace \a ns (previously returned in a call to
  resolveType(), \a type is found and returned to either
  a QDeclarativeType stored at \a type_return, or
  a component located at \a url_return.

  If either return pointer is 0, the corresponding search is not done.
*/
bool QDeclarativeImports::resolveType(QDeclarativeImportedNamespace *ns, const QByteArray &type,
                                      QDeclarativeType **type_return, QUrl *url_return,
                                      int *vmaj, int *vmin) const
{
   Q_ASSERT(d->typeLoader);
   return ns->find(d->typeLoader, type, vmaj, vmin, type_return, url_return);
}

bool QDeclarativeImportedNamespace::find_helper(QDeclarativeTypeLoader *typeLoader, int i, const QByteArray &type,
      int *vmajor, int *vminor,
      QDeclarativeType **type_return, QUrl *url_return,
      QUrl *base, bool *typeRecursionDetected)
{
   int vmaj = majversions.at(i);
   int vmin = minversions.at(i);

   QByteArray qt = uris.at(i).toUtf8();
   qt += '/';
   qt += type;

   QDeclarativeType *t = QDeclarativeMetaType::qmlType(qt, vmaj, vmin);
   if (t) {
      if (vmajor) {
         *vmajor = vmaj;
      }
      if (vminor) {
         *vminor = vmin;
      }
      if (type_return) {
         *type_return = t;
      }
      return true;
   }

   QDeclarativeDirComponents qmldircomponents = qmlDirComponents.at(i);

   bool typeWasDeclaredInQmldir = false;
   if (!qmldircomponents.isEmpty()) {
      const QString typeName = QString::fromUtf8(type);
      foreach (const QDeclarativeDirParser::Component & c, qmldircomponents) {
         if (c.typeName == typeName) {
            typeWasDeclaredInQmldir = true;

            // importing version -1 means import ALL versions
            if ((vmaj == -1) || (c.majorVersion < vmaj || (c.majorVersion == vmaj && vmin >= c.minorVersion))) {
               QUrl url = QUrl(urls.at(i) + QLatin1Char('/') + QString::fromUtf8(type) + QLatin1String(".qml"));
               QUrl candidate = url.resolved(QUrl(c.fileName));
               if (c.internal && base) {
                  if (base->resolved(QUrl(c.fileName)) != candidate) {
                     continue;   // failed attempt to access an internal type
                  }
               }
               if (base && *base == candidate) {
                  if (typeRecursionDetected) {
                     *typeRecursionDetected = true;
                  }
                  continue; // no recursion
               }
               if (url_return) {
                  *url_return = candidate;
               }
               return true;
            }
         }
      }
   }

   if (!typeWasDeclaredInQmldir  && !isLibrary.at(i)) {
      // XXX search non-files too! (eg. zip files, see QT-524)
      QUrl url = QUrl(urls.at(i) + QLatin1Char('/') + QString::fromUtf8(type) + QLatin1String(".qml"));
      QString file = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(url);
      if (!typeLoader->absoluteFilePath(file).isEmpty()) {
         if (base && *base == url) { // no recursion
            if (typeRecursionDetected) {
               *typeRecursionDetected = true;
            }
         } else {
            if (url_return) {
               *url_return = url;
            }
            return true;
         }
      }
   }
   return false;
}

QDeclarativeImportsPrivate::QDeclarativeImportsPrivate(QDeclarativeTypeLoader *loader)
   : ref(1), typeLoader(loader)
{
}

QDeclarativeImportsPrivate::~QDeclarativeImportsPrivate()
{
   foreach (QDeclarativeImportedNamespace * s, set.values())
   delete s;
}

bool QDeclarativeImportsPrivate::importExtension(const QString &absoluteFilePath, const QString &uri,
      QDeclarativeImportDatabase *database,
      QDeclarativeDirComponents *components, QString *errorString)
{
   Q_ASSERT(typeLoader);
   const QDeclarativeDirParser *qmldirParser = typeLoader->qmlDirParser(absoluteFilePath);
   if (qmldirParser->hasError()) {
      if (errorString) {
         const QList<QDeclarativeError> qmldirErrors = qmldirParser->errors(uri);
         for (int i = 0; i < qmldirErrors.size(); ++i) {
            *errorString += qmldirErrors.at(i).description();
         }
      }
      return false;
   }

   if (! qmlDirFilesForWhichPluginsHaveBeenLoaded.contains(absoluteFilePath)) {
      qmlDirFilesForWhichPluginsHaveBeenLoaded.insert(absoluteFilePath);

      QDir dir = QFileInfo(absoluteFilePath).dir();
      foreach (const QDeclarativeDirParser::Plugin & plugin, qmldirParser->plugins()) {

         QString resolvedFilePath = database->resolvePlugin(dir, plugin.path, plugin.name);

         if (!resolvedFilePath.isEmpty()) {
            if (!database->importPlugin(resolvedFilePath, uri, errorString)) {
               if (errorString) {
                  *errorString = QDeclarativeImportDatabase::tr("plugin cannot be loaded for module \"%1\": %2").arg(uri).arg(
                                    *errorString);
               }
               return false;
            }
         } else {
            if (errorString) {
               *errorString = QDeclarativeImportDatabase::tr("module \"%1\" plugin \"%2\" not found").arg(uri).arg(plugin.name);
            }
            return false;
         }
      }
   }

   if (components) {
      *components = qmldirParser->components();
   }

   return true;
}

QString QDeclarativeImportsPrivate::resolvedUri(const QString &dir_arg, QDeclarativeImportDatabase *database)
{
   QString dir = dir_arg;
   if (dir.endsWith(QLatin1Char('/')) || dir.endsWith(QLatin1Char('\\'))) {
      dir.chop(1);
   }

   QStringList paths = database->fileImportPath;
   std::sort(paths.begin(), paths.end(), greaterThan); // Ensure subdirs preceed their parents.

   QString stableRelativePath = dir;
   foreach(const QString & path, paths) {
      if (dir.startsWith(path)) {
         stableRelativePath = dir.mid(path.length() + 1);
         break;
      }
   }

   stableRelativePath.replace(QLatin1Char('\\'), QLatin1Char('/'));

   // remove optional versioning in dot notation from uri
   int lastSlash = stableRelativePath.lastIndexOf(QLatin1Char('/'));
   if (lastSlash >= 0) {
      int versionDot = stableRelativePath.indexOf(QLatin1Char('.'), lastSlash);
      if (versionDot >= 0) {
         stableRelativePath = stableRelativePath.left(versionDot);
      }
   }

   stableRelativePath.replace(QLatin1Char('/'), QLatin1Char('.'));
   return stableRelativePath;
}

bool QDeclarativeImportsPrivate::add(const QDeclarativeDirComponents &qmldircomponentsnetwork,
                                     const QString &uri_arg, const QString &prefix, int vmaj, int vmin,
                                     QDeclarativeScriptParser::Import::Type importType,
                                     QDeclarativeImportDatabase *database, QString *errorString)
{
   Q_ASSERT(typeLoader);
   QDeclarativeDirComponents qmldircomponents = qmldircomponentsnetwork;
   QString uri = uri_arg;
   QDeclarativeImportedNamespace *s;
   if (prefix.isEmpty()) {
      s = &unqualifiedset;
   } else {
      s = set.value(prefix);
      if (!s) {
         set.insert(prefix, (s = new QDeclarativeImportedNamespace));
      }
   }

   bool appendInstead = false;
   if (importType == QDeclarativeScriptParser::Import::Implicit) {
      //Treat same as a File import, but lower precedence
      appendInstead = true;
      importType = QDeclarativeScriptParser::Import::File;
   }

   QString url = uri;
   bool versionFound = false;
   if (importType == QDeclarativeScriptParser::Import::Library) {
      url.replace(QLatin1Char('.'), QLatin1Char('/'));
      bool found = false;
      QString dir;
      QString qmldir;

      // step 1: search for extension with fully encoded version number
      if (vmaj >= 0 && vmin >= 0) {
         foreach (const QString & p, database->fileImportPath) {
            dir = p + QLatin1Char('/') + url;
            qmldir = dir + QString(QLatin1String(".%1.%2")).arg(vmaj).arg(vmin) + QLatin1String("/qmldir");

            QString absoluteFilePath = typeLoader->absoluteFilePath(qmldir);
            if (!absoluteFilePath.isEmpty()) {
               found = true;

               QString absolutePath = absoluteFilePath.left(absoluteFilePath.lastIndexOf(QLatin1Char('/')));
               url = QUrl::fromLocalFile(absolutePath).toString();
               uri = resolvedUri(dir, database);
               if (!importExtension(absoluteFilePath, uri, database, &qmldircomponents, errorString)) {
                  return false;
               }
               break;
            }
         }
      }
      // step 2: search for extension with encoded version major
      if (vmaj >= 0 && vmin >= 0) {
         foreach (const QString & p, database->fileImportPath) {
            dir = p + QLatin1Char('/') + url;
            qmldir = dir + QString(QLatin1String(".%1")).arg(vmaj) + QLatin1String("/qmldir");

            QString absoluteFilePath = typeLoader->absoluteFilePath(qmldir);
            if (!absoluteFilePath.isEmpty()) {
               found = true;

               QString absolutePath = absoluteFilePath.left(absoluteFilePath.lastIndexOf(QLatin1Char('/')));
               url = QUrl::fromLocalFile(absolutePath).toString();
               uri = resolvedUri(dir, database);
               if (!importExtension(absoluteFilePath, uri, database, &qmldircomponents, errorString)) {
                  return false;
               }
               break;
            }
         }
      }
      if (!found) {
         // step 3: search for extension without version number

         foreach (const QString & p, database->fileImportPath) {
            dir = p + QLatin1Char('/') + url;
            qmldir = dir + QLatin1String("/qmldir");

            QString absoluteFilePath = typeLoader->absoluteFilePath(qmldir);
            if (!absoluteFilePath.isEmpty()) {
               found = true;

               QString absolutePath = absoluteFilePath.left(absoluteFilePath.lastIndexOf(QLatin1Char('/')));
               url = QUrl::fromLocalFile(absolutePath).toString();
               uri = resolvedUri(dir, database);
               if (!importExtension(absoluteFilePath, uri, database, &qmldircomponents, errorString)) {
                  return false;
               }
               break;
            }
         }
      }

      if (QDeclarativeMetaType::isModule(uri.toUtf8(), vmaj, vmin)) {
         versionFound = true;
      }

      //Load any type->file mappings registered for this uri
      qmldircomponents << QDeclarativeMetaType::qmlComponents(uri.toUtf8(), vmaj, vmin);

      if (!versionFound && qmldircomponents.isEmpty()) {
         if (errorString) {
            bool anyversion = QDeclarativeMetaType::isModule(uri.toUtf8(), -1, -1);
            if (anyversion) {
               *errorString = QDeclarativeImportDatabase::tr("module \"%1\" version %2.%3 is not installed").arg(uri_arg).arg(
                                 vmaj).arg(vmin);
            } else {
               *errorString = QDeclarativeImportDatabase::tr("module \"%1\" is not installed").arg(uri_arg);
            }
         }
         return false;
      }
   } else {

      if (importType == QDeclarativeScriptParser::Import::File && qmldircomponents.isEmpty()) {
         QUrl importUrl = base.resolved(QUrl(uri + QLatin1String("/qmldir")));
         QString localFileOrQrc = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(importUrl);
         if (!localFileOrQrc.isEmpty()) {
            QString dir = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(base.resolved(QUrl(uri)));
            QFileInfo dirinfo(dir);
            if (dir.isEmpty() || !dirinfo.exists() || !dirinfo.isDir()) {
               if (errorString) {
                  *errorString = QDeclarativeImportDatabase::tr("\"%1\": no such directory").arg(uri_arg);
               }
               return false; // local import dirs must exist
            }
            uri = resolvedUri(QDeclarativeEnginePrivate::urlToLocalFileOrQrc(base.resolved(QUrl(uri))), database);
            if (uri.endsWith(QLatin1Char('/'))) {
               uri.chop(1);
            }
            if (!typeLoader->absoluteFilePath(localFileOrQrc).isEmpty()) {
               if (!importExtension(localFileOrQrc, uri, database, &qmldircomponents, errorString)) {
                  return false;
               }
            }
         } else {
            if (prefix.isEmpty()) {
               // directory must at least exist for valid import
               QString localFileOrQrc = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(base.resolved(QUrl(uri)));
               QFileInfo dirinfo(localFileOrQrc);
               if (localFileOrQrc.isEmpty() || !dirinfo.exists() || !dirinfo.isDir()) {
                  if (errorString) {
                     if (localFileOrQrc.isEmpty()) {
                        *errorString = QDeclarativeImportDatabase::tr("import \"%1\" has no qmldir and no namespace").arg(uri);
                     } else {
                        *errorString = QDeclarativeImportDatabase::tr("\"%1\": no such directory").arg(uri);
                     }
                  }
                  return false;
               }
            }
         }
      }

      url = base.resolved(QUrl(url)).toString();
      if (url.endsWith(QLatin1Char('/'))) {
         url.chop(1);
      }
   }

   if (!versionFound && vmaj > -1 && vmin > -1 && !qmldircomponents.isEmpty()) {
      QList<QDeclarativeDirParser::Component>::ConstIterator it = qmldircomponents.begin();
      int lowest_maj = INT_MAX;
      int lowest_min = INT_MAX;
      int highest_maj = INT_MIN;
      int highest_min = INT_MIN;
      for (; it != qmldircomponents.end(); ++it) {
         if (it->majorVersion > highest_maj || (it->majorVersion == highest_maj && it->minorVersion > highest_min)) {
            highest_maj = it->majorVersion;
            highest_min = it->minorVersion;
         }
         if (it->majorVersion < lowest_maj || (it->majorVersion == lowest_maj && it->minorVersion < lowest_min)) {
            lowest_maj = it->majorVersion;
            lowest_min = it->minorVersion;
         }
      }
      if (lowest_maj > vmaj || (lowest_maj == vmaj && lowest_min > vmin)
            || highest_maj < vmaj || (highest_maj == vmaj && highest_min < vmin)) {
         *errorString = QDeclarativeImportDatabase::tr("module \"%1\" version %2.%3 is not installed").arg(uri_arg).arg(
                           vmaj).arg(vmin);
         return false;
      }
   }

   if (appendInstead) {
      s->uris.append(uri);
      s->urls.append(url);
      s->majversions.append(vmaj);
      s->minversions.append(vmin);
      s->isLibrary.append(importType == QDeclarativeScriptParser::Import::Library);
      s->qmlDirComponents.append(qmldircomponents);
   } else {
      s->uris.prepend(uri);
      s->urls.prepend(url);
      s->majversions.prepend(vmaj);
      s->minversions.prepend(vmin);
      s->isLibrary.prepend(importType == QDeclarativeScriptParser::Import::Library);
      s->qmlDirComponents.prepend(qmldircomponents);
   }
   return true;
}

bool QDeclarativeImportsPrivate::find(const QByteArray &type, int *vmajor, int *vminor, QDeclarativeType **type_return,
                                      QUrl *url_return, QString *errorString)
{
   Q_ASSERT(typeLoader);
   QDeclarativeImportedNamespace *s = 0;
   int slash = type.indexOf('/');
   if (slash >= 0) {
      QString namespaceName = QString::fromUtf8(type.left(slash));
      s = set.value(namespaceName);
      if (!s) {
         if (errorString) {
            *errorString = QDeclarativeImportDatabase::tr("- %1 is not a namespace").arg(namespaceName);
         }
         return false;
      }
      int nslash = type.indexOf('/', slash + 1);
      if (nslash > 0) {
         if (errorString) {
            *errorString = QDeclarativeImportDatabase::tr("- nested namespaces not allowed");
         }
         return false;
      }
   } else {
      s = &unqualifiedset;
   }
   QByteArray unqualifiedtype = slash < 0 ? type : type.mid(slash +
                                1); // common-case opt (QString::mid works fine, but slower)
   if (s) {
      if (s->find(typeLoader, unqualifiedtype, vmajor, vminor, type_return, url_return, &base, errorString)) {
         return true;
      }
      if (s->urls.count() == 1 && !s->isLibrary[0] && url_return && s != &unqualifiedset) {
         // qualified, and only 1 url
         *url_return = QUrl(s->urls[0] + QLatin1Char('/')).resolved(QUrl(QString::fromUtf8(unqualifiedtype) +
                       QLatin1String(".qml")));
         return true;
      }
   }

   return false;
}

QDeclarativeImportedNamespace *QDeclarativeImportsPrivate::findNamespace(const QString &type)
{
   return set.value(type);
}

bool QDeclarativeImportedNamespace::find(QDeclarativeTypeLoader *typeLoader, const QByteArray &type, int *vmajor,
      int *vminor, QDeclarativeType **type_return,
      QUrl *url_return, QUrl *base, QString *errorString)
{
   bool typeRecursionDetected = false;
   for (int i = 0; i < urls.count(); ++i) {
      if (find_helper(typeLoader, i, type, vmajor, vminor, type_return, url_return, base, &typeRecursionDetected)) {
         if (qmlCheckTypes()) {
            // check for type clashes
            for (int j = i + 1; j < urls.count(); ++j) {
               if (find_helper(typeLoader, j, type, vmajor, vminor, 0, 0, base)) {
                  if (errorString) {
                     QString u1 = urls.at(i);
                     QString u2 = urls.at(j);
                     if (base) {
                        QString b = base->toString();
                        int slash = b.lastIndexOf(QLatin1Char('/'));
                        if (slash >= 0) {
                           b = b.left(slash + 1);
                           QString l = b.left(slash);
                           if (u1.startsWith(b)) {
                              u1 = u1.mid(b.count());
                           } else if (u1 == l) {
                              u1 = QDeclarativeImportDatabase::tr("local directory");
                           }
                           if (u2.startsWith(b)) {
                              u2 = u2.mid(b.count());
                           } else if (u2 == l) {
                              u2 = QDeclarativeImportDatabase::tr("local directory");
                           }
                        }
                     }

                     if (u1 != u2)
                        *errorString
                           = QDeclarativeImportDatabase::tr("is ambiguous. Found in %1 and in %2")
                             .arg(u1).arg(u2);
                     else
                        *errorString
                           = QDeclarativeImportDatabase::tr("is ambiguous. Found in %1 in version %2.%3 and %4.%5")
                             .arg(u1)
                             .arg(majversions.at(i)).arg(minversions.at(i))
                             .arg(majversions.at(j)).arg(minversions.at(j));
                  }
                  return false;
               }
            }
         }
         return true;
      }
   }
   if (errorString) {
      if (typeRecursionDetected) {
         *errorString = QDeclarativeImportDatabase::tr("is instantiated recursively");
      } else {
         *errorString = QDeclarativeImportDatabase::tr("is not a type");
      }
   }
   return false;
}

/*!
\class QDeclarativeImportDatabase
\brief The QDeclarativeImportDatabase class manages the QML imports for a QDeclarativeEngine.
\internal
*/
QDeclarativeImportDatabase::QDeclarativeImportDatabase(QDeclarativeEngine *e)
   : engine(e)
{
   filePluginPath << QLatin1String(".");

   // Search order is applicationDirPath(), $QML_IMPORT_PATH, QLibraryInfo::ImportsPath

#ifndef QT_NO_SETTINGS
   QString installImportsPath =  QLibraryInfo::location(QLibraryInfo::ImportsPath);
   addImportPath(installImportsPath);

#endif // QT_NO_SETTINGS

   // env import paths
   QByteArray envImportPath = qgetenv("QML_IMPORT_PATH");
   if (!envImportPath.isEmpty()) {

#if defined(Q_OS_WIN)
      QLatin1Char pathSep(';');
#else
      QLatin1Char pathSep(':');
#endif
      QStringList paths = QString::fromLatin1(envImportPath).split(pathSep, QString::SkipEmptyParts);
      for (int ii = paths.count() - 1; ii >= 0; --ii) {
         addImportPath(paths.at(ii));
      }
   }

   addImportPath(QCoreApplication::applicationDirPath());
}

QDeclarativeImportDatabase::~QDeclarativeImportDatabase()
{
}

/*!
  \internal

  Adds information to \a imports such that subsequent calls to resolveType()
  will resolve types qualified by \a prefix by considering types found at the given \a uri.

  The uri is either a directory (if importType is FileImport), or a URI resolved using paths
  added via addImportPath() (if importType is LibraryImport).

  The \a prefix may be empty, in which case the import location is considered for
  unqualified types.

  The base URL must already have been set with Import::setBaseUrl().
*/
bool QDeclarativeImports::addImport(QDeclarativeImportDatabase *importDb,
                                    const QString &uri, const QString &prefix, int vmaj, int vmin,
                                    QDeclarativeScriptParser::Import::Type importType,
                                    const QDeclarativeDirComponents &qmldircomponentsnetwork,
                                    QString *errorString)
{
   if (qmlImportTrace())
      qDebug().nospace() << "QDeclarativeImports(" << qPrintable(baseUrl().toString()) << ")" << "::addImport: "
                         << uri << " " << vmaj << '.' << vmin << " "
                         << (importType == QDeclarativeScriptParser::Import::Library ? "Library" : "File")
                         << " as " << prefix;

   return d->add(qmldircomponentsnetwork, uri, prefix, vmaj, vmin, importType, importDb, errorString);
}

/*!
  \internal

  Returns the result of the merge of \a baseName with \a path, \a suffixes, and \a prefix.
  The \a prefix must contain the dot.

  \a qmldirPath is the location of the qmldir file.
 */
QString QDeclarativeImportDatabase::resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath,
      const QString &baseName, const QStringList &suffixes,
      const QString &prefix)
{
   QStringList searchPaths = filePluginPath;
   bool qmldirPluginPathIsRelative = QDir::isRelativePath(qmldirPluginPath);
   if (!qmldirPluginPathIsRelative) {
      searchPaths.prepend(qmldirPluginPath);
   }

   foreach (const QString & pluginPath, searchPaths) {

      QString resolvedPath;

      if (pluginPath == QLatin1String(".")) {
         if (qmldirPluginPathIsRelative) {
            resolvedPath = qmldirPath.absoluteFilePath(qmldirPluginPath);
         } else {
            resolvedPath = qmldirPath.absolutePath();
         }
      } else {
         resolvedPath = pluginPath;
      }

      // hack for resources, should probably go away
      if (resolvedPath.startsWith(QLatin1Char(':'))) {
         resolvedPath = QCoreApplication::applicationDirPath();
      }

      QDir dir(resolvedPath);
      foreach (const QString & suffix, suffixes) {
         QString pluginFileName = prefix;

         pluginFileName += baseName;
         pluginFileName += suffix;

         QFileInfo fileInfo(dir, pluginFileName);

         if (fileInfo.exists()) {
            return fileInfo.absoluteFilePath();
         }
      }
   }

   if (qmlImportTrace())
      qDebug() << "QDeclarativeImportDatabase::resolvePlugin: Could not resolve plugin" << baseName
               << "in" << qmldirPath.absolutePath();

   return QString();
}

/*!
  \internal

  Returns the result of the merge of \a baseName with \a dir and the platform suffix.

  \table
  \header \i Platform \i Valid suffixes
  \row \i Windows     \i \c .dll
  \row \i Unix/Linux  \i \c .so
  \row \i AIX  \i \c .a
  \row \i HP-UX       \i \c .sl, \c .so (HP-UXi)
  \row \i Mac OS X    \i \c .dylib, \c .bundle, \c .so
  \row \i Symbian     \i \c .dll
  \endtable

  Version number on unix are ignored.
*/
QString QDeclarativeImportDatabase::resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath,
      const QString &baseName)
{
#if defined(Q_OS_WIN32)
   return resolvePlugin(qmldirPath, qmldirPluginPath, baseName,
                        QStringList()
# ifdef QT_DEBUG
                        << QLatin1String("d.dll") // try a qmake-style debug build first
# endif
                        << QLatin1String(".dll"));

#else

# if defined(Q_OS_DARWIN)

   return resolvePlugin(qmldirPath, qmldirPluginPath, baseName,
                        QStringList()
# ifdef QT_DEBUG
                        << QLatin1String("_debug.dylib") // try a qmake-style debug build first
                        << QLatin1String(".dylib")
# else
                        << QLatin1String(".dylib")
                        << QLatin1String("_debug.dylib") // try a qmake-style debug build after
# endif
                        << QLatin1String(".so")
                        << QLatin1String(".bundle"),
                        QLatin1String("lib"));
# else  // Generic Unix
   QStringList validSuffixList;

#  if defined(Q_OS_HPUX)
   /*
       See "HP-UX Linker and Libraries User's Guide", section "Link-time Differences between PA-RISC and IPF":
       "In PA-RISC (PA-32 and PA-64) shared libraries are suffixed with .sl. In IPF (32-bit and 64-bit),
       the shared libraries are suffixed with .so. For compatibility, the IPF linker also supports the .sl suffix."
    */
   validSuffixList << QLatin1String(".sl");
#   if defined __ia64
   validSuffixList << QLatin1String(".so");
#   endif

#  elif defined(Q_OS_UNIX)
   validSuffixList << QLatin1String(".so");
#  endif

   // Examples of valid library names:
   //  libfoo.so

   return resolvePlugin(qmldirPath, qmldirPluginPath, baseName, validSuffixList, QLatin1String("lib"));
# endif

#endif
}

/*!
    \internal
*/
QStringList QDeclarativeImportDatabase::pluginPathList() const
{
   return filePluginPath;
}

/*!
    \internal
*/
void QDeclarativeImportDatabase::setPluginPathList(const QStringList &paths)
{
   filePluginPath = paths;
}

/*!
    \internal
*/
void QDeclarativeImportDatabase::addPluginPath(const QString &path)
{
   if (qmlImportTrace()) {
      qDebug().nospace() << "QDeclarativeImportDatabase::addPluginPath: " << path;
   }

   QUrl url = QUrl(path);
   if (url.isRelative() || url.scheme() == QLatin1String("file")
         || (url.scheme().length() == 1 && QFile::exists(path)) ) {  // windows path
      QDir dir = QDir(path);
      filePluginPath.prepend(dir.canonicalPath());
   } else {
      filePluginPath.prepend(path);
   }
}

/*!
    \internal
*/
void QDeclarativeImportDatabase::addImportPath(const QString &path)
{
   if (qmlImportTrace()) {
      qDebug().nospace() << "QDeclarativeImportDatabase::addImportPath: " << path;
   }

   if (path.isEmpty()) {
      return;
   }

   QUrl url = QUrl(path);
   QString cPath;

   if (url.isRelative() || url.scheme() == QLatin1String("file")
         || (url.scheme().length() == 1 && QFile::exists(path)) ) {  // windows path
      QDir dir = QDir(path);
      cPath = dir.canonicalPath();
   } else {
      cPath = path;
      cPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
   }

   if (!cPath.isEmpty()
         && !fileImportPath.contains(cPath)) {
      fileImportPath.prepend(cPath);
   }
}

/*!
    \internal
*/
QStringList QDeclarativeImportDatabase::importPathList() const
{
   return fileImportPath;
}

/*!
    \internal
*/
void QDeclarativeImportDatabase::setImportPathList(const QStringList &paths)
{
   fileImportPath = paths;
}

/*!
    \internal
*/
bool QDeclarativeImportDatabase::importPlugin(const QString &filePath, const QString &uri, QString *errorString)
{
   if (qmlImportTrace()) {
      qDebug().nospace() << "QDeclarativeImportDatabase::importPlugin: " << uri << " from " << filePath;
   }

   QFileInfo fileInfo(filePath);
   const QString absoluteFilePath = fileInfo.absoluteFilePath();

   bool engineInitialized = initializedPlugins.contains(absoluteFilePath);
   bool typesRegistered = qmlEnginePluginsWithRegisteredTypes()->contains(absoluteFilePath);

   if (typesRegistered) {
      Q_ASSERT_X(qmlEnginePluginsWithRegisteredTypes()->value(absoluteFilePath) == uri,
                 "QDeclarativeImportDatabase::importExtension",
                 "Internal error: Plugin imported previously with different uri");
   }

   if (!engineInitialized || !typesRegistered) {
      if (!QDeclarative_isFileCaseCorrect(absoluteFilePath)) {
         if (errorString) {
            *errorString = tr("File name case mismatch for \"%1\"").arg(absoluteFilePath);
         }
         return false;
      }
      QPluginLoader loader(absoluteFilePath);

      if (!loader.load()) {
         if (errorString) {
            *errorString = loader.errorString();
         }
         return false;
      }

      if (QDeclarativeExtensionInterface *iface = qobject_cast<QDeclarativeExtensionInterface *>(loader.instance())) {

         const QByteArray bytes = uri.toUtf8();
         const char *moduleId = bytes.constData();
         if (!typesRegistered) {

            // ### this code should probably be protected with a mutex.
            qmlEnginePluginsWithRegisteredTypes()->insert(absoluteFilePath, uri);
            iface->registerTypes(moduleId);
         }
         if (!engineInitialized) {
            // things on the engine (eg. adding new global objects) have to be done for every engine.

            // protect against double initialization
            initializedPlugins.insert(absoluteFilePath);
            iface->initializeEngine(engine, moduleId);
         }
      } else {
         if (errorString) {
            *errorString = loader.errorString();
         }
         return false;
      }
   }

   return true;
}

QT_END_NAMESPACE
