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

#ifndef QDECLARATIVELIST_H
#define QDECLARATIVELIST_H

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QObject;
class QMetaObject;

#ifndef QDECLARATIVELISTPROPERTY
#define QDECLARATIVELISTPROPERTY

template<typename T>
class QDeclarativeListProperty
{
 public:
   typedef void (*AppendFunction)(QDeclarativeListProperty<T> *, T *);
   typedef int (*CountFunction)(QDeclarativeListProperty<T> *);
   typedef T *(*AtFunction)(QDeclarativeListProperty<T> *, int);
   typedef void (*ClearFunction)(QDeclarativeListProperty<T> *);

   QDeclarativeListProperty()
      : object(0), data(0), append(0), count(0), at(0), clear(0), dummy1(0), dummy2(0) {}
   QDeclarativeListProperty(QObject *o, QList<T *> &list)
      : object(o), data(&list), append(qlist_append), count(qlist_count), at(qlist_at),
        clear(qlist_clear), dummy1(0), dummy2(0) {}
   QDeclarativeListProperty(QObject *o, void *d, AppendFunction a, CountFunction c = 0, AtFunction t = 0,
                            ClearFunction r = 0)
      : object(o), data(d), append(a), count(c), at(t), clear(r), dummy1(0), dummy2(0) {}

   bool operator==(const QDeclarativeListProperty &o) const {
      return object == o.object &&
             data == o.data &&
             append == o.append &&
             count == o.count &&
             at == o.at &&
             clear == o.clear;
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
      reinterpret_cast<QList<T *> *>(p->data)->append(v);
   }
   static int qlist_count(QDeclarativeListProperty *p) {
      return reinterpret_cast<QList<T *> *>(p->data)->count();
   }
   static T *qlist_at(QDeclarativeListProperty *p, int idx) {
      return reinterpret_cast<QList<T *> *>(p->data)->at(idx);
   }
   static void qlist_clear(QDeclarativeListProperty *p) {
      return reinterpret_cast<QList<T *> *>(p->data)->clear();
   }
};
#endif

class QDeclarativeEngine;
class QDeclarativeListReferencePrivate;

class Q_DECLARATIVE_EXPORT QDeclarativeListReference
{
 public:
   QDeclarativeListReference();
   QDeclarativeListReference(QObject *, const char *property, QDeclarativeEngine * = 0);
   QDeclarativeListReference(const QDeclarativeListReference &);
   QDeclarativeListReference &operator=(const QDeclarativeListReference &);
   ~QDeclarativeListReference();

   bool isValid() const;

   QObject *object() const;
   const QMetaObject *listElementType() const;

   bool canAppend() const;
   bool canAt() const;
   bool canClear() const;
   bool canCount() const;

   bool append(QObject *) const;
   QObject *at(int) const;
   bool clear() const;
   int count() const;

 private:
   friend class QDeclarativeListReferencePrivate;
   QDeclarativeListReferencePrivate *d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeListReference)

#endif // QDECLARATIVELIST_H
