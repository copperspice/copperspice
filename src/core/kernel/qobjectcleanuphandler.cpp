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

#include <qobjectcleanuphandler.h>

QObjectCleanupHandler::QObjectCleanupHandler()
{
}

QObjectCleanupHandler::~QObjectCleanupHandler()
{
   clear();
}

QObject *QObjectCleanupHandler::add(QObject *object)
{
   if (! object) {
      return nullptr;
   }

   connect(object, &QObject::destroyed, this, &QObjectCleanupHandler::objectDestroyed);
   cleanupObjects.insert(0, object);

   return object;
}

void QObjectCleanupHandler::remove(QObject *object)
{
   int index;

   if ((index = cleanupObjects.indexOf(object)) != -1) {
      cleanupObjects.removeAt(index);
      disconnect(object, &QObject::destroyed, this, &QObjectCleanupHandler::objectDestroyed);
   }
}

bool QObjectCleanupHandler::isEmpty() const
{
   return cleanupObjects.isEmpty();
}

void QObjectCleanupHandler::clear()
{
   while (!cleanupObjects.isEmpty()) {
      delete cleanupObjects.takeFirst();
   }
}

void QObjectCleanupHandler::objectDestroyed(QObject *object)
{
   remove(object);
}

