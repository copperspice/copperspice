/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef CSMETA_H
#define CSMETA_H

#include "csmeta_internal_1.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QVariant>

#include <utility>
#include <tuple>

QT_BEGIN_NAMESPACE

class QMetaObject;

class Q_CORE_EXPORT QMetaClassInfo
{
 public:
   QMetaClassInfo(const char *name, const char *value);

   const char *name() const;
   const char *value() const;

 private:
   const char *m_name;
   const char *m_value;
};

Q_DECLARE_TYPEINFO(QMetaClassInfo, Q_MOVABLE_TYPE);


class Q_CORE_EXPORT QMetaEnum
{
 public:
   QMetaEnum(const char *name, const char *scope, bool isFlag);
   QMetaEnum();

   bool isFlag() const;
   bool isValid() const;

   const char *key(int index) const;
   int keyCount() const;
   int keyToValue(const char *key) const;
   int keysToValue(const char *keys) const;

   const char *name() const;
   const char *scope() const;

   void setData(QMap<QByteArray, int> valueMap);

   int value(int index) const;
   const char *valueToKey(int value) const;
   QByteArray valueToKeys(int value) const;

 private:
   const char *m_name;
   const char *m_scope;
   bool m_flag;

   QMap<QByteArray, int> m_data;
};

Q_DECLARE_TYPEINFO(QMetaEnum, Q_MOVABLE_TYPE);


class Q_CORE_EXPORT QMetaMethod
{
 public:
   enum Access { Private, Protected, Public };
   enum MethodType { Method, Signal, Slot, Constructor };
   enum Attributes { Compatibility = 0x1, Cloned = 0x2, Scriptable = 0x4 };   // internal

   QMetaMethod(const char *typeName, const QByteArray &signature, QList<QByteArray> paramNames,
               Access access, MethodType methodType, Attributes attributes, QMetaObject *obj);

   QMetaMethod();

   Access access() const;
   Attributes attributes() const;

   bool compare(const BentoAbstract &method) const;
   const BentoAbstract *getBentoBox() const;
   const QMetaObject *getMetaObject() const;

   QByteArray name() const;

   int methodIndex() const;
   MethodType methodType() const;

   QList<QByteArray> parameterNames() const;
   QList<QByteArray> parameterTypes() const;

   int parameterCount() const;
   int parameterType(int index) const;

   QByteArray methodSignature() const;
   const char *typeName() const;

   int revision() const;
   void setRevision(int revision);

   void setBentoBox(const BentoAbstract *method);
   void setTag(const char *data);
   const char *tag() const;

   template<class R, class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template<class R, class ...Ts>
   bool invoke(QObject *object, CSReturnArgument<R> retval, Ts &&... Vs) const;

   template<class ...Ts>
   bool invoke(QObject *object, Qt::ConnectionType type, Ts &&... Vs) const;

   template<class ...Ts>
   bool invoke(QObject *object, Ts &&... Vs) const;

 private:
   const char *m_typeName;
   QByteArray m_signature;
   QList<QByteArray> m_paramNames;
   Access m_access;
   MethodType m_methodType;
   Attributes m_attributes;
   QMetaObject *m_metaObject;
   const char *m_tag;
   const BentoAbstract *m_bento;
   int m_revision;

   friend bool operator==(const QMetaMethod &m1, const QMetaMethod &m2);
   friend bool operator!=(const QMetaMethod &m1, const QMetaMethod &m2);
};

Q_DECLARE_TYPEINFO(QMetaMethod, Q_MOVABLE_TYPE);

