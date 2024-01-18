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

#ifndef QDECLARATIVE_H
#define QDECLARATIVE_H

#include <QtDeclarative/qdeclarativeprivate.h>
#include <QtDeclarative/qdeclarativeparserstatus.h>
#include <QtDeclarative/qdeclarativepropertyvaluesource.h>
#include <QtDeclarative/qdeclarativepropertyvalueinterceptor.h>
#include <QtDeclarative/qdeclarativelist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetaobject.h>

#define QML_DECLARE_TYPE(TYPE) \
    Q_DECLARE_METATYPE(TYPE *) \
    Q_DECLARE_METATYPE(QDeclarativeListProperty<TYPE>)

#define QML_DECLARE_TYPE_HASMETATYPE(TYPE) \
    Q_DECLARE_METATYPE(QDeclarativeListProperty<TYPE>)

#define QML_DECLARE_INTERFACE(INTERFACE) \
    QML_DECLARE_TYPE(INTERFACE)

#define QML_DECLARE_INTERFACE_HASMETATYPE(INTERFACE) \
    QML_DECLARE_TYPE_HASMETATYPE(INTERFACE)

enum { /* TYPEINFO flags */
   QML_HAS_ATTACHED_PROPERTIES = 0x01
};

#define QML_DECLARE_TYPEINFO(TYPE, FLAGS) \
QT_BEGIN_NAMESPACE \
template <> \
class QDeclarativeTypeInfo<TYPE > \
{ \
public: \
    enum { \
        hasAttachedProperties = (((FLAGS) & QML_HAS_ATTACHED_PROPERTIES) == QML_HAS_ATTACHED_PROPERTIES) \
    }; \
}; \
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE

template<typename T>
int qmlRegisterType()
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      0, 0,
      QString(),

      0, 0, 0, 0, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      0,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

int qmlRegisterTypeNotAvailable(const char *uri, int versionMajor, int versionMinor, const char *qmlName,
                                const QString &message);

template<typename T>
int qmlRegisterUncreatableType(const char *uri, int versionMajor, int versionMinor, const char *qmlName,
                               const QString &reason)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      0, 0,
      reason,

      uri, versionMajor, versionMinor, qmlName, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      0,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

template<typename T>
int qmlRegisterType(const char *uri, int versionMajor, int versionMinor, const char *qmlName)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      sizeof(T), QDeclarativePrivate::createInto<T>,
      QString(),

      uri, versionMajor, versionMinor, qmlName, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      0,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

template<typename T, int metaObjectRevision>
int qmlRegisterType(const char *uri, int versionMajor, int versionMinor, const char *qmlName)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      1,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      sizeof(T), QDeclarativePrivate::createInto<T>,
      QString(),

      uri, versionMajor, versionMinor, qmlName, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      0,
      metaObjectRevision
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

template<typename T, int metaObjectRevision>
int qmlRegisterRevision(const char *uri, int versionMajor, int versionMinor)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      1,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      sizeof(T), QDeclarativePrivate::createInto<T>,
      QString(),

      uri, versionMajor, versionMinor, 0, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      0,
      metaObjectRevision
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}


template<typename T, typename E>
int qmlRegisterExtendedType()
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      0, 0,
      QString(),

      0, 0, 0, 0, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      QDeclarativePrivate::createParent<E>, &E::staticMetaObject,

      0,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

template<typename T, typename E>
int qmlRegisterExtendedType(const char *uri, int versionMajor, int versionMinor,
                            const char *qmlName)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativeAttachedPropertiesFunc attached = QDeclarativePrivate::attachedPropertiesFunc<E>();
   const QMetaObject *attachedMetaObject = QDeclarativePrivate::attachedPropertiesMetaObject<E>();
   if (!attached) {
      attached = QDeclarativePrivate::attachedPropertiesFunc<T>();
      attachedMetaObject = QDeclarativePrivate::attachedPropertiesMetaObject<T>();
   }

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      sizeof(T), QDeclarativePrivate::createInto<T>,
      QString(),

      uri, versionMajor, versionMinor, qmlName, &T::staticMetaObject,

      attached,
      attachedMetaObject,

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      QDeclarativePrivate::createParent<E>, &E::staticMetaObject,

      0,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

template<typename T>
int qmlRegisterInterface(const char *typeName)
{
   QByteArray name(typeName);

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterInterface qmlInterface = {
      0,
      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      qobject_interface_iid<T *>()
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::InterfaceRegistration, &qmlInterface);
}

template<typename T>
int qmlRegisterCustomType(const char *uri, int versionMajor, int versionMinor,
                          const char *qmlName, QDeclarativeCustomParser *parser)
{
   QByteArray name(T::staticMetaObject.className());

   QByteArray pointerName(name + '*');
   QByteArray listName("QDeclarativeListProperty<" + name + ">");

   QDeclarativePrivate::RegisterType type = {
      0,

      qRegisterMetaType<T *>(pointerName.constData()),
      qRegisterMetaType<QDeclarativeListProperty<T> >(listName.constData()),
      sizeof(T), QDeclarativePrivate::createInto<T>,
      QString(),

      uri, versionMajor, versionMinor, qmlName, &T::staticMetaObject,

      QDeclarativePrivate::attachedPropertiesFunc<T>(),
      QDeclarativePrivate::attachedPropertiesMetaObject<T>(),

      QDeclarativePrivate::StaticCastSelector<T, QDeclarativeParserStatus>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueSource>::cast(),
      QDeclarativePrivate::StaticCastSelector<T, QDeclarativePropertyValueInterceptor>::cast(),

      0, 0,

      parser,
      0
   };

   return QDeclarativePrivate::qmlregister(QDeclarativePrivate::TypeRegistration, &type);
}

class QDeclarativeContext;
class QDeclarativeEngine;
Q_DECLARATIVE_EXPORT void qmlExecuteDeferred(QObject *);
Q_DECLARATIVE_EXPORT QDeclarativeContext *qmlContext(const QObject *);
Q_DECLARATIVE_EXPORT QDeclarativeEngine *qmlEngine(const QObject *);
Q_DECLARATIVE_EXPORT QObject *qmlAttachedPropertiesObjectById(int, const QObject *, bool create = true);
Q_DECLARATIVE_EXPORT QObject *qmlAttachedPropertiesObject(int *, const QObject *, const QMetaObject *, bool create);

template<typename T>
QObject *qmlAttachedPropertiesObject(const QObject *obj, bool create = true)
{
   static int idx = -1;
   return qmlAttachedPropertiesObject(&idx, obj, &T::staticMetaObject, create);
}

QT_END_NAMESPACE

QML_DECLARE_TYPE(QObject)
Q_DECLARE_METATYPE(QVariant)

#endif // QDECLARATIVE_H
