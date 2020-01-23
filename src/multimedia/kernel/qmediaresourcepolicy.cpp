/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qmediaresourcepolicy_p.h>

#include <qfactoryloader_p.h>
#include <qmediaresourcepolicyplugin_p.h>
#include <qmediaresourceset_p.h>

namespace {
class QDummyMediaPlayerResourceSet : public QMediaPlayerResourceSetInterface
{
 public:
   QDummyMediaPlayerResourceSet(QObject *parent)
      : QMediaPlayerResourceSetInterface(parent) {
   }

   bool isVideoEnabled() const {
      return true;
   }

   bool isGranted() const {
      return true;
   }

   bool isAvailable() const {
      return true;
   }

   void acquire() {}
   void release() {}
   void setVideoEnabled(bool) {}
};
}

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QMediaResourceSetFactoryInterface_iid, "/resourcepolicy", Qt::CaseInsensitive);
   return &retval;
}

Q_GLOBAL_STATIC(QObject, dummyRoot)

QObject *QMediaResourcePolicy::createResourceSet(const QString &interfaceId)
{
   QObject *obj = nullptr;

   QFactoryLoader *factoryObj = loader();

   if (factoryObj != nullptr) {
      QMediaResourceSetFactoryInterface *plugin = dynamic_cast<QMediaResourceSetFactoryInterface *>(factoryObj->instance("default"));

      if (plugin) {
         obj = plugin->create(interfaceId);
      }

      if (! obj) {
         if (interfaceId == QMediaPlayerResourceSetInterface_iid) {
            obj = new QDummyMediaPlayerResourceSet(dummyRoot());
         }
      }
   }

   Q_ASSERT(obj);

   return obj;
}

void QMediaResourcePolicy::destroyResourceSet(QObject *resourceSet)
{
   if (resourceSet->parent() == dummyRoot()) {
      delete resourceSet;
      return;
   }

   QFactoryLoader *factoryObj = loader();

   if (factoryObj == nullptr) {
      return;
   }

   QMediaResourceSetFactoryInterface *plugin = dynamic_cast<QMediaResourceSetFactoryInterface *>(factoryObj->instance("default"));
   Q_ASSERT(plugin);

   if (plugin == nullptr) {
      return;
   }

   return plugin->destroy(resourceSet);
}

