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

#ifndef QSCRIPTCLASSPROPERTYITERATOR_H
#define QSCRIPTCLASSPROPERTYITERATOR_H

#include <QtCore/qstring.h>

#include <QtCore/qscopedpointer.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QScriptClassPropertyIteratorPrivate;

class Q_SCRIPT_EXPORT QScriptClassPropertyIterator
{

 public:
   virtual ~QScriptClassPropertyIterator();

   QScriptValue object() const;

   virtual bool hasNext() const = 0;
   virtual void next() = 0;

   virtual bool hasPrevious() const = 0;
   virtual void previous() = 0;

   virtual void toFront() = 0;
   virtual void toBack() = 0;

   virtual QScriptString name() const = 0;
   virtual uint id() const;
   virtual QScriptValue::PropertyFlags flags() const;

 protected:
   QScriptClassPropertyIterator(const QScriptValue &object);
   QScriptClassPropertyIterator(const QScriptValue &object, QScriptClassPropertyIteratorPrivate &dd);
   QScopedPointer<QScriptClassPropertyIteratorPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QScriptClassPropertyIterator)
   Q_DISABLE_COPY(QScriptClassPropertyIterator)
};

QT_END_NAMESPACE

#endif
