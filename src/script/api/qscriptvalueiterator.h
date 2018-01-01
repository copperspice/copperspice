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

#ifndef QSCRIPTVALUEITERATOR_H
#define QSCRIPTVALUEITERATOR_H

#include <QtScript/qscriptvalue.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QString;
class QScriptString;
class QScriptValueIteratorPrivate;

class Q_SCRIPT_EXPORT QScriptValueIterator
{
 public:
   QScriptValueIterator(const QScriptValue &value);
   ~QScriptValueIterator();

   bool hasNext() const;
   void next();

   bool hasPrevious() const;
   void previous();

   QString name() const;
   QScriptString scriptName() const;

   QScriptValue value() const;
   void setValue(const QScriptValue &value);

   QScriptValue::PropertyFlags flags() const;

   void remove();

   void toFront();
   void toBack();

   QScriptValueIterator &operator=(QScriptValue &value);

 private:
   QScopedPointer<QScriptValueIteratorPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptValueIterator)
   Q_DISABLE_COPY(QScriptValueIterator)
};

QT_END_NAMESPACE

#endif // QSCRIPTVALUEITERATOR_H
