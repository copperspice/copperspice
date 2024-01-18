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

#ifndef QDECLARATIVECONTEXT_P_H
#define QDECLARATIVECONTEXT_P_H

#include "qdeclarativecontext.h"
#include "qdeclarativedata_p.h"
#include "qdeclarativeintegercache_p.h"
#include "qdeclarativetypenamecache_p.h"
#include "qdeclarativenotifier_p.h"
#include "qdeclarativelist.h"
#include "qdeclarativeparser_p.h"
#include <QtCore/qhash.h>
#include <QtScript/qscriptvalue.h>
#include <QtCore/qset.h>
#include "qdeclarativeguard_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QDeclarativeExpression;
class QDeclarativeEngine;
class QDeclarativeExpression;
class QDeclarativeExpressionPrivate;
class QDeclarativeAbstractExpression;
class QDeclarativeCompiledBindings;
class QDeclarativeContextData;

class QDeclarativeContextPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeContext)

 public:
   QDeclarativeContextPrivate();

   QDeclarativeContextData *data;

   QList<QVariant> propertyValues;
   int notifyIndex;

   static QDeclarativeContextPrivate *get(QDeclarativeContext *context) {
      return static_cast<QDeclarativeContextPrivate *>(QObjectPrivate::get(context));
   }
   static QDeclarativeContext *get(QDeclarativeContextPrivate *context) {
      return static_cast<QDeclarativeContext *>(context->q_func());
   }

   // Only used for debugging
   QList<QPointer<QObject> > instances;

   static int context_count(QDeclarativeListProperty<QObject> *);
   static QObject *context_at(QDeclarativeListProperty<QObject> *, int);
};

class QDeclarativeComponentAttached;
class QDeclarativeGuardedContextData;
class QDeclarativeContextData
{
 public:
   QDeclarativeContextData();
   QDeclarativeContextData(QDeclarativeContext *);
   void clearContext();
   void destroy();
   void invalidate();

   inline bool isValid() const {
      return engine && (!isInternal || !contextObject || !QObjectPrivate::get(contextObject)->wasDeleted);
   }

   // My parent context and engine
   QDeclarativeContextData *parent;
   QDeclarativeEngine *engine;

   void setParent(QDeclarativeContextData *);
   void refreshExpressions();

   void addObject(QObject *);

   QUrl resolvedUrl(const QUrl &);

   // My containing QDeclarativeContext.  If isInternal is true this owns publicContext.
   // If internal is false publicContext owns this.
   QDeclarativeContext *asQDeclarativeContext();
   QDeclarativeContextPrivate *asQDeclarativeContextPrivate();
   bool isInternal;
   QDeclarativeContext *publicContext;

   // Property name cache
   QDeclarativeIntegerCache *propertyNames;

   // Context object
   QObject *contextObject;

   // Any script blocks that exist on this context
   QList<QScriptValue> importedScripts;
   void addImportedScript(const QDeclarativeParser::Object::ScriptBlock &script);

   // Context base url
   QUrl url;

   // List of imports that apply to this context
   QDeclarativeTypeNameCache *imports;

   // My children
   QDeclarativeContextData *childContexts;

   // My peers in parent's childContexts list
   QDeclarativeContextData  *nextChild;
   QDeclarativeContextData **prevChild;

   // Expressions that use this context
   QDeclarativeAbstractExpression *expressions;

   // Doubly-linked list of objects that are owned by this context
   QDeclarativeData *contextObjects;

   // Doubly-linked list of context guards (XXX merge with contextObjects)
   QDeclarativeGuardedContextData *contextGuards;

   // id guards
   struct ContextGuard : public QDeclarativeGuard<QObject> {
      ContextGuard() : context(0) {}
      inline ContextGuard &operator=(QObject *obj) {
         QDeclarativeGuard<QObject>::operator=(obj);
         return *this;
      }
      virtual void objectDestroyed(QObject *) {
         if (context->contextObject && !QObjectPrivate::get(context->contextObject)->wasDeleted) {
            bindings.notify();
         }
      }
      QDeclarativeContextData *context;
      QDeclarativeNotifier bindings;
   };
   ContextGuard *idValues;
   int idValueCount;
   void setIdProperty(int, QObject *);
   void setIdPropertyData(QDeclarativeIntegerCache *);

   // Optimized binding pointer
   QDeclarativeCompiledBindings *optimizedBindings;

   // Linked contexts. this owns linkedContext.
   QDeclarativeContextData *linkedContext;

   // Linked list of uses of the Component attached property in this
   // context
   QDeclarativeComponentAttached *componentAttached;

   // Return the outermost id for obj, if any.
   QString findObjectId(const QObject *obj) const;

   static QDeclarativeContextData *get(QDeclarativeContext *context) {
      return QDeclarativeContextPrivate::get(context)->data;
   }

 private:
   ~QDeclarativeContextData() {}
};

class QDeclarativeGuardedContextData
{
 public:
   inline QDeclarativeGuardedContextData();
   inline QDeclarativeGuardedContextData(QDeclarativeContextData *);
   inline ~QDeclarativeGuardedContextData();

   inline void setContextData(QDeclarativeContextData *);

   inline QDeclarativeContextData *contextData();

   inline operator QDeclarativeContextData *() const {
      return m_contextData;
   }
   inline QDeclarativeContextData *operator->() const {
      return m_contextData;
   }
   inline QDeclarativeGuardedContextData &operator=(QDeclarativeContextData *d);

 private:
   QDeclarativeGuardedContextData &operator=(const QDeclarativeGuardedContextData &);
   QDeclarativeGuardedContextData(const QDeclarativeGuardedContextData &);
   friend class QDeclarativeContextData;

   inline void clear();

   QDeclarativeContextData *m_contextData;
   QDeclarativeGuardedContextData  *m_next;
   QDeclarativeGuardedContextData **m_prev;
};

QDeclarativeGuardedContextData::QDeclarativeGuardedContextData()
   : m_contextData(0), m_next(0), m_prev(0)
{
}

QDeclarativeGuardedContextData::QDeclarativeGuardedContextData(QDeclarativeContextData *data)
   : m_contextData(0), m_next(0), m_prev(0)
{
   setContextData(data);
}

QDeclarativeGuardedContextData::~QDeclarativeGuardedContextData()
{
   clear();
}

void QDeclarativeGuardedContextData::setContextData(QDeclarativeContextData *contextData)
{
   clear();

   if (contextData) {
      m_contextData = contextData;
      m_next = contextData->contextGuards;
      if (m_next) {
         m_next->m_prev = &m_next;
      }
      m_prev = &contextData->contextGuards;
      contextData->contextGuards = this;
   }
}

QDeclarativeContextData *QDeclarativeGuardedContextData::contextData()
{
   return m_contextData;
}

void QDeclarativeGuardedContextData::clear()
{
   if (m_prev) {
      *m_prev = m_next;
      if (m_next) {
         m_next->m_prev = m_prev;
      }
      m_contextData = 0;
      m_next = 0;
      m_prev = 0;
   }
}

QDeclarativeGuardedContextData &
QDeclarativeGuardedContextData::operator=(QDeclarativeContextData *d)
{
   setContextData(d);
   return *this;
}

QT_END_NAMESPACE

#endif // QDECLARATIVECONTEXT_P_H
