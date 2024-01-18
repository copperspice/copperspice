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

#ifndef QDECLARATIVEFASTPROPERTIES_P_H
#define QDECLARATIVEFASTPROPERTIES_P_H

#include <QtCore/qvector.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QObject;
class QDeclarativeNotifierEndpoint;

class QDeclarativeFastProperties
{
 public:
   typedef void (*Accessor)(QObject *object, void *output, QDeclarativeNotifierEndpoint *endpoint);

   QDeclarativeFastProperties();

   Accessor accessor(int index) const {
      return m_accessors.at(index);
   }
   int accessorIndexForProperty(const QMetaObject *, int);

 private:
   void add(const QMetaObject *, int, Accessor);

   QHash<QPair<const QMetaObject *, int>, int> m_index;
   QVector<Accessor> m_accessors;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEFASTPROPERTIES_P_H
