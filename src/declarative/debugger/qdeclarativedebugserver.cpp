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

#include <qdeclarativedebugserver_p.h>
#include <qdeclarativedebugservice_p.h>
#include <qdeclarativedebugservice_p_p.h>
#include <qdeclarativeengine_p.h>

#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QStringList>

#include <private/qcoreapplication_p.h>

QT_BEGIN_NAMESPACE

/*
  QDeclarativeDebug Protocol (Version 1):

  handshake:
    1. Client sends
         "QDeclarativeDebugServer" 0 version pluginNames
       version: an int representing the highest protocol version the client knows
       pluginNames: plugins available on client side
    2. Server sends
         "QDeclarativeDebugClient" 0 version pluginNames
       version: an int representing the highest protocol version the client & server know
       pluginNames: plugins available on server side. plugins both in the client and server message are enabled.
  client plugin advertisement
    1. Client sends
         "QDeclarativeDebugServer" 1 pluginNames
  server plugin advertisement
    1. Server sends
         "QDeclarativeDebugClient" 1 pluginNames
  plugin communication:
       Everything send with a header different to "QDeclarativeDebugServer" is sent to the appropriate plugin.
  */

const int protocolVersion = 1;


class QDeclarativeDebugServerPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeDebugServer)

 public:
   QDeclarativeDebugServerPrivate();

   void advertisePlugins();

   QDeclarativeDebugServerConnection *connection;
   QHash<QString, QDeclarativeDebugService *> plugins;
   QStringList clientPlugins;
   bool gotHello;
   QString waitingForMsgFromService;
   bool waitingForMsgSucceeded;

 private:
   // private slot
   void _q_deliverMessage(const QString &serviceName, const QByteArray &message);
   static QDeclarativeDebugServerConnection *loadConnectionPlugin(QPluginLoader *loader, const QString &pluginName);

 protected:
   QDeclarativeDebugServer *q_ptr;

};

QDeclarativeDebugServerPrivate::QDeclarativeDebugServerPrivate()
   : connection(0), gotHello(false), waitingForMsgSucceeded(false)
{
}

void QDeclarativeDebugServerPrivate::advertisePlugins()
{
   if (!gotHello) {
      return;
   }

   QByteArray message;
   {
      QDataStream out(&message, QIODevice::WriteOnly);
      out << QString(QLatin1String("QDeclarativeDebugClient")) << 1 << plugins.keys();
   }
   connection->send(message);
}

QDeclarativeDebugServerConnection *QDeclarativeDebugServerPrivate::loadConnectionPlugin(
   QPluginLoader *loader, const QString &pluginName)
{
   QStringList pluginCandidates;
   const QStringList paths = QCoreApplication::libraryPaths();
   foreach (const QString & libPath, paths) {
      const QDir dir(libPath + QLatin1String("/qmltooling"));
      if (dir.exists()) {
         QStringList plugins(dir.entryList(QDir::Files));
         foreach (const QString & pluginPath, plugins) {
            if (QFileInfo(pluginPath).fileName().contains(pluginName)) {
               pluginCandidates << dir.absoluteFilePath(pluginPath);
            }
         }
      }
   }

   foreach (const QString & pluginPath, pluginCandidates) {
      loader->setFileName(pluginPath);
      if (!loader->load()) {
         continue;
      }
      QDeclarativeDebugServerConnection *connection = 0;
      if (QObject *instance = loader->instance()) {
         connection = qobject_cast<QDeclarativeDebugServerConnection *>(instance);
      }

      if (connection) {
         return connection;
      }
      loader->unload();
   }

   return 0;
}

bool QDeclarativeDebugServer::hasDebuggingClient() const
{
   Q_D(const QDeclarativeDebugServer);
   return d->connection
          && d->connection->isConnected()
          && d->gotHello;
}

