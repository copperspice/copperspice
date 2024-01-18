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

#include <qdeclarativewatcher_p.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativecontext.h>
#include <qdeclarative.h>
#include <qdeclarativedebugservice_p.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativevaluetype_p.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QDeclarativeWatchProxy : public QObject
{
   DECL_CS_OBJECT(QDeclarativeWatchProxy)

 public:
   QDeclarativeWatchProxy(int id, QObject *object, int debugId,
                          const QMetaProperty &prop, QDeclarativeWatcher *parent = 0);

   QDeclarativeWatchProxy(int id, QDeclarativeExpression *exp,
                          int debugId, QDeclarativeWatcher *parent = 0);

 private:
   DECL_CS_SLOT_1(Private, void notifyValueChanged())
   DECL_CS_SLOT_2(notifyValueChanged)


   friend class QDeclarativeWatcher;
   int m_id;
   QDeclarativeWatcher *m_watch;
   QObject *m_object;
   int m_debugId;
   QMetaProperty m_property;

   QDeclarativeExpression *m_expr;
};

QDeclarativeWatchProxy::QDeclarativeWatchProxy(int id,
      QDeclarativeExpression *exp,
      int debugId,
      QDeclarativeWatcher *parent)
   : QObject(parent), m_id(id), m_watch(parent), m_object(0), m_debugId(debugId), m_expr(exp)
{
   QObject::connect(m_expr, SIGNAL(valueChanged()), this, SLOT(notifyValueChanged()));
}

QDeclarativeWatchProxy::QDeclarativeWatchProxy(int id,
      QObject *object,
      int debugId,
      const QMetaProperty &prop,
      QDeclarativeWatcher *parent)
   : QObject(parent), m_id(id), m_watch(parent), m_object(object), m_debugId(debugId), m_property(prop), m_expr(0)
{
   static int refreshIdx = -1;
   if (refreshIdx == -1) {
      refreshIdx = QDeclarativeWatchProxy::staticMetaObject.indexOfMethod("notifyValueChanged()");
   }

   if (prop.hasNotifySignal()) {
      QDeclarativePropertyPrivate::connect(m_object, prop.notifySignalIndex(), this, refreshIdx);
   }
}

void QDeclarativeWatchProxy::notifyValueChanged()
{
   QVariant v;
   if (m_expr) {
      v = m_expr->evaluate();
   } else if (QDeclarativeValueTypeFactory::isValueType(m_property.userType())) {
      v = m_property.read(m_object);
   }

   emit m_watch->propertyChanged(m_id, m_debugId, m_property, v);
}


QDeclarativeWatcher::QDeclarativeWatcher(QObject *parent)
   : QObject(parent)
{
}

bool QDeclarativeWatcher::addWatch(int id, quint32 debugId)
{
   QObject *object = QDeclarativeDebugService::objectForId(debugId);
   if (object) {
      int propCount = object->metaObject()->propertyCount();
      for (int ii = 0; ii < propCount; ii++) {
         addPropertyWatch(id, object, debugId, object->metaObject()->property(ii));
      }
      return true;
   }
   return false;
}

bool QDeclarativeWatcher::addWatch(int id, quint32 debugId, const QByteArray &property)
{
   QObject *object = QDeclarativeDebugService::objectForId(debugId);
   if (object) {
      int index = object->metaObject()->indexOfProperty(property.constData());
      if (index >= 0) {
         addPropertyWatch(id, object, debugId, object->metaObject()->property(index));
         return true;
      }
   }
   return false;
}

bool QDeclarativeWatcher::addWatch(int id, quint32 objectId, const QString &expr)
{
   QObject *object = QDeclarativeDebugService::objectForId(objectId);
   QDeclarativeContext *context = qmlContext(object);
   if (context) {
      QDeclarativeExpression *exprObj = new QDeclarativeExpression(context, object, expr);
      exprObj->setNotifyOnValueChanged(true);
      QDeclarativeWatchProxy *proxy = new QDeclarativeWatchProxy(id, exprObj, objectId, this);
      exprObj->setParent(proxy);
      m_proxies[id].append(proxy);
      proxy->notifyValueChanged();
      return true;
   }
   return false;
}

void QDeclarativeWatcher::removeWatch(int id)
{
   if (!m_proxies.contains(id)) {
      return;
   }

   QList<QPointer<QDeclarativeWatchProxy> > proxies = m_proxies.take(id);
   qDeleteAll(proxies);
}

void QDeclarativeWatcher::addPropertyWatch(int id, QObject *object, quint32 debugId, const QMetaProperty &property)
{
   QDeclarativeWatchProxy *proxy = new QDeclarativeWatchProxy(id, object, debugId, property, this);
   m_proxies[id].append(proxy);

   proxy->notifyValueChanged();
}

QT_END_NAMESPACE
