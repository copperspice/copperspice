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

#ifndef QSCRIPTVALUE_P_H
#define QSCRIPTVALUE_P_H

#include <qshareddata.h>

#include "wtf/Platform.h"
#include "JSValue.h"

#include <qstringfwd.h>

class QScriptEnginePrivate;
class QScriptValue;

class QScriptValuePrivate : public QSharedData
{
 public:
   enum Type {
      JavaScriptCore,
      Number,
      String
   };

   inline QScriptValuePrivate(QScriptEnginePrivate *);

   QScriptValuePrivate(const QScriptValuePrivate &) = delete;
   QScriptValuePrivate &operator=(const QScriptValuePrivate &) = delete;

   inline ~QScriptValuePrivate();

   inline void *operator new (size_t, QScriptEnginePrivate *);
   inline void operator delete (void *);

   inline void initFrom(JSC::JSValue value);
   inline void initFrom(qsreal value);
   inline void initFrom(const QString &value);

   inline bool isJSC() const;
   inline bool isObject() const;

   static inline QScriptValuePrivate *get(const QScriptValue &q) {
      return q.d_ptr.data();
   }

   static inline QScriptValue toPublic(QScriptValuePrivate *d) {
      return QScriptValue(d);
   }

   static inline QScriptEnginePrivate *getEngine(const QScriptValue &q) {
      if (! q.d_ptr) {
         return nullptr;
      }
      return q.d_ptr->engine;
   }

   inline JSC::JSValue property(const JSC::Identifier &id,
      const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

   inline JSC::JSValue property(quint32 index,
      const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

   inline JSC::JSValue property(const JSC::UString &,
      const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

   inline void setProperty(const JSC::UString &name, const JSC::JSValue &value,
      const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);
   inline void setProperty(const JSC::Identifier &id, const JSC::JSValue &value,
      const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);
   inline void setProperty(quint32 index, const JSC::JSValue &value,
      const QScriptValue::PropertyFlags &flags = QScriptValue::KeepExistingFlags);
   inline QScriptValue::PropertyFlags propertyFlags(
      const JSC::Identifier &id, const QScriptValue::ResolveFlags &mode = QScriptValue::ResolvePrototype) const;

   void detachFromEngine();

   qint64 objectId() {
      if ( (type == JavaScriptCore) && (engine) && jscValue.isCell() ) {
         return (qint64)jscValue.asCell();
      } else {
         return -1;
      }
   }

   QScriptEnginePrivate *engine;
   Type type;
   JSC::JSValue jscValue;
   qsreal numberValue;
   QString stringValue;

   // linked list of engine's script values
   QScriptValuePrivate *prev;
   QScriptValuePrivate *next;
};

inline QScriptValuePrivate::QScriptValuePrivate(QScriptEnginePrivate *e)
   : engine(e), prev(nullptr), next(nullptr)
{
}

inline bool QScriptValuePrivate::isJSC() const
{
   return (type == JavaScriptCore);
}

inline bool QScriptValuePrivate::isObject() const
{
   return isJSC() && jscValue && jscValue.isObject();
}

// Rest of inline functions implemented in qscriptengine_p.h

#endif
