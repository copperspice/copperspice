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

#ifndef QDECLARATIVEMETATYPE_P_H
#define QDECLARATIVEMETATYPE_P_H

#include <qdeclarative.h>
#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>
#include <QtCore/qbitarray.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativedirparser_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeCustomParser;
class QDeclarativeTypePrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeMetaType
{
 public:
   static bool canCopy(int type);
   static bool copy(int type, void *data, const void *copy = 0);

   static QList<QByteArray> qmlTypeNames();
   static QList<QDeclarativeType *> qmlTypes();

   static QDeclarativeType *qmlType(const QByteArray &, int, int);
   static QDeclarativeType *qmlType(const QMetaObject *);
   static QDeclarativeType *qmlType(const QMetaObject *metaObject, const QByteArray &module, int version_major,
                                    int version_minor);
   static QDeclarativeType *qmlType(int);

   static QDeclarativeDirComponents qmlComponents(const QByteArray &module, int version_major, int version_minor);

   static QMetaProperty defaultProperty(const QMetaObject *);
   static QMetaProperty defaultProperty(QObject *);
   static QMetaMethod defaultMethod(const QMetaObject *);
   static QMetaMethod defaultMethod(QObject *);

   static bool isQObject(int);
   static QObject *toQObject(const QVariant &, bool *ok = 0);

   static int listType(int);
   static int attachedPropertiesFuncId(const QMetaObject *);
   static QDeclarativeAttachedPropertiesFunc attachedPropertiesFuncById(int);

   enum TypeCategory { Unknown, Object, List };
   static TypeCategory typeCategory(int);

   static bool isInterface(int);
   static const char *interfaceIId(int);
   static bool isList(int);

   typedef QVariant (*StringConverter)(const QString &);
   static void registerCustomStringConverter(int, StringConverter);
   static StringConverter customStringConverter(int);

   static bool isModule(const QByteArray &module, int versionMajor, int versionMinor);

   static QList<QDeclarativePrivate::AutoParentFunction> parentFunctions();
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeType
{
 public:
   QByteArray typeName() const;
   QByteArray qmlTypeName() const;

   QByteArray module() const;
   int majorVersion() const;
   int minorVersion() const;

   bool availableInVersion(int vmajor, int vminor) const;
   bool availableInVersion(const QByteArray &module, int vmajor, int vminor) const;

   QObject *create() const;
   void create(QObject **, void **, size_t) const;

   typedef void (*CreateFunc)(void *);
   CreateFunc createFunction() const;
   int createSize() const;

   QDeclarativeCustomParser *customParser() const;

   bool isCreatable() const;
   bool isExtendedType() const;
   QString noCreationReason() const;

   bool isInterface() const;
   int typeId() const;
   int qListTypeId() const;

   const QMetaObject *metaObject() const;
   const QMetaObject *baseMetaObject() const;
   int metaObjectRevision() const;
   bool containsRevisionedAttributes() const;

   QDeclarativeAttachedPropertiesFunc attachedPropertiesFunction() const;
   const QMetaObject *attachedPropertiesType() const;
   int attachedPropertiesId() const;

   int parserStatusCast() const;
   QVariant fromObject(QObject *) const;
   const char *interfaceIId() const;
   int propertyValueSourceCast() const;
   int propertyValueInterceptorCast() const;

   int index() const;

 private:
   QDeclarativeType *superType() const;
   friend class QDeclarativeTypePrivate;
   friend struct QDeclarativeMetaTypeData;
   friend int registerType(const QDeclarativePrivate::RegisterType &);
   friend int registerInterface(const QDeclarativePrivate::RegisterInterface &);
   friend int registerComponent(const QDeclarativePrivate::RegisterComponent &);
   QDeclarativeType(int, const QDeclarativePrivate::RegisterInterface &);
   QDeclarativeType(int, const QDeclarativePrivate::RegisterType &);
   ~QDeclarativeType();

   QDeclarativeTypePrivate *d;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEMETATYPE_P_H

