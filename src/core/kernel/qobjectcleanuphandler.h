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

#ifndef QOBJECTCLEANUPHANDLER_H
#define QOBJECTCLEANUPHANDLER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QObjectCleanupHandler : public QObject
{
   CORE_CS_OBJECT(QObjectCleanupHandler)

 public:
   QObjectCleanupHandler();
   ~QObjectCleanupHandler();

   QObject *add(QObject *object);
   void remove(QObject *object);
   bool isEmpty() const;
   void clear();

   // ### move into d pointer
   QObjectList cleanupObjects;

 private :
   CORE_CS_SLOT_1(Private, void objectDestroyed(QObject *un_named_arg1))
   CORE_CS_SLOT_2(objectDestroyed)
};

QT_END_NAMESPACE

#endif 
