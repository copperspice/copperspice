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

#ifndef CSMETA_H
#define CSMETA_H

#include <csmetafwd.h>
#include <csmeta_internal_1.h>

#include <qlist.h>
#include <qmap.h>
#include <qstring8.h>
#include <qstringfwd.h>
#include <qsharedpointer.h>
#include <qvariant.h>

#include <utility>
#include <tuple>

class QMetaObject;

class Q_CORE_EXPORT QMetaClassInfo
{
 public:
   QMetaClassInfo() = default;
   QMetaClassInfo(const QString &name, const QString &value);

   const QString &name() const;
   const QString &value() const;

 private:
   QString m_name;
   QString m_value;
};

class Q_CORE_EXPORT QMetaEnum
{
 public:
   QMetaEnum();
   QMetaEnum(const QString &name, const QString &scope, bool isFlag);

   bool isFlag() const;
   bool isValid() const;

   const QString &key(int index) const;
   int keyCount() const;
   int keyToValue(const QString &key) const;
   int keysToValue(const QString &keys) const;

   const QString &name() const;
   const QString &scope() const;

   void setData(QMap<QString, int> valueMap);

   int value(int index) const;
   const QString &valueToKey(int value) const;
   QString valueToKeys(int value) const;

 private:
   QString m_name;
   QString m_scope;
   bool m_flag;

   QMap<QString, int> m_data;
};

class Q_CORE_EXPORT QMetaMethod
{
 public:
   enum Access {
      Private,
      Protected,
      Public
   };

   enum MethodType {
      Method,
      Signal,
      Slot,
      Constructor
   };

   // might be internal
   enum Attributes {
      Compatibility = 0x1,
      Cloned        = 0x2,
      Scriptable    = 0x4
   };

   QMetaMethod(const QString &typeName, const QString &signature, std::vector<QString> paramNames,
         Access access, MethodType methodType, Attributes attributes, QMetaObject *obj);

   QMetaMethod();

   Access access() const;
   Attributes attributes() const;

   bool compare(const CsSignal::Internal::BentoAbstract &method) const;

   template <typename SignalClass, typename ...SignalArgs>
   static QMetaMethod fromSignal(void (SignalClass::* signalMethod)(SignalArgs ...) );

   const CSBentoAbstract *getBentoBox() const;
   const QMetaObject *getMetaObject() const;

   bool isValid() const;

   const QString name() const;

   int methodIndex() const;
   MethodType methodType() const;

   QList<QString> parameterNames() const;
   QList<QString> parameterTypes() const;

   int parameterCount() const;
   uint parameterType(int index) const;

   const QString &methodSignature() const;
   const QString &typeName() const;

   int revision() const;
   void setRevision(int revision);

   void setBentoBox(const CSBentoAbstract *method);

   void setTag(const QString &data);
   const QString &tag() const;

   template <class R, class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template <class R, class ...Ts>
   bool invoke(QObject *object, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template <class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, Ts &&... Vs) const;

   template <class ...Ts>
   bool invoke(QObject *object, Ts &&... Vs) const;

 private:
   int m_revision;
   QString m_typeName;
   QString m_signature;

   QList<QString> m_paramNames;

   Access       m_access;
   MethodType   m_methodType;
   Attributes   m_attributes;
   QMetaObject *m_metaObject;

   QString m_tag;
   const CSBentoAbstract *m_bento;

   friend bool operator==(const QMetaMethod &method1, const QMetaMethod &method2);
   friend bool operator!=(const QMetaMethod &method1, const QMetaMethod &method2);
};

inline bool operator==(const QMetaMethod &method1, const QMetaMethod &method2)
{
   if (method1.m_metaObject != method2.m_metaObject)  {
      return false;
   }

   if (method1.m_signature != method2.m_signature) {
      return false;
   }

   return true;
}

inline bool operator!=(const QMetaMethod &method1, const QMetaMethod &method2)
{
   return !(method1 == method2);
}

class Q_CORE_EXPORT QMetaProperty
{
 public:
   enum Kind {
      READ,
      WRITE,
      RESET,
      NOTIFY,
      REVISION,
      DESIGNABLE,
      SCRIPTABLE,
      STORED,
      USER,
      CONSTANT,
      FINAL
   };

   QMetaProperty(const QString &name = QString(), QMetaObject *object = nullptr);

   bool isReadable() const;
   bool isWritable() const;
   bool isResettable() const;
   bool isDesignable(const QObject *object = nullptr) const;
   bool isScriptable(const QObject *object = nullptr) const;
   bool isStored(const QObject *object = nullptr) const;
   bool isUser(const QObject *object = nullptr) const;
   bool isConstant() const;
   bool isFinal() const;
   bool isValid() const;
   bool isFlagType() const;
   bool isEnumType() const;

   QMetaEnum enumerator() const;

