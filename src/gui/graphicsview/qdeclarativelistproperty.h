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

#ifndef QDECLARATIVELISTPROPERTY_P_H
#define QDECLARATIVELISTPROPERTY_P_H

#include <qlist.h>

class QObject;

template <typename T>
class QDeclarativeListProperty
{
 public:
   typedef void (*AppendFunction)(QDeclarativeListProperty<T> *, T *);
   typedef int (*CountFunction)(QDeclarativeListProperty<T> *);
   typedef T *(*AtFunction)(QDeclarativeListProperty<T> *, int);
   typedef void (*ClearFunction)(QDeclarativeListProperty<T> *);

   QDeclarativeListProperty()
      : object(nullptr), data(nullptr), append(nullptr), count(nullptr), at(nullptr),
        clear(nullptr), dummy1(nullptr), dummy2(nullptr)
   {
   }

   QDeclarativeListProperty(QObject *o, QList<T *> &list)
      : object(o), data(&list), append(qlist_append), count(qlist_count), at(qlist_at),
        clear(qlist_clear), dummy1(nullptr), dummy2(nullptr)
   {
   }

   QDeclarativeListProperty(QObject *o, void *d, AppendFunction a, CountFunction c = nullptr,
               AtFunction t = nullptr, ClearFunction r = nullptr)
      : object(o), data(d), append(a), count(c), at(t), clear(r), dummy1(nullptr), dummy2(nullptr)
   {
   }

   bool operator==(const QDeclarativeListProperty &o) const {
      return object == o.object &&
         data   == o.data &&
         append == o.append &&
         count  == o.count &&
         at     == o.at &&
         clear  == o.clear;
   }

   QObject *object;
   void *data;

   AppendFunction append;

   CountFunction count;
   AtFunction at;

   ClearFunction clear;

   void *dummy1;
   void *dummy2;

 private:
   static void qlist_append(QDeclarativeListProperty *p, T *v) {
      ((QList<T *> *)p->data)->append(v);
   }

   static int qlist_count(QDeclarativeListProperty *p) {
      return ((QList<T *> *)p->data)->count();
   }

   static T *qlist_at(QDeclarativeListProperty *p, int idx) {
      return ((QList<T *> *)p->data)->at(idx);
   }

   static void qlist_clear(QDeclarativeListProperty *p) {
      return ((QList<T *> *)p->data)->clear();
   }
};

CS_REGISTER_TEMPLATE(QDeclarativeListProperty)

#endif
