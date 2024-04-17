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

#include <qdesktopservices.h>

#ifndef QT_NO_DESKTOPSERVICES

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qhash.h>
#include <qmutex.h>
#include <qobject.h>
#include <qplatform_integration.h>
#include <qplatform_services.h>
#include <qstandardpaths.h>
#include <qurl.h>

#include <qguiapplication_p.h>

class QOpenUrlHandlerRegistry : public QObject
{
   GUI_CS_OBJECT(QOpenUrlHandlerRegistry)

 public:
   QOpenUrlHandlerRegistry()
   {
   }

   QRecursiveMutex mutex;

   struct Handler {
      QObject *receiver;
      QByteArray name;
   };

   typedef QHash<QString, Handler> HandlerHash;
   HandlerHash handlers;

   GUI_CS_SLOT_1(Public, void handlerDestroyed(QObject *handler))
   GUI_CS_SLOT_2(handlerDestroyed)
};

static QOpenUrlHandlerRegistry *handlerRegistry()
{
   static QOpenUrlHandlerRegistry retval;
   return &retval;
}

void QOpenUrlHandlerRegistry::handlerDestroyed(QObject *handler)
{
   HandlerHash::iterator it = handlers.begin();

   while (it != handlers.end()) {
      if (it->receiver == handler) {
         it = handlers.erase(it);
      } else {
         ++it;
      }
   }
}

bool QDesktopServices::openUrl(const QUrl &url)
{
   QOpenUrlHandlerRegistry *registry = handlerRegistry();
   QRecursiveMutexLocker locker(&registry->mutex);

   static bool insideOpenUrlHandler = false;

   if (! insideOpenUrlHandler) {
      QOpenUrlHandlerRegistry::HandlerHash::const_iterator handler = registry->handlers.constFind(url.scheme());

      if (handler != registry->handlers.constEnd()) {
         insideOpenUrlHandler = true;

         bool result = QMetaObject::invokeMethod(handler->receiver, handler->name,
                     Qt::DirectConnection, Q_ARG(const QUrl &, url));

         insideOpenUrlHandler = false;
         return result;
      }
   }

   if (! url.isValid()) {
      return false;
   }

   QPlatformIntegration *platformIntegration = QGuiApplicationPrivate::platformIntegration();

   if (! platformIntegration) {
      return false;
   }

   QPlatformServices *platformServices = platformIntegration->services();

   if (! platformServices) {
      qWarning("QDesktopServices::openUrl() Platform plugin does not support services");
      return false;
   }

   return url.scheme() == "file" ?
         platformServices->openDocument(url) : platformServices->openUrl(url);
}

void QDesktopServices::setUrlHandler(const QString &scheme, QObject *receiver, const char *method)
{
   QOpenUrlHandlerRegistry *registry = handlerRegistry();
   QRecursiveMutexLocker locker(&registry->mutex);

   if (! receiver) {
      registry->handlers.remove(scheme.toLower());
      return;
   }

   QOpenUrlHandlerRegistry::Handler h;
   h.receiver = receiver;
   h.name = method;
   registry->handlers.insert(scheme.toLower(), h);

   QObject::connect(receiver, &QObject::destroyed, registry, &QOpenUrlHandlerRegistry::handlerDestroyed);
}

void QDesktopServices::unsetUrlHandler(const QString &scheme)
{
   setUrlHandler(scheme, nullptr, nullptr);
}

extern Q_CORE_EXPORT QString qt_applicationName_noFallback();

QString QDesktopServices::storageLocationImpl(QStandardPaths::StandardLocation type)
{
   if (type == QStandardPaths::AppLocalDataLocation) {

      // QCoreApplication::applicationName() must default to empty
      // Unix data location is under the "data/" subdirectory

      const QString compatAppName = qt_applicationName_noFallback();
      const QString baseDir       = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
      QString result = baseDir;

      if (! QCoreApplication::organizationName().isEmpty()) {
         result += '/' + QCoreApplication::organizationName();
      }

      if (! compatAppName.isEmpty()) {
         result += '/' + compatAppName;
      }

      return result;

#elif defined(Q_OS_UNIX)
      return baseDir + "/data/" + QCoreApplication::organizationName() + '/' + compatAppName;
#endif
   }

   return QStandardPaths::writableLocation(type);
}
#endif