   bool hasNotifySignal() const;
   bool hasStdCppSet() const;

   const QString &name() const;
   QMetaMethod notifySignal() const;
   int notifySignalIndex() const;
   int propertyIndex() const;

   QVariant read(const QObject *obj) const;

   // Note: Doxypress docs must be located here due to an overload with similar signatures

   //! Reads the property's value from the given \a obj. Returns the property value if
   //! valid, otherwise returns a default constructed T.
   template <class T>
   T read(const QObject *obj) const;

   bool reset(QObject *obj) const;
   int revision() const;

   void setConstant();
   void setFinal();
   void setRevision(int value);
   void setTypeName(const QString &typeName);

   const QString &typeName() const;
   QVariant::Type type() const;
   uint userType() const;
   bool write(QObject *object, const QVariant &value) const;

   // properties
   void setReadMethod(std::type_index returnTypeId, QString (*returnTypeFuncPtr)(), JarReadAbstract *jarRead);
   void setWriteMethod(JarWriteAbstract *method, const QString &methodName);

   template <class T>
   void setNotifyMethod(T method);

   template <class T>
   void setResetMethod(void (T::*method)());

   void setDesignable(JarReadAbstract *method);
   void setScriptable(JarReadAbstract *method);
   void setStored(JarReadAbstract *method);
   void setUser(JarReadAbstract *method);

 private:
   void loadTypeName() const;

   QMetaObject *m_metaObject;

   QString m_name;
   QString m_typeName;
   QString m_writeMethodName;

   std::type_index m_returnTypeId;
   QString (*m_returnTypeFuncPtr)();

   bool m_read_able;
   bool m_write_able;
   bool m_reset_able;
   bool m_notify_able;

   int  m_revision;
   bool m_constant;
   bool m_final;

   //
   JarReadAbstract *m_readJar;
   JarReadAbstract *m_designJar;
   JarReadAbstract *m_scriptJar;
   JarReadAbstract *m_storedJar;
   JarReadAbstract *m_userJar;

   JarWriteAbstract *m_writeJar;
   JarResetAbstract *m_resetJar;

   CSBentoAbstract *m_notifyBento;
};

// **

template <typename SignalClass, typename ...SignalArgs>
QMetaMethod QMetaMethod::fromSignal(void (SignalClass::* signalMethod)(SignalArgs ...) )
{
   const auto &metaObj = SignalClass::staticMetaObject();

   CSBento<void(SignalClass::*)(SignalArgs ...)> tmp = CSBento<void(SignalClass::*)(SignalArgs ...)>(signalMethod);
   QMetaMethod testMethod = metaObj.lookUpMethod(tmp);

   if (testMethod.methodType() == QMetaMethod::Signal) {
      return testMethod;
   }

   return QMetaMethod();
}

// QMetaMethod::invoke moved to csobject_internal.h becasue invoke() calls methods in QObject
// template <class ...Ts>
// bool QMetaMethod::invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts&&...Vs)

template <class R, class ...Ts>
bool QMetaMethod::invoke(QObject *object, CSReturnArgument<R> retval, Ts &&...Vs) const
{
   // calls first overload
   return this->invoke(object, Qt::AutoConnection, retval, std::forward<Ts>(Vs)...);
}

template <class ...Ts>
bool QMetaMethod::invoke(QObject *object, Ts &&...Vs) const
{
   // calls third overload, no return value from the invoked method
   return this->invoke(object, Qt::AutoConnection, std::forward<Ts>(Vs)...);
}

// **
template <class T, class = void>
class CS_ReturnType
{
 public:
   static const QString &getName() {
      static_assert(! std::is_same_v<T, T>, "Requested type name has not been registered.");
      static const QString retval;
      return retval;
   }
};

#if ! defined (CS_DOXYPRESS)

#define CS_REGISTER_CLASS(dataType) \
   class dataType;                  \
   CS_REGISTER_TYPE(dataType)

