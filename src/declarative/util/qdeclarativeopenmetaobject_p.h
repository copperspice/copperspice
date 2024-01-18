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

#ifndef QDECLARATIVEOPENMETAOBJECT_P_H
#define QDECLARATIVEOPENMETAOBJECT_P_H

#include <QtCore/QMetaObject>
#include <QtCore/QObject>
#include <qdeclarativerefcount_p.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QMetaPropertyBuilder;
class QDeclarativeOpenMetaObjectTypePrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeOpenMetaObjectType : public QDeclarativeRefCount
{
 public:
   QDeclarativeOpenMetaObjectType(const QMetaObject *base, QDeclarativeEngine *engine);
   ~QDeclarativeOpenMetaObjectType();

   int createProperty(const QByteArray &name);

   int propertyOffset() const;
   int signalOffset() const;

 protected:
   virtual void propertyCreated(int, QMetaPropertyBuilder &);

 private:
   QDeclarativeOpenMetaObjectTypePrivate *d;
   friend class QDeclarativeOpenMetaObject;
   friend class QDeclarativeOpenMetaObjectPrivate;
};

class QDeclarativeOpenMetaObjectPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeOpenMetaObject : public QAbstractDynamicMetaObject
{
 public:
   QDeclarativeOpenMetaObject(QObject *, bool = true);
   QDeclarativeOpenMetaObject(QObject *, QDeclarativeOpenMetaObjectType *, bool = true);
   ~QDeclarativeOpenMetaObject();

   QVariant value(const QByteArray &) const;
   void setValue(const QByteArray &, const QVariant &);
   QVariant value(int) const;
   void setValue(int, const QVariant &);
   QVariant &operator[](const QByteArray &);
   QVariant &operator[](int);
   bool hasValue(int) const;

   int count() const;
   QByteArray name(int) const;

   QObject *object() const;
   virtual QVariant initialValue(int);

   // Be careful - once setCached(true) is called createProperty() is no
   // longer automatically called for new properties.
   void setCached(bool);

   QDeclarativeOpenMetaObjectType *type() const;

 protected:
   virtual int metaCall(QMetaObject::Call _c, int _id, void **_a);
   virtual int createProperty(const char *, const char *);

   virtual void propertyRead(int);
   virtual void propertyWrite(int);
   virtual void propertyWritten(int);
   virtual void propertyCreated(int, QMetaPropertyBuilder &);

   QAbstractDynamicMetaObject *parent() const;

 private:
   QDeclarativeOpenMetaObjectPrivate *d;
   friend class QDeclarativeOpenMetaObjectType;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEOPENMETAOBJECT_H
