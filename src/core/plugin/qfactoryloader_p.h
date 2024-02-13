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

#ifndef QFACTORYLOADER_P_H
#define QFACTORYLOADER_P_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmultimap.h>
#include <qset.h>
#include <qscopedpointer.h>

#include <qlibrary_p.h>

class QFactoryLoaderPrivate;

class Q_CORE_EXPORT QFactoryLoader : public QObject
{
   CORE_CS_OBJECT(QFactoryLoader)
   Q_DECLARE_PRIVATE(QFactoryLoader)

 public:
   explicit QFactoryLoader(const QString &iid, const QString &suffix = QString(), Qt::CaseSensitivity = Qt::CaseSensitive);
   ~QFactoryLoader();

#if defined(Q_OS_UNIX) && ! defined (Q_OS_DARWIN)
   QLibraryHandle *library(const QString &key) const;
#endif

   QObject *instance(QString key) const;
   QObject *instance(QLibraryHandle *library) const;

   QSet<QString> keySet() const;
   QSet<QLibraryHandle *> librarySet(QString key) const;

   void setup();
   static void refreshAll();

   struct PluginStatus {
      QString pathName;
      QString fileName;
      QString keyFound;
   };

/*
   QSet<QString> getPluginLocations() const {
      QSet<QString> retval;

      for (auto &item : mp_pluginsFound) {
         // duplicates will be tossed
         retval.insert(item.pathName);
      }

      return retval;
   }

   QVector<PluginStatus> getPluginStatus() const {
      return mp_pluginsFound;
   }
*/

 protected:
   QScopedPointer<QFactoryLoaderPrivate> d_ptr;

 private:
   mutable QMultiMap<QString, QLibraryHandle *> m_pluginMap;
   // QVector<PluginStatus> mp_pluginsFound;
};

template <class PluginInterface, class FactoryInterface, class ...Ts>
PluginInterface *cs_load_plugin(const QFactoryLoader *loader, const QString &key, const Ts &... Vs)
{
   QObject *factoryObject = loader->instance(key);

   if (factoryObject != nullptr) {
      FactoryInterface *factory = dynamic_cast<FactoryInterface *>(factoryObject);

      if (factory != nullptr) {
         PluginInterface *retval = factory->create(key, Vs...);

         if (retval != nullptr) {
            return retval;
         }
      }
   }

   return nullptr;
}

#endif