// specialization of a templated function
#define CS_REGISTER_TYPE(dataType)                       \
   template <>                                           \
   inline const QString &cs_typeToName<dataType>()       \
   {                                                     \
      static const QString retval(#dataType);            \
      return retval;                                     \
   }

// specialization of a templated class
#define CS_REGISTER_TEMPLATE(dataType)                    \
   template <class... Ts>                                 \
   class CS_ReturnType<dataType<Ts...>>                   \
   {                                                      \
    public:                                               \
      static const QString &getName();                    \
   };                                                     \
   template <class... Ts>                                 \
   const QString &CS_ReturnType< dataType<Ts...> >::getName()                               \
   {                                                                                        \
      static const QString retval(QString(#dataType) + "<" + cs_typeToName<Ts...>() + ">"); \
      return retval;                                                                        \
   }

#endif   // doxypress

// methods for these 2 class, located in csmeta_internal2.h around line 113
template <class E>
class CS_ReturnType<E, typename std::enable_if<std::is_enum_v<E>>::type>
{
 public:
   static const QString &getName();
};

template <class E>
class CS_ReturnType<QFlags<E>>
{
 public:
   static const QString &getName();
};

// QObject and children
template <class T>
class CS_ReturnType<T, typename std::enable_if< std::is_base_of< QMetaObject,
      typename std::remove_reference< decltype(T::staticMetaObject() )>::type >::value>::type >
{
 public:
   static const QString &getName();
};

template <class T>
const QString &CS_ReturnType<T, typename std::enable_if< std::is_base_of< QMetaObject,
      typename std::remove_reference<decltype(T::staticMetaObject() )>::type>::value>::type>::getName()
{
   return T::staticMetaObject().className();
}

// standard template function   ( class T1 = cs_internalEmpty, default value located in csmetafwd.h )
template <class T1>
const QString &cs_typeToName()
{
   if constexpr (std::is_same_v<T1, cs_internalEmpty>) {
      static const QString retval("");
      return retval;

   } else if constexpr (std::is_const_v<std::remove_pointer_t<T1>> && std::is_pointer_v<T1>) {
      static const QString retval = "const " + cs_typeToName<std::remove_const_t<std::remove_pointer_t<T1>>>() + "*";
      return retval;

   } else if constexpr (std::is_pointer_v<T1>) {
      static const QString retval = cs_typeToName<std::remove_pointer_t<T1>>() + "*";
      return retval;

   } else if constexpr (std::is_const_v<std::remove_reference_t<T1>> && std::is_lvalue_reference_v<T1>) {
      static const QString retval = "const " + cs_typeToName<std::remove_const_t<std::remove_reference_t<T1>>>() + "&";
      return retval;

   } else if constexpr (std::is_lvalue_reference_v<T1>) {
      static const QString retval = cs_typeToName<std::remove_reference_t<T1>>() + "&";
      return retval;

   } else if constexpr (std::is_rvalue_reference_v<T1>) {
      static const QString retval = cs_typeToName<std::remove_reference_t<T1>>() + "&&";
      return retval;

   } else if constexpr (std::is_base_of_v<QObject, T1>) {
      // T1 inherits from QObject
      return T1::staticMetaObject().className();

   } else {
      // uses the class which was set up in the cs_register_template macro
      return CS_ReturnType<T1>::getName();
   }
}

template <class T>
const QString &cs_typeToName_fold()
{
   static const QString retval = "," + cs_typeToName<T>();

   return retval;
}

template <class T1, class T2, class ...Ts>
const QString &cs_typeToName()
{
   // fold expression
   static const QString retval = ((cs_typeToName<T1>() + "," + cs_typeToName<T2>()) + ... + cs_typeToName_fold<Ts>());

   return retval;
}

// register names of all types which are used in QVariant

// primitive
CS_REGISTER_TYPE(bool)
CS_REGISTER_TYPE(short)
CS_REGISTER_TYPE(unsigned short)
CS_REGISTER_TYPE(int)
CS_REGISTER_TYPE(unsigned int)
CS_REGISTER_TYPE(long)
CS_REGISTER_TYPE(unsigned long)
CS_REGISTER_TYPE(long long)
CS_REGISTER_TYPE(unsigned long long)
CS_REGISTER_TYPE(double)
CS_REGISTER_TYPE(long double)
CS_REGISTER_TYPE(float)

CS_REGISTER_TYPE(char)
CS_REGISTER_TYPE(signed char)
CS_REGISTER_TYPE(unsigned char)
//  CS_REGISTER_TYPE(char8_t)          // add with C++20
CS_REGISTER_TYPE(char16_t)
CS_REGISTER_TYPE(char32_t)
CS_REGISTER_TYPE(void)

CS_REGISTER_CLASS(QBitArray)
CS_REGISTER_CLASS(QByteArray)
CS_REGISTER_CLASS(QChar32)
CS_REGISTER_CLASS(QString8)
CS_REGISTER_CLASS(QString16)
CS_REGISTER_CLASS(QStringList)

CS_REGISTER_CLASS(QDate)
CS_REGISTER_CLASS(QDateTime)
CS_REGISTER_CLASS(QTime)
CS_REGISTER_CLASS(QTimeZone)
CS_REGISTER_CLASS(QLocale)

CS_REGISTER_CLASS(QJsonValue)
CS_REGISTER_CLASS(QJsonObject)
CS_REGISTER_CLASS(QJsonArray)
CS_REGISTER_CLASS(QJsonDocument)

CS_REGISTER_CLASS(QLine)
CS_REGISTER_CLASS(QLineF)
CS_REGISTER_CLASS(QPoint)
CS_REGISTER_CLASS(QPointF)
CS_REGISTER_CLASS(QPolygon)
CS_REGISTER_CLASS(QPolygonF)
CS_REGISTER_CLASS(QRect)
CS_REGISTER_CLASS(QRectF)
CS_REGISTER_CLASS(QSize)
CS_REGISTER_CLASS(QSizeF)

// core
CS_REGISTER_CLASS(QEasingCurve)
CS_REGISTER_CLASS(QMargins)
CS_REGISTER_CLASS(QModelIndex)
CS_REGISTER_CLASS(QPersistentModelIndex)
CS_REGISTER_CLASS(QUrl)
CS_REGISTER_CLASS(QUuid)
CS_REGISTER_CLASS(QVariant)

// gui
CS_REGISTER_CLASS(QBitmap)
CS_REGISTER_CLASS(QBrush)
CS_REGISTER_CLASS(QColor)
CS_REGISTER_CLASS(QCursor)
CS_REGISTER_CLASS(QFont)
CS_REGISTER_CLASS(QIcon)
CS_REGISTER_CLASS(QImage)
CS_REGISTER_CLASS(QKeySequence)
CS_REGISTER_CLASS(QMatrix)
CS_REGISTER_CLASS(QMatrix4x4)
CS_REGISTER_CLASS(QPalette)
CS_REGISTER_CLASS(QPen)
CS_REGISTER_CLASS(QPixmap)
CS_REGISTER_CLASS(QQuaternion)
CS_REGISTER_CLASS(QRegion)
CS_REGISTER_CLASS(QSizePolicy)
CS_REGISTER_CLASS(QTextLength)
CS_REGISTER_CLASS(QTextFormat)
CS_REGISTER_CLASS(QTransform)
CS_REGISTER_CLASS(QVector2D)
CS_REGISTER_CLASS(QVector3D)
CS_REGISTER_CLASS(QVector4D)

CS_REGISTER_TYPE(QRegularExpression8)
CS_REGISTER_TYPE(QRegularExpression16)
CS_REGISTER_TYPE(QStringView8)
CS_REGISTER_TYPE(QStringView16)

CS_REGISTER_TEMPLATE(QHash)
CS_REGISTER_TEMPLATE(QList)
CS_REGISTER_TEMPLATE(QMap)
CS_REGISTER_TEMPLATE(QMultiHash)
CS_REGISTER_TEMPLATE(QMultiMap)

// added for invokable or property return types
CS_REGISTER_CLASS(QTimerInfo)
CS_REGISTER_TEMPLATE(QSharedPointer)
CS_REGISTER_TEMPLATE(QWeakPointer)
CS_REGISTER_TEMPLATE(std::pair)

#if ! defined (CS_DOXYPRESS)

// next 8 function are specializations for containers to omit the Compare template when it is not specified
template <class Key, class Value>
class CS_ReturnType<QMap<Key, Value, qMapCompare<Key>>>
{
 public:
   static const QString &getName() {
      static QString retval("QMap<" + cs_typeToName<Key>() + "," + cs_typeToName<Value>() + ">");
      return retval;
   }
};

template <class Key, class Value>
class CS_ReturnType<QMultiMap<Key, Value, qMapCompare<Key>>>
{
 public:
   static const QString &getName() {
      static const QString retval("QMultiMap<" + cs_typeToName<Key>() + "," + cs_typeToName<Value>() + ">");
      return retval;
   }
};

template <class Key, class Value>
class CS_ReturnType<QHash<Key, Value, qHashFunc<Key>, qHashEqual<Key>>>
{
 public:
   static const QString &getName() {
      static const QString retval("QHash<" + cs_typeToName<Key>() + "," + cs_typeToName<Value>() + ">");
      return retval;
   }
};

template <class Key, class Value>
class CS_ReturnType<QMultiHash<Key, Value, qHashFunc<Key>, qHashEqual<Key>>>
{
 public:
   static const QString &getName() {
      static const QString retval("QMultiHash<" + cs_typeToName<Key>() + "," + cs_typeToName<Value>() + ">");
      return retval;
   }
};

#endif   // doxypress

// **
template <class T>
T QMetaProperty::read(const QObject *object) const
{
   if (! object || ! m_readJar) {
      throw std::logic_error("QMetaProperty::read() Object was null");
   }

   return m_readJar->run<T>(object);
}

template <class T>
void QMetaProperty::setNotifyMethod(T method)
{
   if (! method)  {
      return;
   }

   CSBento<T> *temp = new CSBento<T>(method);
   m_notifyBento  = temp;

   m_notify_able = true;
}

template <class T>
void QMetaProperty::setResetMethod( void (T::*method)() )
{
   if (! method)  {
      return;
   }

   m_resetJar   = new SpiceJarReset<T>(method);
   m_reset_able = true;
}

#endif
