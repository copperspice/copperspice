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

#include <qdesktopservices.h>

#ifndef QT_NO_DESKTOPSERVICES

#include <qdebug.h>

#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
#include <qdesktopservices_qws.cpp>

#elif defined(Q_WS_X11)
#include <qdesktopservices_x11.cpp>

#elif defined(Q_OS_WIN)
#include <qdesktopservices_win.cpp>

#elif defined(Q_OS_MAC)
#include <qdesktopservices_mac.cpp>

#endif

#include <qhash.h>
#include <qobject.h>
#include <qcoreapplication.h>
#include <qurl.h>
#include <qmutex.h>

class QOpenUrlHandlerRegistry : public QObject
{
   GUI_CS_OBJECT(QOpenUrlHandlerRegistry)

 public:
   inline QOpenUrlHandlerRegistry() : mutex(QMutex::Recursive) {}

   QMutex mutex;

   struct Handler {
      QObject *receiver;
      QByteArray name;
   };

   typedef QHash<QString, Handler> HandlerHash;
   HandlerHash handlers;

   GUI_CS_SLOT_1(Public, void handlerDestroyed(QObject *handler))
   GUI_CS_SLOT_2(handlerDestroyed)
};

Q_GLOBAL_STATIC(QOpenUrlHandlerRegistry, handlerRegistry)

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
   QMutexLocker locker(&registry->mutex);
   static bool insideOpenUrlHandler = false;

   if (!insideOpenUrlHandler) {
      QOpenUrlHandlerRegistry::HandlerHash::const_iterator handler = registry->handlers.constFind(url.scheme());

      if (handler != registry->handlers.constEnd()) {
         insideOpenUrlHandler = true;

         bool result = QMetaObject::invokeMethod(handler->receiver, handler->name, Qt::DirectConnection, Q_ARG(const QUrl &, url));
         insideOpenUrlHandler = false;
         return result;
      }
   }

   bool result;
   if (url.scheme() == QLatin1String("file")) {
      result = openDocument(url);
   } else {
      result = launchWebBrowser(url);
   }

   return result;
}

void QDesktopServices::setUrlHandler(const QString &scheme, QObject *receiver, const char *method)
{
   QOpenUrlHandlerRegistry *registry = handlerRegistry();
   QMutexLocker locker(&registry->mutex);
   if (!receiver) {
      registry->handlers.remove(scheme);
      return;
   }
   QOpenUrlHandlerRegistry::Handler h;
   h.receiver = receiver;
   h.name = method;
   registry->handlers.insert(scheme, h);
   QObject::connect(receiver, SIGNAL(destroyed(QObject *)),
                    registry, SLOT(handlerDestroyed(QObject *)));
}

void QDesktopServices::unsetUrlHandler(const QString &scheme)
{
   setUrlHandler(scheme, 0, 0);
}


#endif // QT_NO_DESKTOPSERVICES
