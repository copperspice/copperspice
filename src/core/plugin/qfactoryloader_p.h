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

#ifndef QFACTORYLOADER_P_H
#define QFACTORYLOADER_P_H

#include <qobject.h>
#include <qstringlist.h>
#include <qjsonobject.h>
#include <qmap.h>
#include <QScopedPointer>

#include <qlibrary_p.h>

class QFactoryLoaderPrivate;

class Q_CORE_EXPORT QFactoryLoader : public QObject
{
   CORE_CS_OBJECT(QFactoryLoader)
   Q_DECLARE_PRIVATE(QFactoryLoader)

 public:
   explicit QFactoryLoader(const QString &iid, const QString &suffix = QString(), Qt::CaseSensitivity = Qt::CaseSensitive);
   ~QFactoryLoader();

   QList<QJsonObject> metaData() const;
   QObject *instance(int index) const;

#if defined(Q_OS_UNIX) && ! defined (Q_OS_MAC)
   QLibraryPrivate *library(const QString &key) const;
#endif

   QMultiMap<int, QString> keyMap() const;
   int indexOf(const QString &needle) const;
   void update();
   static void refreshAll();

 protected:
   QScopedPointer<QFactoryLoaderPrivate> d_ptr;
};

template <class PluginInterface, class FactoryInterface>
PluginInterface *qLoadPlugin(const QFactoryLoader *loader, const QString &key)
{
   const int index = loader->indexOf(key);
   if (index != -1) {
      QObject *factoryObject = loader->instance(index);
      if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
         if (PluginInterface *result = factory->create(key)) {
            return result;
         }
   }
   return 0;
}

template <class PluginInterface, class FactoryInterface, class Parameter1>
PluginInterface *qLoadPlugin1(const QFactoryLoader *loader,
   const QString &key,
   const Parameter1 &parameter1)
{
   const int index = loader->indexOf(key);
   if (index != -1) {
      QObject *factoryObject = loader->instance(index);
      if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
         if (PluginInterface *result = factory->create(key, parameter1)) {
            return result;
         }
   }
   return 0;
}

#endif
