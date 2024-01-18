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

#ifndef QDECLARATIVEPROPERTY_P_H
#define QDECLARATIVEPROPERTY_P_H

#include <qdeclarativeproperty.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativepropertycache_p.h>
#include <qdeclarativeguard_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QDeclarativeEnginePrivate;
class QDeclarativeExpression;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativePropertyPrivate : public QDeclarativeRefCount
{
 public:
   enum WriteFlag { BypassInterceptor = 0x01, DontRemoveBinding = 0x02, RemoveBindingOnAliasWrite = 0x04 };
   using WriteFlags = QFlags<WriteFlag>;

   QDeclarativePropertyPrivate()
      : context(0), engine(0), object(0), isNameCached(false) {}

   QDeclarativeContextData *context;
   QDeclarativeEngine *engine;
   QDeclarativeGuard<QObject> object;

   bool isNameCached: 1;
   QDeclarativePropertyCache::Data core;
   QString nameCache;

   // Describes the "virtual" value-type sub-property.
   QDeclarativePropertyCache::ValueTypeData valueType;

   void initProperty(QObject *obj, const QString &name);
   void initDefault(QObject *obj);

   bool isValueType() const;
   int propertyType() const;
   QDeclarativeProperty::Type type() const;
   QDeclarativeProperty::PropertyTypeCategory propertyTypeCategory() const;

   QVariant readValueProperty();
   bool writeValueProperty(const QVariant &, WriteFlags);

   static const QMetaObject *rawMetaObjectForType(QDeclarativeEnginePrivate *, int);
   static bool writeEnumProperty(const QMetaProperty &prop, int idx, QObject *object,
                                 const QVariant &value, int flags);
   static bool write(QObject *, const QDeclarativePropertyCache::Data &, const QVariant &,
                     QDeclarativeContextData *, WriteFlags flags = 0);
   static void findAliasTarget(QObject *, int, QObject **, int *);
   static QDeclarativeAbstractBinding *setBinding(QObject *, int coreIndex, int valueTypeIndex /* -1 */,
         QDeclarativeAbstractBinding *,
         WriteFlags flags = DontRemoveBinding);
   static QDeclarativeAbstractBinding *setBindingNoEnable(QObject *, int coreIndex, int valueTypeIndex /* -1 */,
         QDeclarativeAbstractBinding *);
   static QDeclarativeAbstractBinding *binding(QObject *, int coreIndex, int valueTypeIndex /* -1 */);

   static QByteArray saveValueType(const QMetaObject *, int,
                                   const QMetaObject *, int);
   static QByteArray saveProperty(const QMetaObject *, int);

   static QDeclarativeProperty restore(const QByteArray &, QObject *, QDeclarativeContextData *);
   static QDeclarativeProperty restore(const QDeclarativePropertyCache::Data &,
                                       const QDeclarativePropertyCache::ValueTypeData &,
                                       QObject *,
                                       QDeclarativeContextData *);

   static bool equal(const QMetaObject *, const QMetaObject *);
   static bool canConvert(const QMetaObject *from, const QMetaObject *to);

   // "Public" (to QML) methods
   static QDeclarativeAbstractBinding *binding(const QDeclarativeProperty &that);
   static QDeclarativeAbstractBinding *setBinding(const QDeclarativeProperty &that,
         QDeclarativeAbstractBinding *,
         WriteFlags flags = DontRemoveBinding);
   static QDeclarativeExpression *signalExpression(const QDeclarativeProperty &that);
   static QDeclarativeExpression *setSignalExpression(const QDeclarativeProperty &that,
         QDeclarativeExpression *) ;
   static bool write(const QDeclarativeProperty &that, const QVariant &, WriteFlags);
   static int valueTypeCoreIndex(const QDeclarativeProperty &that);
   static int bindingIndex(const QDeclarativeProperty &that);
   static QMetaMethod findSignalByName(const QMetaObject *mo, const QByteArray &);
   static bool connect(QObject *sender, int signal_index,
                       const QObject *receiver, int method_index,
                       int type = 0, int *types = 0);
   static const QMetaObject *metaObjectForProperty(const QMetaObject *, int);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativePropertyPrivate::WriteFlags)

QT_END_NAMESPACE

#endif // QDECLARATIVEPROPERTY_P_H
