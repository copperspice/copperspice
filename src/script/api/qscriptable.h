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

#ifndef QSCRIPTABLE_H
#define QSCRIPTABLE_H

#include <qscopedpointer.h>

class QScriptEngine;
class QScriptContext;
class QScriptValue;
class QScriptablePrivate;

class Q_SCRIPT_EXPORT QScriptable
{
 public:
   QScriptable();

   QScriptable(const QScriptable &) = delete;
   QScriptable &operator=(const QScriptable &) = delete;

   ~QScriptable();

   QScriptEngine *engine() const;
   QScriptContext *context() const;
   QScriptValue thisObject() const;
   int argumentCount() const;
   QScriptValue argument(int index) const;

 private:
   Q_DECLARE_PRIVATE(QScriptable)
   QScopedPointer<QScriptablePrivate> d_ptr;
};

#endif