inline bool operator==(const QMetaMethod &m1, const QMetaMethod &m2)
{
   if (m1.m_metaObject != m2.m_metaObject)  {
      return false;
   }

   if ( strcmp(m1.m_signature, m2.m_signature) != 0) {
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

   QMetaProperty(const char *name = 0, QMetaObject *obj = 0 );

   bool isReadable() const;
   bool isWritable() const;
   bool isResettable() const;
   bool isDesignable(const QObject *obj = 0) const;
   bool isScriptable(const QObject *obj = 0) const;
   bool isStored(const QObject *obj = 0) const;
   bool isUser(const QObject *obj = 0) const;
   bool isConstant() const;
   bool isFinal() const;
   bool isValid() const;
   bool isFlagType() const;
   bool isEnumType() const;

   QMetaEnum enumerator() const;

   bool hasNotifySignal() const;
   bool hasStdCppSet() const;

   const char *name() const;
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
   void setTypeName(const char *typeName);

   const char *typeName() const;
   QVariant::Type type() const;
   int userType() const;
   bool write(QObject *obj, const QVariant &value) const;

   // properties
   void setReadMethod(const char *typeName, JarReadAbstract *jarRead);
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
   const char *m_name;
   QMetaObject *m_metaObject;
   const char *m_typeName;

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

   BentoAbstract *m_notifyBento;
};

// **

// QMetaMethod::invoke moved to csobject_internal.h becasue Invoke calls methods in QObject
// template<class ...Ts>
// bool QMetaMethod::invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts&&...Vs)

template<class R, class ...Ts>
bool QMetaMethod::invoke(QObject *object, CSReturnArgument<R> retval, Ts &&...Vs) const
{
   // calls first overload
   return this->invoke(object, Qt::AutoConnection, retval, std::forward(Vs)...);
}

template<class ...Ts>
bool QMetaMethod::invoke(QObject *object, Ts &&...Vs) const
{
   // calls third overload, no return value from the invoked method
   return this->invoke(object, Qt::AutoConnection, std::forward(Vs)...);
}

// **
template <class T, class = void>
class Q_CORE_EXPORT cs_typeName_internal
{
 public:
   static const char *typeName();
};


// three macros
#define CS_REGISTER_CLASS(dataType) \
   class dataType; \
   template <>  \
   Q_CORE_EXPORT const char * cs_typeName_internal<dataType, void>::typeName() \
   { \
      return #dataType; \
   } \
   template const char *cs_typeName_internal<dataType,void>::typeName();


#define CS_REGISTER_TYPE(dataType) \
   template <>  \
   Q_CORE_EXPORT const char * cs_typeName_internal<dataType,void>::typeName() \
   { \
      return #dataType; \
   } \
   template const char *cs_typeName_internal<dataType,void>::typeName();


#define CS_REGISTER_TEMPLATE(dataType) \
   template<class... Ts> \
   class cs_typeName_internal< dataType<Ts...> >  \
   { \
      public:  \
         static const char * typeName();  \
   };  \
   template<class... Ts> \
   const char * cs_typeName_internal< dataType<Ts...> >::typeName() \
   { \
      static QByteArray temp = QByteArray(#dataType) + "<" + cs_typeName<Ts...>() + ">";  \
      return temp.constData(); \
   }

// methods for these 2 class, located in csmeta_internal2.h around line 117
template<class E>
class cs_typeName_internal<E, typename std::enable_if<std::is_enum<E>::value>::type>
{
 public:
   static const char *typeName();
   \
};

template<class E>
class cs_typeName_internal< QFlags<E> >
{
 public:
   static const char *typeName();
   \
};


// QObject and children
template<class T>
class cs_typeName_internal<T, typename std::enable_if< std::is_base_of< QMetaObject,
   typename std::remove_reference< decltype(T::staticMetaObject() )>::type >::value>::type >
{
 public:
   static const char *typeName();
   \
};

template<class T>
inline const char *cs_typeName_internal<T, typename std::enable_if< std::is_base_of< QMetaObject ,
       typename std::remove_reference< decltype(T::staticMetaObject() )>::type >::value>::type >::typeName()
{
   return T::staticMetaObject().className();
}


// 1   standard template functions
class cs_internalEmpty;

template<class T1 = cs_internalEmpty>
const char *cs_typeName()
{
   if (std::is_same<T1, cs_internalEmpty>::value) {
      return "";

   } else {
      static QByteArray temp = QByteArray( cs_typeName_internal<T1>::typeName() );
      return temp.constData();
   }
}

template<class T1, class T2, class ...Ts>
const char *cs_typeName()
{
   static QByteArray temp = QByteArray( cs_typeName_internal<T1>::typeName() ) + "," + cs_typeName<T2, Ts...>();
   return temp.constData();
}


// 2   specialization for pointers  (template classes)
template<class T>
class cs_typeName_internal<T *>
{
 public:
   static const char *typeName();
   \
};

template<class T>
const char *cs_typeName_internal<T *>::typeName()
{
   static QByteArray temp = QByteArray( cs_typeName<T>() ) + "*";
   return temp.constData();
}


// 3   specialization for const pointers  (template classes)
template<class T>
class cs_typeName_internal<const T *>
{
 public:
   static const char *typeName();
   \
};

template<class T>
const char *cs_typeName_internal<const T *>::typeName()
{
   static QByteArray temp = "const " + QByteArray( cs_typeName<T>() ) + "*";
   return temp.constData();
}


// 4   specialization for references  (template classes)
template<class T>
class cs_typeName_internal<T &>
{
 public:
   static const char *typeName();
   \
};

template<class T>
const char *cs_typeName_internal<T &>::typeName()
{
   static QByteArray temp = QByteArray( cs_typeName<T>() ) + "&";
   return temp.constData();
}


// 5   specialization for const references  (template classes)
template<class T>
class cs_typeName_internal<const T &>
{
 public:
   static const char *typeName();
   \
};

template<class T>
const char *cs_typeName_internal<const T &>::typeName()
{
   static QByteArray temp = "const " + QByteArray( cs_typeName<T>() ) + "&";
   return temp.constData();
}

template<class T1>
class QDeclarativeListProperty;

template<class T1, class T2>
class QPair;

CS_REGISTER_TEMPLATE(QHash)
CS_REGISTER_TEMPLATE(QList)
CS_REGISTER_TEMPLATE(QMap)
CS_REGISTER_TEMPLATE(QPair)
CS_REGISTER_TEMPLATE(QDeclarativeListProperty)
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

   Bento<T> *temp = new Bento<T>(method);
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

QT_END_NAMESPACE

#endif
