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

#ifndef QDECLARATIVELIST_P_H
#define QDECLARATIVELIST_P_H

#include "qdeclarativelist.h"
#include "qdeclarativeguard_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeListReferencePrivate
{
 public:
   QDeclarativeListReferencePrivate();

   static QDeclarativeListReference init(const QDeclarativeListProperty<QObject> &, int, QDeclarativeEngine *);

   QDeclarativeGuard<QObject> object;
   const QMetaObject *elementType;
   QDeclarativeListProperty<QObject> property;
   int propertyType;

   void addref();
   void release();
   int refCount;

   static inline QDeclarativeListReferencePrivate *get(QDeclarativeListReference *ref) {
      return ref->d;
   }
};


QT_END_NAMESPACE

#endif // QDECLARATIVELIST_P_H
