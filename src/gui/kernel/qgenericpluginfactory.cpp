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

#include <qgenericpluginfactory.h>

#include <qguiapplication.h>
#include <qfactoryloader_p.h>
#include <qgenericplugin.h>
#include <qdebug.h>

#if ! defined(Q_OS_WIN) || defined(QT_SHARED)
   static QFactoryLoader *loader()
   {
      static QFactoryLoader retval(QGenericPluginInterface_ID, "/generic", Qt::CaseInsensitive);
      return &retval;
   }

#endif

QObject *QGenericPluginFactory::create(const QString &key, const QString &specification)
{
#if (! defined(Q_OS_WIN) || defined(QT_SHARED))
   const QString driver = key.toLower();

   if (QObject *object = cs_load_plugin<QObject, QGenericPlugin>(loader(), driver, specification)) {
      return object;
   }
#endif

   return nullptr;
}

QStringList QGenericPluginFactory::keys()
{
   QStringList list;

#if ! defined(Q_OS_WIN) || defined(QT_SHARED)
   auto keySet = loader()->keySet();
   list.append(keySet.toList());
#endif

   return list;
}


