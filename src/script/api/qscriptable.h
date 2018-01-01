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

#ifndef QSCRIPTABLE_H
#define QSCRIPTABLE_H

#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QScriptEngine;
class QScriptContext;
class QScriptValue;
class QScriptablePrivate;

class Q_SCRIPT_EXPORT QScriptable
{
 public:
   QScriptable();
   ~QScriptable();

   QScriptEngine *engine() const;
   QScriptContext *context() const;
   QScriptValue thisObject() const;
   int argumentCount() const;
   QScriptValue argument(int index) const;

 private:
   QScopedPointer<QScriptablePrivate> d_ptr;

   Q_DISABLE_COPY(QScriptable)
   Q_DECLARE_PRIVATE(QScriptable)
};


QT_END_NAMESPACE

#endif // QSCRIPTABLE_H
