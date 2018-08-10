/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef CSMETA_H
#define CSMETA_H

#include <csmeta_internal_1.h>

#include <qlist.h>
#include <qmap.h>
#include <qstring8.h>
#include <qvariant.h>
#include <qstringfwd.h>

#include <utility>
#include <tuple>

class QMetaObject;

class Q_CORE_EXPORT QMetaClassInfo
{
 public:
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
   QMetaEnum(const QString &name, const QString &scope, bool isFlag);
   QMetaEnum();

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
   bool     m_flag;

   QMap<QString, int> m_data;
};

class Q_CORE_EXPORT QMetaMethod
{
 public:
   enum Access { Private, Protected, Public };
   enum MethodType { Method, Signal, Slot, Constructor };
   enum Attributes { Compatibility = 0x1, Cloned = 0x2, Scriptable = 0x4 };   // internal

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

   const QString name() const;

   int methodIndex() const;
   MethodType methodType() const;

   QList<QString> parameterNames() const;
   QList<QString> parameterTypes() const;

   int parameterCount() const;
   int parameterType(int index) const;

   const QString &methodSignature() const;
   const QString &typeName() const;

   int revision() const;
   void setRevision(int revision);

   void setBentoBox(const CSBentoAbstract *method);

   void setTag(const QString &data);
   const QString &tag() const;

   template<class R, class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template<class R, class ...Ts>
   bool invoke(QObject *object, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template<class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, Ts &&... Vs) const;

   template<class ...Ts>
   bool invoke(QObject *object, Ts &&... Vs) const;

 private:
   int m_revision;
   QString m_typeName;
   QString m_signature;

   QList<QString> m_paramNames;

   MethodType   m_methodType;
   Attributes   m_attributes;
   QMetaObject *m_metaObject;
   Access       m_access;

   QString m_tag;
   const CSBentoAbstract *m_bento;

   friend bool operator==(const QMetaMethod &m1, const QMetaMethod &m2);
   friend bool operator!=(const QMetaMethod &m1, const QMetaMethod &m2);
};

Q_DECLARE_TYPEINFO(QMetaMethod, Q_MOVABLE_TYPE);

inline bool operator==(const QMetaMethod &m1, const QMetaMethod &m2)
{
   if (m1.m_metaObject != m2.m_metaObject)  {
      return false;
   }

   if (m1.m_signature != m2.m_signature) {
      return false;
   }

   return true;
}

inline bool operator!=(const QMetaMethod &m1, const QMetaMethod &m2)
{
   return !(m1 == m2);
}

class Q_CORE_EXPORT QMetaProperty
{
 public:
   enum Kind { READ, WRITE, RESET, NOTIFY, REVISION, DESIGNABLE, SCRIPTABLE,
               STORED, USER, CONSTANT, FINAL
             };

   QMetaProperty(const QString &name = QString(), QMetaObject *obj = nullptr);

   bool isReadable() const;
   bool isWritable() const;
   bool isResettable() const;
   bool isDesignable(const QObject *obj = nullptr) const;
   bool isScriptable(const QObject *obj = nullptr) const;
   bool isStored(const QObject *obj = nullptr) const;
   bool isUser(const QObject *obj = nullptr) const;
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

   template<class T>
   T read(const QObject *obj) const;

   bool reset(QObject *obj) const;
   int revision() const;

   void setConstant();
   void setFinal();
   void setRevision(int value);
   void setTypeName(const QString &typeName);

   const QString &typeName() const;
   QVariant::Type type() const;
   int userType() const;
   bool write(QObject *obj, const QVariant &value) const;

   // properties
   void setReadMethod(const QString &typeName, JarReadAbstract *jarRead);
   void setWriteMethod(JarWriteAbstract *method);

   template<class T>
   void setNotifyMethod(T method);

   template<class T>
   void setResetMethod(void (T::*method)());

   void setDesignable(JarReadAbstract *method);
   void setScriptable(JarReadAbstract *method);
   void setStored(JarReadAbstract *method);
   void setUser(JarReadAbstract *method);