QDeclarativeDebugServer *QDeclarativeDebugServer::instance()
{
   static bool commandLineTested = false;
   static QDeclarativeDebugServer *server = 0;

   if (!commandLineTested) {
      commandLineTested = true;

      QCoreApplicationPrivate *appD = static_cast<QCoreApplicationPrivate *>(QObjectPrivate::get(qApp));

#ifndef QDECLARATIVE_NO_DEBUG_PROTOCOL
      // ### remove port definition when protocol is changed
      int port   = 0;
      bool block = false;
      bool ok    = false;

      // format: qmljsdebugger=port:3768[,block] OR qmljsdebugger=ost[,block]
      if (!appD->qmljsDebugArguments().isEmpty()) {
         if (!QDeclarativeEnginePrivate::qml_debugging_enabled) {
            qWarning() << QString::fromLatin1(
                          "QDeclarativeDebugServer: Ignoring \"-qmljsdebugger=%1\". "
                          "Debugging has not been enabled.").arg(
                          appD->qmljsDebugArguments());
            return 0;
         }

         QString pluginName;
         if (appD->qmljsDebugArguments().indexOf(QLatin1String("port:")) == 0) {
            int separatorIndex = appD->qmljsDebugArguments().indexOf(QLatin1Char(','));
            port = appD->qmljsDebugArguments().mid(5, separatorIndex - 5).toInt(&ok);
            pluginName = QLatin1String("qmldbg_tcp");
         } else if (appD->qmljsDebugArguments().contains(QLatin1String("ost"))) {
            pluginName = QLatin1String("qmldbg_ost");
            ok = true;
         }

         block = appD->qmljsDebugArguments().contains(QLatin1String("block"));

         if (ok) {
            server = new QDeclarativeDebugServer();

            QPluginLoader *loader = new QPluginLoader(server);
            QDeclarativeDebugServerConnection *connection
               = QDeclarativeDebugServerPrivate::loadConnectionPlugin(loader, pluginName);
            if (connection) {
               server->d_func()->connection = connection;

               connection->setServer(server);
               connection->setPort(port, block);
            } else {
               qWarning() << QString::fromLatin1(
                             "QDeclarativeDebugServer: Ignoring \"-qmljsdebugger=%1\". "
                             "Remote debugger plugin has not been found.").arg(
                             appD->qmljsDebugArguments());
            }

         } else {
            qWarning() << QString::fromLatin1(
                          "QDeclarativeDebugServer: Ignoring \"-qmljsdebugger=%1\". "
                          "Format is -qmljsdebugger=port:<port>[,block]").arg(
                          appD->qmljsDebugArguments());
         }
      }
#else
      if (!appD->qmljsDebugArguments().isEmpty()) {
         qWarning() << QString::fromLatin1(
                       "QDeclarativeDebugServer: Ignoring \"-qmljsdebugger=%1\". "
                       "QtDeclarative is not configured for debugging.").arg(
                       appD->qmljsDebugArguments());
      }
#endif
   }

   return server;
}

QDeclarativeDebugServer::QDeclarativeDebugServer()
   : QObject(), d_ptr(new QDeclarativeDebugServerPrivate)
{
   d_ptr->q_ptr = this;
}

QDeclarativeDebugServer::~QDeclarativeDebugServer()
{
}

void QDeclarativeDebugServer::receiveMessage(const QByteArray &message)
{
   Q_D(QDeclarativeDebugServer);

   QDataStream in(message);
   if (!d->gotHello) {
      QString name;
      int op;
      in >> name >> op;

      if (name != QLatin1String("QDeclarativeDebugServer")
            || op != 0) {
         qWarning("QDeclarativeDebugServer: Invalid hello message");
         d->connection->disconnect();
         return;
      }

      int version;
      in >> version >> d->clientPlugins;

      // Send the hello answer immediately, since it needs to arrive before
      // the plugins below start sending messages.
      QByteArray helloAnswer;
      {
         QDataStream out(&helloAnswer, QIODevice::WriteOnly);
         out << QString(QLatin1String("QDeclarativeDebugClient")) << 0 << protocolVersion << d->plugins.keys();
      }
      d->connection->send(helloAnswer);

      d->gotHello = true;

      QHash<QString, QDeclarativeDebugService *>::Iterator iter = d->plugins.begin();
      for (; iter != d->plugins.end(); ++iter) {
         QDeclarativeDebugService::Status newStatus = QDeclarativeDebugService::Unavailable;
         if (d->clientPlugins.contains(iter.key())) {
            newStatus = QDeclarativeDebugService::Enabled;
         }
         iter.value()->d_func()->status = newStatus;
         iter.value()->statusChanged(newStatus);
      }

      qDebug("QDeclarativeDebugServer: Connection established");
   } else {

      QString debugServer(QLatin1String("QDeclarativeDebugServer"));

      QString name;
      in >> name;

      if (name == debugServer) {
         int op = -1;
         in >> op;

         if (op == 1) {
            // Service Discovery
            QStringList oldClientPlugins = d->clientPlugins;
            in >> d->clientPlugins;

            QHash<QString, QDeclarativeDebugService *>::Iterator iter = d->plugins.begin();
            for (; iter != d->plugins.end(); ++iter) {
               const QString pluginName = iter.key();
               QDeclarativeDebugService::Status newStatus = QDeclarativeDebugService::Unavailable;
               if (d->clientPlugins.contains(pluginName)) {
                  newStatus = QDeclarativeDebugService::Enabled;
               }

               if (oldClientPlugins.contains(pluginName)
                     != d->clientPlugins.contains(pluginName)) {
                  iter.value()->d_func()->status = newStatus;
                  iter.value()->statusChanged(newStatus);
               }
            }
         } else {
            qWarning("QDeclarativeDebugServer: Invalid control message %d", op);
         }
      } else {
         QByteArray message;
         in >> message;

         if (d->waitingForMsgFromService == name) {
            // deliver directly so that it is delivered before waitForMessage is returning.
            d->_q_deliverMessage(name, message);
            d->waitingForMsgSucceeded = true;
         } else {
            // deliver message in next event loop run.
            // Fixes the case that the service does start it's own event loop ...,
            // but the networking code doesn't deliver any new messages because readyRead
            // hasn't returned.
            QMetaObject::invokeMethod(this, "_q_deliverMessage", Qt::QueuedConnection,
                                      Q_ARG(QString, name),
                                      Q_ARG(QByteArray, message));
         }
      }
   }
}

