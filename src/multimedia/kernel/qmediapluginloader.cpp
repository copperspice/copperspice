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

#include <qmediapluginloader_p.h>

#include <qcoreapplication.h>
#include <qmediaservice_provider_plugin.h>
#include <qset.h>

#include <qfactoryloader_p.h>

QMediaPluginLoader::QMediaPluginLoader(const QString &iid, const QString &location, Qt::CaseSensitivity caseSensitivity):
    m_iid(iid)
{
    m_location = QString("/%1").formatArg(location);
    m_factoryLoader = new QFactoryLoader(m_iid, m_location, caseSensitivity);
}

QMediaPluginLoader::~QMediaPluginLoader()
{
    delete m_factoryLoader;
}

QStringList QMediaPluginLoader::keys() const
{
   QStringList retval;

   auto keySet = m_factoryLoader->keySet();

   for (auto item : keySet) {
      retval.append(item);
   }

   return retval;
}

QObject *QMediaPluginLoader::instanceForKey(QString const &key)
{
    return m_factoryLoader->instance(key);
}

QList<QObject *> QMediaPluginLoader::instances(QString const &key)
{
   QList<QObject *> retval;

   auto librarySet = m_factoryLoader->librarySet(key);

   for (auto item : librarySet) {
      QObject *object = m_factoryLoader->instance(item);

      if (! retval.contains(object)) {
         retval.append(object);
      }
   }

   return retval;
}

