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

#ifndef QDECLARATIVEPROXYMETAOBJECT_P_H
#define QDECLARATIVEPROXYMETAOBJECT_P_H

#include <qmetaobjectbuilder_p.h>
#include <qdeclarative.h>
#include <QtCore/QMetaObject>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QDeclarativeProxyMetaObject : public QAbstractDynamicMetaObject
{
 public:
   struct ProxyData {
      typedef QObject *(*CreateFunc)(QObject *);
      QMetaObject *metaObject;
      CreateFunc createFunc;
      int propertyOffset;
      int methodOffset;
   };

   QDeclarativeProxyMetaObject(QObject *, QList<ProxyData> *);
   virtual ~QDeclarativeProxyMetaObject();

 protected:
   virtual int metaCall(QMetaObject::Call _c, int _id, void **_a);

 private:
   QList<ProxyData> *metaObjects;
   QObject **proxies;

   QAbstractDynamicMetaObject *parent;
   QObject *object;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEPROXYMETAOBJECT_P_H

