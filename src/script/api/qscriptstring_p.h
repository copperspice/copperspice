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

#ifndef QSCRIPTSTRING_P_H
#define QSCRIPTSTRING_P_H

#include <qshareddata.h>
#include "Identifier.h"

class QScriptEnginePrivate;

class QScriptStringPrivate : public QSharedData
{
 public:
   enum AllocationType {
      StackAllocated,
      HeapAllocated
   };

   inline QScriptStringPrivate(QScriptEnginePrivate *engine, const JSC::Identifier &id,
      AllocationType type);
   inline ~QScriptStringPrivate();
   static inline void init(QScriptString &q, QScriptStringPrivate *d);

   static inline QScriptStringPrivate *get(const QScriptString &q);

   inline void detachFromEngine();

   static inline bool isValid(const QScriptString &q);
   QScriptEnginePrivate *engine;
   JSC::Identifier identifier;
   AllocationType type;

   // linked list of engine's script values
   QScriptStringPrivate *prev;
   QScriptStringPrivate *next;
};

inline QScriptStringPrivate::QScriptStringPrivate(QScriptEnginePrivate *e, const JSC::Identifier &id,
   AllocationType tp)
   : engine(e), identifier(id), type(tp), prev(nullptr), next(nullptr)
{
}

inline QScriptStringPrivate::~QScriptStringPrivate()
{
}

inline void QScriptStringPrivate::init(QScriptString &q, QScriptStringPrivate *d)
{
   q.d_ptr = d;
}

inline QScriptStringPrivate *QScriptStringPrivate::get(const QScriptString &q)
{
   return const_cast<QScriptStringPrivate *>(q.d_func());
}

inline void QScriptStringPrivate::detachFromEngine()
{
   engine = nullptr;
   identifier = JSC::Identifier();
}

inline bool QScriptStringPrivate::isValid(const QScriptString &q)
{
   return (q.d_ptr && q.d_ptr->engine);
}

#endif
