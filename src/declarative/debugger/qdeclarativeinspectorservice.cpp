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

#include <qdeclarativeinspectorservice_p.h>
#include <qdeclarativeinspectorinterface_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>

#include <QtDeclarative/QDeclarativeView>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QDeclarativeInspectorService, serviceInstance)

QDeclarativeInspectorService::QDeclarativeInspectorService()
   : QDeclarativeDebugService(QLatin1String("QDeclarativeObserverMode"))
   , m_inspectorPlugin(0)
{
}

QDeclarativeInspectorService *QDeclarativeInspectorService::instance()
{
   return serviceInstance();
}

void QDeclarativeInspectorService::addView(QDeclarativeView *view)
{
   m_views.append(view);
   updateStatus();
}

void QDeclarativeInspectorService::removeView(QDeclarativeView *view)
{
   m_views.removeAll(view);
   updateStatus();
}

void QDeclarativeInspectorService::sendMessage(const QByteArray &message)
{
   if (status() != Enabled) {
      return;
   }

   QDeclarativeDebugService::sendMessage(message);
}

void QDeclarativeInspectorService::statusChanged(Status status)
{
   Q_UNUSED(status);
   updateStatus();
}

void QDeclarativeInspectorService::updateStatus()
{
   if (m_views.isEmpty()) {
      if (m_inspectorPlugin) {
         m_inspectorPlugin->deactivate();
      }
      return;
   }

   if (status() == Enabled) {
      if (!m_inspectorPlugin) {
         m_inspectorPlugin = loadInspectorPlugin();
      }

      if (!m_inspectorPlugin) {
         qWarning() << "Error while loading inspector plugin";
         return;
      }

      m_inspectorPlugin->activate();
   } else {
      if (m_inspectorPlugin) {
         m_inspectorPlugin->deactivate();
      }
   }
}

void QDeclarativeInspectorService::messageReceived(const QByteArray &message)
{
   emit gotMessage(message);
}

QDeclarativeInspectorInterface *QDeclarativeInspectorService::loadInspectorPlugin()
{
   QStringList pluginCandidates;
   const QStringList paths = QCoreApplication::libraryPaths();
   foreach (const QString & libPath, paths) {
      const QDir dir(libPath + QLatin1String("/qmltooling"));
      if (dir.exists())
         foreach (const QString & pluginPath, dir.entryList(QDir::Files))
         pluginCandidates << dir.absoluteFilePath(pluginPath);
   }

   foreach (const QString & pluginPath, pluginCandidates) {
      QPluginLoader loader(pluginPath);
      if (!loader.load()) {
         continue;
      }

      QDeclarativeInspectorInterface *inspector =
         qobject_cast<QDeclarativeInspectorInterface *>(loader.instance());

      if (inspector) {
         return inspector;
      }
      loader.unload();
   }
   return 0;
}

QT_END_NAMESPACE