void QDeclarativeDebugServerPrivate::_q_deliverMessage(const QString &serviceName, const QByteArray &message)
{
   QHash<QString, QDeclarativeDebugService *>::Iterator iter = plugins.find(serviceName);
   if (iter == plugins.end()) {
      qWarning() << "QDeclarativeDebugServer: Message received for missing plugin" << serviceName;
   } else {
      (*iter)->messageReceived(message);
   }
}

QList<QDeclarativeDebugService *> QDeclarativeDebugServer::services() const
{
   const Q_D(QDeclarativeDebugServer);
   return d->plugins.values();
}

QStringList QDeclarativeDebugServer::serviceNames() const
{
   const Q_D(QDeclarativeDebugServer);
   return d->plugins.keys();
}

bool QDeclarativeDebugServer::addService(QDeclarativeDebugService *service)
{
   Q_D(QDeclarativeDebugServer);
   if (!service || d->plugins.contains(service->name())) {
      return false;
   }

   d->plugins.insert(service->name(), service);
   d->advertisePlugins();

   QDeclarativeDebugService::Status newStatus = QDeclarativeDebugService::Unavailable;
   if (d->clientPlugins.contains(service->name())) {
      newStatus = QDeclarativeDebugService::Enabled;
   }
   service->d_func()->status = newStatus;
   service->statusChanged(newStatus);
   return true;
}

bool QDeclarativeDebugServer::removeService(QDeclarativeDebugService *service)
{
   Q_D(QDeclarativeDebugServer);
   if (!service || !d->plugins.contains(service->name())) {
      return false;
   }

   d->plugins.remove(service->name());
   d->advertisePlugins();

   QDeclarativeDebugService::Status newStatus = QDeclarativeDebugService::NotConnected;
   service->d_func()->server = 0;
   service->d_func()->status = newStatus;
   service->statusChanged(newStatus);
   return true;
}

void QDeclarativeDebugServer::sendMessage(QDeclarativeDebugService *service,
      const QByteArray &message)
{
   Q_D(QDeclarativeDebugServer);
   QByteArray msg;
   {
      QDataStream out(&msg, QIODevice::WriteOnly);
      out << service->name() << message;
   }
   d->connection->send(msg);
}

bool QDeclarativeDebugServer::waitForMessage(QDeclarativeDebugService *service)
{
   Q_D(QDeclarativeDebugServer);

   if (!service
         || !d->plugins.contains(service->name())
         || !d->waitingForMsgFromService.isEmpty()) {
      return false;
   }

   d->waitingForMsgFromService = service->name();

   do {
      d->connection->waitForMessage();
   } while (!d->waitingForMsgSucceeded);
   d->waitingForMsgSucceeded = false;
   d->waitingForMsgFromService.clear();
   return true;
}

void QDeclarativeDebugServer::_q_deliverMessage(QString un_named_arg1, QByteArray un_named_arg2)
{
   Q_D(QDeclarativeDebugServer);
   d->_q_deliverMessage(un_named_arg1, un_named_arg2);
}

QT_END_NAMESPACE
