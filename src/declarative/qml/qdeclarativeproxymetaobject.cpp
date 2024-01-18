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

#include "private/qdeclarativeproxymetaobject_p.h"
#include "private/qdeclarativeproperty_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeProxyMetaObject::QDeclarativeProxyMetaObject(QObject *obj, QList<ProxyData> *mList)
   : metaObjects(mList), proxies(0), parent(0), object(obj)
{
   *static_cast<QMetaObject *>(this) = *metaObjects->first().metaObject;

   QObjectPrivate *op = QObjectPrivate::get(obj);
   if (op->metaObject) {
      parent = static_cast<QAbstractDynamicMetaObject *>(op->metaObject);
   }

   op->metaObject = this;
}

QDeclarativeProxyMetaObject::~QDeclarativeProxyMetaObject()
{
   if (parent) {
      delete parent;
   }
   parent = 0;

   if (proxies) {
      delete [] proxies;
   }
   proxies = 0;
}

int QDeclarativeProxyMetaObject::metaCall(QMetaObject::Call c, int id, void **a)
{
   if ((c == QMetaObject::ReadProperty ||
         c == QMetaObject::WriteProperty) &&
         id >= metaObjects->last().propertyOffset) {

      for (int ii = 0; ii < metaObjects->count(); ++ii) {
         const ProxyData &data = metaObjects->at(ii);
         if (id >= data.propertyOffset) {
            if (!proxies) {
               proxies = new QObject*[metaObjects->count()];
               ::memset(proxies, 0,
                        sizeof(QObject *) * metaObjects->count());
            }

            if (!proxies[ii]) {
               QObject *proxy = data.createFunc(object);
               const QMetaObject *metaObject = proxy->metaObject();
               proxies[ii] = proxy;

               int localOffset = data.metaObject->methodOffset();
               int methodOffset = metaObject->methodOffset();
               int methods = metaObject->methodCount() - methodOffset;

               // ### - Can this be done more optimally?
               for (int jj = 0; jj < methods; ++jj) {
                  QMetaMethod method =
                     metaObject->method(jj + methodOffset);
                  if (method.methodType() == QMetaMethod::Signal) {
                     QDeclarativePropertyPrivate::connect(proxy, methodOffset + jj, object, localOffset + jj);
                  }
               }
            }

            int proxyOffset = proxies[ii]->metaObject()->propertyOffset();
            int proxyId = id - data.propertyOffset + proxyOffset;

            return proxies[ii]->qt_metacall(c, proxyId, a);
         }
      }
   } else if (c == QMetaObject::InvokeMetaMethod &&
              id >= metaObjects->last().methodOffset) {
      QMetaMethod m = object->metaObject()->method(id);
      if (m.methodType() == QMetaMethod::Signal) {
         QMetaObject::activate(object, id, a);
         return -1;
      }
   }

   if (parent) {
      return parent->metaCall(c, id, a);
   } else {
      return object->qt_metacall(c, id, a);
   }
}

QT_END_NAMESPACE
