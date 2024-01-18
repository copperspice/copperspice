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

#include "private/qdeclarativecleanup_p.h"

#include "private/qdeclarativeengine_p.h"

QT_BEGIN_NAMESPACE

/*!
\internal
\class QDeclarativeCleanup
\brief The QDeclarativeCleanup provides a callback when a QDeclarativeEngine is deleted.

Any object that needs cleanup to occur before the QDeclarativeEngine's QScriptEngine is
destroyed should inherit from QDeclarativeCleanup.  The clear() virtual method will be
called by QDeclarativeEngine just before it deletes the QScriptEngine.
*/

/*!
\internal

Create a QDeclarativeCleanup for \a engine
*/
QDeclarativeCleanup::QDeclarativeCleanup(QDeclarativeEngine *engine)
   : prev(0), next(0)
{
   if (!engine) {
      return;
   }

   QDeclarativeEnginePrivate *p = QDeclarativeEnginePrivate::get(engine);

   if (p->cleanup) {
      next = p->cleanup;
   }
   p->cleanup = this;
   prev = &p->cleanup;
   if (next) {
      next->prev = &next;
   }
}

/*!
\internal
*/
QDeclarativeCleanup::~QDeclarativeCleanup()
{
   if (prev) {
      *prev = next;
   }
   if (next) {
      next->prev = prev;
   }
   prev = 0;
   next = 0;
}
QT_END_NAMESPACE
