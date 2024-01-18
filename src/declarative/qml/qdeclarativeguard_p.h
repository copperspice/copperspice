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

#ifndef QDECLARATIVEGUARD_P_H
#define QDECLARATIVEGUARD_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>
#include <qdeclarativedata_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGuardImpl
{
 public:
   inline QDeclarativeGuardImpl();
   inline QDeclarativeGuardImpl(QObject *);
   inline QDeclarativeGuardImpl(const QDeclarativeGuardImpl &);
   inline ~QDeclarativeGuardImpl();

   QObject *o;
   QDeclarativeGuardImpl  *next;
   QDeclarativeGuardImpl **prev;

   inline void addGuard();
   inline void remGuard();
};

class QObject;
template<class T>
class QDeclarativeGuard : private QDeclarativeGuardImpl
{
   friend class QDeclarativeData;
 public:
   inline QDeclarativeGuard();
   inline QDeclarativeGuard(T *);
   inline QDeclarativeGuard(const QDeclarativeGuard<T> &);
   inline virtual ~QDeclarativeGuard();

   inline QDeclarativeGuard<T> &operator=(const QDeclarativeGuard<T> &o);
   inline QDeclarativeGuard<T> &operator=(T *);

   inline T *object() const;
   inline void setObject(T *g);

   inline bool isNull() const {
      return !o;
   }

   inline T *operator->() const {
      return static_cast<T *>(const_cast<QObject *>(o));
   }
   inline T &operator*() const {
      return *static_cast<T *>(const_cast<QObject *>(o));
   }
   inline operator T *() const {
      return static_cast<T *>(const_cast<QObject *>(o));
   }
   inline T *data() const {
      return static_cast<T *>(const_cast<QObject *>(o));
   }

 protected:
   virtual void objectDestroyed(T *) {}
};

QDeclarativeGuardImpl::QDeclarativeGuardImpl()
   : o(0), next(0), prev(0)
{
}

QDeclarativeGuardImpl::QDeclarativeGuardImpl(QObject *g)
   : o(g), next(0), prev(0)
{
   if (o) {
      addGuard();
   }
}

QDeclarativeGuardImpl::QDeclarativeGuardImpl(const QDeclarativeGuardImpl &g)
   : o(g.o), next(0), prev(0)
{
   if (o) {
      addGuard();
   }
}

QDeclarativeGuardImpl::~QDeclarativeGuardImpl()
{
   if (prev) {
      remGuard();
   }
   o = 0;
}

void QDeclarativeGuardImpl::addGuard()
{
   Q_ASSERT(!prev);

   if (QObjectPrivate::get(o)->wasDeleted) {
      return;
   }

   QDeclarativeData *data = QDeclarativeData::get(o, true);
   next = data->guards;
   if (next) {
      next->prev = &next;
   }
   data->guards = this;
   prev = &data->guards;
}

void QDeclarativeGuardImpl::remGuard()
{
   Q_ASSERT(prev);

   if (next) {
      next->prev = prev;
   }
   *prev = next;
   next = 0;
   prev = 0;
}

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard()
{
}

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard(T *g)
   : QDeclarativeGuardImpl(g)
{
}

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard(const QDeclarativeGuard<T> &g)
   : QDeclarativeGuardImpl(g)
{
}

template<class T>
QDeclarativeGuard<T>::~QDeclarativeGuard()
{
}

template<class T>
QDeclarativeGuard<T> &QDeclarativeGuard<T>::operator=(const QDeclarativeGuard<T> &g)
{
   setObject(g.object());
   return *this;
}

template<class T>
QDeclarativeGuard<T> &QDeclarativeGuard<T>::operator=(T *g)
{
   setObject(g);
   return *this;
}

template<class T>
T *QDeclarativeGuard<T>::object() const
{
   return static_cast<T *>(o);
};

template<class T>
void QDeclarativeGuard<T>::setObject(T *g)
{
   if (g != o) {
      if (prev) {
         remGuard();
      }
      o = g;
      if (o) {
         addGuard();
      }
   }
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeGuard<QObject>)

#endif // QDECLARATIVEGUARD_P_H
