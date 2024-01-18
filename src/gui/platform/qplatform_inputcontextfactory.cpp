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

#include <qplatform_inputcontextfactory_p.h>

#include <qplatform_inputcontext.h>
#include <qguiapplication.h>
#include <qdebug.h>

#include <qplatform_inputcontextplugin_p.h>
#include <qfactoryloader_p.h>

#include <stdlib.h>

#if ! defined(QT_NO_SETTINGS)
   static QFactoryLoader *loader()
   {
      static QFactoryLoader retval(QPlatformInputContextInterface_ID, "/platforminputcontexts", Qt::CaseInsensitive);
      return &retval;
   }
#endif

QStringList QPlatformInputContextFactory::keys()
{

#if ! defined(QT_NO_SETTINGS)
   auto keySet = loader()->keySet();
   QStringList retval(keySet.toList());
   return retval;

#else
    return QStringList();

#endif
}

QString QPlatformInputContextFactory::requested()
{
    QByteArray env = qgetenv("QT_IM_MODULE");
    return env.isNull() ? QString() : QString::fromUtf8(env);
}

QPlatformInputContext *QPlatformInputContextFactory::create(const QString& key)
{
#if ! defined(QT_NO_SETTINGS)
    if (! key.isEmpty()) {
        QStringList paramList  = key.split(':');
        const QString platform = paramList.takeFirst().toLower();

        QPlatformInputContext *ic = cs_load_plugin<QPlatformInputContext, QPlatformInputContextPlugin> (loader(), platform, paramList);

        if (ic && ic->isValid()) {
            return ic;
        }

        delete ic;
    }
#endif

    return nullptr;
}

QPlatformInputContext *QPlatformInputContextFactory::create()
{
   return create(requested());
}