 private:
   QMetaObject *m_metaObject;
   QString m_name;
   QString m_typeName;

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

// QMetaMethod::invoke moved to csobject_internal.h becasue Invoke calls methods in QObject
// template<class ...Ts>
// bool QMetaMethod::invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts&&...Vs)

template<class R, class ...Ts>
bool QMetaMethod::invoke(QObject *object, CSReturnArgument<R> retval, Ts &&...Vs) const
{
   // calls first overload
   return this->invoke(object, Qt::AutoConnection, retval, std::forward<Ts>(Vs)...);
}

template<class ...Ts>
bool QMetaMethod::invoke(QObject *object, Ts &&...Vs) const
{
   // calls third overload, no return value from the invoked method
   return this->invoke(object, Qt::AutoConnection, std::forward<Ts>(Vs)...);
}

// **
// cs_typeName_internal is a templated class
template <class T, class = void>
class Q_CORE_EXPORT cs_typeName_internal
{
   public:
      static const QString &typeName();
};


// three macros

// cs_typeName_internal<dataType,void>::typeName is a method belonging to a specialization of a templated class
#define CS_REGISTER_CLASS(dataType) \
   class dataType; \
   Q_CORE_EXPORT const QString &cs_typeName_internal<dataType, void>::typeName() \
   { \
      static QString retval(#dataType); \
      return retval; \
   }

// cs_typeName_internal<dataType> is specialization of a templated class
#define CS_DECLARE_CLASS(dataType) \
   class dataType; \
   template <>  \
   class Q_CORE_EXPORT cs_typeName_internal<dataType,void>  \
   { \
      public: \
         static const QString &typeName(); \
   };


// cs_typeName_internal<dataType,void>::typeName is a method belonging to a specialization of a templated class
#define CS_REGISTER_TYPEDEF(dataType) \
   Q_CORE_EXPORT const QString &cs_typeName_internal<dataType, void>::typeName() \
   { \
      static QString retval(#dataType); \
      return retval; \
   }

// cs_typeName_internal<dataType> is specialization of a templated class
#define CS_DECLARE_TYPEDEF(dataType) \
   template <>  \
   class Q_CORE_EXPORT cs_typeName_internal<dataType,void>  \
   { \
      public: \
         static const QString &typeName(); \
   };


#define CS_REGISTER_TYPE(dataType) \
   template <>  \
   Q_CORE_EXPORT const QString &cs_typeName_internal<dataType,void>::typeName() \
   { \
      static QString retval(#dataType); \
      return retval; \
   }


#define CS_REGISTER_TEMPLATE(dataType) \
   template<class... Ts> \
   class cs_typeName_internal< dataType<Ts...> >  \
   { \
      public:  \
         static const QString &typeName();  \
   };  \
   template<class... Ts> \
   const QString &cs_typeName_internal< dataType<Ts...> >::typeName() \
   { \
      static QString retval(QString(#dataType) + "<" + cs_typeName<Ts...>() + ">");  \
      return retval; \
   }

// methods for these 2 class, located in csmeta_internal2.h around line 117
template<class E>
class cs_typeName_internal<E, typename std::enable_if<std::is_enum<E>::value>::type>
{
 public:
   static const QString &typeName();
};

template<class E>
class cs_typeName_internal< QFlags<E> >
{
 public:
   static const QString &typeName();
};


// QObject and children
template<class T>
class cs_typeName_internal<T, typename std::enable_if< std::is_base_of< QMetaObject,
   typename std::remove_reference< decltype(T::staticMetaObject() )>::type >::value>::type >
{
 public:
   static const QString &typeName();
};

template<class T>
const QString &cs_typeName_internal<T, typename std::enable_if< std::is_base_of< QMetaObject ,
       typename std::remove_reference< decltype(T::staticMetaObject() )>::type >::value>::type >::typeName()
{
   return T::staticMetaObject().className();
}


// 1   standard template functions
class cs_internalEmpty;

template<class T1 = cs_internalEmpty>
const QString &cs_typeName()
{
   if (std::is_same<T1, cs_internalEmpty>::value) {
      static QString retval("");
      return retval;

   } else {
      return cs_typeName_internal<T1>::typeName();
   }
}

template<class T1, class T2, class ...Ts>
const QString &cs_typeName()
{
   static QString tmp = cs_typeName_internal<T1>::typeName() + "," + cs_typeName<T2, Ts...>();
   return tmp;
}


// 2   specialization for pointers  (template classes)
template<class T>
class cs_typeName_internal<T *>
{
 public:
   static const QString &typeName();
};

template<class T>
const QString &cs_typeName_internal<T *>::typeName()
{
   static QString tmp = cs_typeName<T>() + "*";
   return tmp;
}


// 3   specialization for const pointers  (template classes)
template<class T>
class cs_typeName_internal<const T *>
{
 public:
   static const QString &typeName();
};

template<class T>
const QString &cs_typeName_internal<const T *>::typeName()
{
   static QString tmp = "const " + cs_typeName<T>() + "*";
   return tmp;
}


// 4   specialization for references  (template classes)
template<class T>
class cs_typeName_internal<T &>
{
 public:
   static const QString &typeName();
   \
};

template<class T>
const QString &cs_typeName_internal<T &>::typeName()
{
   static QString tmp = cs_typeName<T>() + "&";
   return tmp;
}


// 5   specialization for const references  (template classes)
template<class T>
class cs_typeName_internal<const T &>
{
 public:
   static const QString &typeName();
   \
};

template<class T>
const QString &cs_typeName_internal<const T &>::typeName()
{
   static QString tmp = "const " + cs_typeName<T>() + "&";
   return tmp;
}

template<class T1>
class QDeclarativeListProperty;


// declare here, register in csObject_private.cpp
CS_DECLARE_CLASS(QAbstractState)
CS_DECLARE_CLASS(QChar32)
CS_DECLARE_CLASS(QColor)
CS_DECLARE_CLASS(QCursor)
CS_DECLARE_CLASS(QBitmap)
CS_DECLARE_CLASS(QBrush)
CS_DECLARE_CLASS(QBitArray)
CS_DECLARE_CLASS(QByteArray)
CS_DECLARE_CLASS(QDate)
CS_DECLARE_CLASS(QDateTime)
CS_DECLARE_CLASS(QEasingCurve)
CS_DECLARE_CLASS(QFont)
CS_DECLARE_CLASS(QGraphicsEffect)
CS_DECLARE_CLASS(QGraphicsLayout)
CS_DECLARE_CLASS(QHostAddress)
CS_DECLARE_CLASS(QIcon)
CS_DECLARE_CLASS(QImage)
CS_DECLARE_CLASS(QJsonValue)
CS_DECLARE_CLASS(QJsonObject)
CS_DECLARE_CLASS(QJsonArray)
CS_DECLARE_CLASS(QJsonDocument)
CS_DECLARE_CLASS(QKeySequence)
CS_DECLARE_CLASS(QLine)
CS_DECLARE_CLASS(QLineF)
CS_DECLARE_CLASS(QLocale)
CS_DECLARE_CLASS(QMatrix)
CS_DECLARE_CLASS(QMatrix4x4)
CS_DECLARE_CLASS(QModelIndex)
CS_DECLARE_CLASS(QPalette)
CS_DECLARE_CLASS(QPen)
CS_DECLARE_CLASS(QPoint)
CS_DECLARE_CLASS(QPointF)
CS_DECLARE_CLASS(QPolygon)
CS_DECLARE_CLASS(QPolygonF)
CS_DECLARE_CLASS(QPixmap)
CS_DECLARE_CLASS(QRect)
CS_DECLARE_CLASS(QRectF)
CS_DECLARE_CLASS(QRegion)
CS_DECLARE_CLASS(QSize)
CS_DECLARE_CLASS(QSizeF)
CS_DECLARE_CLASS(QSizePolicy)
CS_DECLARE_CLASS(QState)
CS_DECLARE_CLASS(QString8)
CS_DECLARE_CLASS(QString16)
CS_DECLARE_CLASS(QStringList)
CS_DECLARE_CLASS(QStyleOption)
CS_DECLARE_CLASS(QStyleOptionViewItem)
CS_DECLARE_CLASS(QTextCursor)
CS_DECLARE_CLASS(QTextFormat)
CS_DECLARE_CLASS(QTextLength)
CS_DECLARE_CLASS(QTextOption)
CS_DECLARE_CLASS(QTime)
CS_DECLARE_CLASS(QTransform)
CS_DECLARE_CLASS(QQuaternion)
CS_DECLARE_CLASS(QUrl)
CS_DECLARE_CLASS(QUuid)
CS_DECLARE_CLASS(QVariant)
CS_DECLARE_CLASS(QVector2D)
CS_DECLARE_CLASS(QVector3D)
CS_DECLARE_CLASS(QVector4D)

CS_DECLARE_TYPEDEF(QRegularExpression8)
CS_DECLARE_TYPEDEF(QRegularExpression16)
CS_DECLARE_TYPEDEF(QStringView8)
CS_DECLARE_TYPEDEF(QStringView16)

//
CS_REGISTER_TEMPLATE(QFlatMap)
CS_REGISTER_TEMPLATE(QHash)
CS_REGISTER_TEMPLATE(QMultiHash)
CS_REGISTER_TEMPLATE(QLinkedList)
CS_REGISTER_TEMPLATE(QList)
CS_REGISTER_TEMPLATE(QMap)
CS_REGISTER_TEMPLATE(QMultiMap)
CS_REGISTER_TEMPLATE(QQueue)
CS_REGISTER_TEMPLATE(QSet)
CS_REGISTER_TEMPLATE(QStack)
CS_REGISTER_TEMPLATE(QVector)
CS_REGISTER_TEMPLATE(QDeclarativeListProperty)
CS_REGISTER_TEMPLATE(qMapCompare)
CS_REGISTER_TEMPLATE(std::tuple)
CS_REGISTER_TEMPLATE(std::pair)

// **
template<class T>
T QMetaProperty::read(const QObject *object) const
{
   if (! object || ! m_readJar) {
      throw std::logic_error("QMetaProperty::read() Object was null");
   }

   return m_readJar->run<T>(object);
}

template<class T>
void QMetaProperty::setNotifyMethod(T method)
{
   if (! method)  {
      return;
   }

   CSBento<T> *temp = new CSBento<T>(method);
   m_notifyBento  = temp;

   m_notify_able = true;
}

template<class T>
void QMetaProperty::setResetMethod( void (T::*method)() )
{
   if (! method)  {
      return;
   }

   m_resetJar   = new SpiceJarReset<T>(method);
   m_reset_able = true;
}

#endif
