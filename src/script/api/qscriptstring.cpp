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

#include "config.h"

#include <qscriptstring.h>
#include <qscriptstring_p.h>
#include <qscriptengine.h>
#include <qscriptengine_p.h>

QScriptString::QScriptString()
   : d_ptr(nullptr)
{
}

QScriptString::QScriptString(const QScriptString &other)
   : d_ptr(other.d_ptr)
{
   if (d_func() && (d_func()->type == QScriptStringPrivate::StackAllocated)) {
      Q_ASSERT(d_func()->ref.load() != 1);
      d_ptr.detach();
      d_func()->ref.store(1);
      d_func()->type = QScriptStringPrivate::HeapAllocated;
      d_func()->engine->registerScriptString(d_func());
   }
}

QScriptString::~QScriptString()
{
   Q_D(QScriptString);
   if (d) {
      switch (d->type) {
         case QScriptStringPrivate::StackAllocated:
            Q_ASSERT(d->ref.load() == 1);
            d->ref.ref(); // avoid deletion
            break;
         case QScriptStringPrivate::HeapAllocated:
            if (d->engine && (d->ref.load() == 1)) {
               // Make sure the identifier is removed from the correct engine.
               QScript::APIShim shim(d->engine);
               d->identifier = JSC::Identifier();
               d->engine->unregisterScriptString(d);
            }
            break;
      }
   }
}

QScriptString &QScriptString::operator=(const QScriptString &other)
{
   if (d_func() && d_func()->engine && (d_func()->ref.load() == 1) &&
      (d_func()->type == QScriptStringPrivate::HeapAllocated)) {
      // current d_ptr will be deleted at the assignment below, so unregister it first
      d_func()->engine->unregisterScriptString(d_func());
   }
   d_ptr = other.d_ptr;
   if (d_func() && (d_func()->type == QScriptStringPrivate::StackAllocated)) {
      Q_ASSERT(d_func()->ref.load() != 1);
      d_ptr.detach();
      d_func()->ref.store(1);
      d_func()->type = QScriptStringPrivate::HeapAllocated;
      d_func()->engine->registerScriptString(d_func());
   }
   return *this;
}

bool QScriptString::isValid() const
{
   return QScriptStringPrivate::isValid(*this);
}

bool QScriptString::operator==(const QScriptString &other) const
{
   Q_D(const QScriptString);
   if (!d || !other.d_func()) {
      return d == other.d_func();
   }
   return d->identifier == other.d_func()->identifier;
}

bool QScriptString::operator!=(const QScriptString &other) const
{
   return !operator==(other);
}

quint32 QScriptString::toArrayIndex(bool *ok) const
{
   Q_D(const QScriptString);
   if (!d) {
      if (ok) {
         *ok = false;
      }
      return -1;
   }
   bool tmp;
   bool *okok = ok ? ok : &tmp;
   quint32 result = d->identifier.toArrayIndex(okok);
   if (!*okok) {
      result = -1;
   }
   return result;
}

QString QScriptString::toString() const
{
   Q_D(const QScriptString);
   if (!d || !d->engine) {
      return QString();
   }
   return d->identifier.ustring();
}

QScriptString::operator QString() const
{
   return toString();
}

uint qHash(const QScriptString &key)
{
   QScriptStringPrivate *d = QScriptStringPrivate::get(key);
   if (!d) {
      return 0;
   }
   return qHash(d->identifier.ustring().rep());
}
