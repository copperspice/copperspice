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

// do not move include, if qobject.h is included directly forward declarations are not sufficient
#include <qobject.h>

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#include <csmeta.h>
#include <qlog.h>
#include <qmap.h>
#include <qmultimap.h>
#include <qmutex.h>
#include <qstring8.h>
#include <qvector.h>

#include <optional>
#include <utility>
#include <stdexcept>
#include <typeindex>
#include <tuple>

class Q_CORE_EXPORT QMetaObject
{
 public:
   virtual ~QMetaObject() {}

   virtual QMetaClassInfo classInfo(int index) const = 0;
   virtual int classInfoCount() const = 0;
   int classInfoOffset() const;

   virtual const QString &className() const = 0;
   static void connectSlotsByName(QObject *object);

   virtual const QString &getInterface_iid() const = 0;

   virtual QMetaMethod constructor(int index) const = 0;
   virtual int constructorCount() const = 0;

   virtual QMetaEnum enumerator(int index) const = 0 ;
   virtual int enumeratorCount() const = 0;
   int enumeratorOffset() const;

   int indexOfClassInfo(const QString &name) const;
   int indexOfConstructor(const QString &constructor) const;
   int indexOfEnumerator(const QString &name) const;
   int indexOfMethod(const QString &method) const;
   int indexOfMethod(const CsSignal::Internal::BentoAbstract &tmp) const;
   int indexOfProperty(const QString &name) const;
   int indexOfSignal(const QString &signal) const;
   int indexOfSlot(const QString &slot) const;

   template<class SignalClass, class ...SignalArgs>
   int indexOfSignal(void (SignalClass::*methodPtr)(SignalArgs...)) const;

   template<class MethodClass, class ...MethodArgs, class MethodReturn>
   int indexOfMethod(MethodReturn (MethodClass::*methodPtr)(MethodArgs...)) const;

   virtual QMetaMethod method(int index) const = 0;
   QMetaMethod method(const CSBentoAbstract &tmp) const;

   // alternate name for method()
   QMetaMethod lookUpMethod(const CSBentoAbstract &tmp) const {
      return method(tmp);
   }

   virtual int methodCount() const = 0;
   int methodOffset() const;

   virtual QMetaProperty property(int index) const = 0;
   virtual int propertyCount() const = 0;
   int propertyOffset() const;

   virtual const QMetaObject *superClass() const = 0;

   QString tr(const char *text, const char *comment = nullptr, std::optional<int> numArg = std::optional<int>()) const;
   QString tr(const QString &text, const QString &comment = QString(), std::optional<int> numArg = std::optional<int>()) const;

   QMetaProperty userProperty() const;

   //
   static bool checkConnectArgs(const QString &signal, const QString &method);
   static bool checkConnectArgs(const QMetaMethod &signal, const QMetaMethod &method);
   static QString normalizedSignature(const QString &method);
   static QString normalizedType(const QString &type);

   template<class T>
   static QMetaEnum findEnum();

   static QMetaEnum findEnum(std::type_index id);

   template<class R, class ...Ts>
   static bool invokeMethod(QObject *object, const QString &member, Qt::ConnectionType type, CSReturnArgument<R> retval,
         CSArgument<Ts>... Vs);

   template<class R, class ...Ts>
   static bool invokeMethod(QObject *object, const QString &member, CSReturnArgument<R> retval, CSArgument<Ts>... Vs);

   template<class ...Ts>
   static bool invokeMethod(QObject *object, const QString &member, Qt::ConnectionType type, CSArgument<Ts>... Vs);

   template<class ...Ts>
   static bool invokeMethod(QObject *object, const QString &member, CSArgument<Ts>... Vs);

   template<class MethodClass, class... MethodArgs, class MethodReturn>
   QMetaMethod method(MethodReturn (MethodClass::*methodPtr)(MethodArgs...) ) const;

   template<class ...Ts>
   QObject *newInstance(Ts... Vs) const;

   QObject *newInstance() const;

 protected:
   static std::tuple<std::vector<QString>, QString, std::vector<QString>> getSignatures(const QString &fullName);

   // global enum list
   static QMap<std::type_index, std::pair<QMetaObject *, QString>> &m_enumsAll();

   int enum_calculate(QString enumData, QMap<QString, int> valueMap);

 private:
   static QString getType(const QString &fullName);
};

template<class SignalClass, class ...SignalArgs>
int QMetaObject::indexOfSignal(void (SignalClass::*methodPtr)(SignalArgs...)) const
{
   int retval = -1;
   const int count = methodCount();

   CSBento<void (SignalClass::*)(SignalArgs...)> tmp = methodPtr;

   for (int index = 0; index < count; ++index)  {
      QMetaMethod metaMethod = method(index);

      bool ok = metaMethod.compare(tmp);

      if (ok && metaMethod.methodType() == QMetaMethod::Signal) {
         // found QMetaMethod match
         retval = index;
         break;
      }
   }

   return retval;
}

template<class MethodClass, class ...MethodArgs, class MethodReturn>
int QMetaObject::indexOfMethod(MethodReturn (MethodClass::*methodPtr)(MethodArgs...)) const
{
   int retval = -1;
   const int count = methodCount();

   CSBento<MethodReturn (MethodClass::*)(MethodArgs...)> tmp = methodPtr;

   for (int index = 0; index < count; ++index)  {
      QMetaMethod metaMethod = method(index);

      bool ok = metaMethod.compare(tmp);

      if (ok) {
         // found QMetaMethod match
         retval = index;
         break;
      }
   }

   return retval;
}

template<class T>
QMetaEnum QMetaObject::findEnum()
{
   // call non template version
   return findEnum(typeid(T));
}

template<class MethodClass, class... MethodArgs, class MethodReturn>
QMetaMethod QMetaObject::method(MethodReturn (MethodClass::*methodPtr)(MethodArgs...) ) const
{
   QMetaMethod retval;
   const int count = methodCount();

   CSBento<MethodReturn (MethodClass::*)(MethodArgs...)> tmp = methodPtr;

   for (int index = 0; index < count; ++index)  {
      QMetaMethod metaMethod = method(index);

      bool ok = metaMethod.compare(tmp);

      if (ok) {
         // found QMetaMethod match
         return metaMethod;
      }
   }

   return retval;
}

template<class ...Ts>
QObject *QMetaObject::newInstance(Ts... Vs) const
{
   QString constructorSig = className();

   // signature of the method being invoked
   constructorSig += "(";
   constructorSig += cs_typeToName<Ts...>();
   constructorSig += ")";

   int index = this->indexOfConstructor(constructorSig);

   if (index == -1)  {
      return nullptr;
   }

   QMetaMethod metaMethod = this->constructor(index);
   QObject *retval = nullptr;

   // about to call QMetaMethod::invoke()
   metaMethod.invoke(0, Qt::DirectConnection, CSReturnArgument<QObject *>(retval), Vs...);

   return retval;
}

#if ! defined (CS_DOXYPRESS)

// **
class Q_CORE_EXPORT QMetaObject_X : public QMetaObject
{
 public:
   void register_classInfo(const QString &name, const QString &value);
   QMetaClassInfo classInfo(int index) const override final;

   int classInfoCount() const override final;

   QMetaMethod constructor(int index) const override final;
   int constructorCount() const override final;

   void register_enum_data(const QString &args);

   QMetaEnum enumerator(int index) const override final;
   int enumeratorCount() const override final;

   QMetaMethod method(int index) const override final;
   int methodCount() const override final;

   QMetaProperty property(int index) const override final;
   int propertyCount() const override final;

   //
   int  register_enum(const QString &name, std::type_index id, const QString &scope);
   int  register_flag(const QString &enumName, const QString &scope, const QString &flagName, std::type_index id);
   void register_tag(const QString &name, const QString &method);

   void register_method_s1(const QString &name, QMetaMethod::Access access, QMetaMethod::MethodType kind);
   void register_method_s2_part2(QString className, const QString &name, CSBentoAbstract *methodBento, QMetaMethod::MethodType kind);

   // properties
   void register_property_read(const QString &name, std::type_index returnTypeId,
         QString (*returnTypeFuncPtr)(), JarReadAbstract *readJar);

   void register_property_write(const QString &name, JarWriteAbstract *method, const QString &methodName);
   void register_property_bool(const QString &name, JarReadAbstract *method, QMetaProperty::Kind kind);

   void register_property_int(const QString &name, int value, QMetaProperty::Kind kind);

 protected:
   QMap<QString, QMetaClassInfo> m_classInfo;
   QMap<QString, QMetaMethod>    m_constructor;         // constructor
   QMap<QString, QMetaMethod>    m_methods;             // methods, signals, slots
   QMap<QString, QMetaEnum>      m_enums;
   QMap<QString, QMetaProperty>  m_properties;
   QMultiMap<QString, QString>   m_flag;
};

// **
template<class T>
class QMetaObject_T : public QMetaObject_X
{
 public:
   QMetaObject_T() = default;

   void postConstruct();

   const QString &className() const override;
   const QString &getInterface_iid() const override;
   const QMetaObject *superClass() const override;

   // revision
   template<class U>
   void register_method_rev(U method, int revision);

   // signals
   template<class U>
   void register_method_s2(const QString &name, U method, QMetaMethod::MethodType kind);

   // slots, invokables
   template<class U>
   void register_method(const QString &name, U method, QMetaMethod::MethodType kind,
         const QString &va_args, QMetaMethod::Access access);

   // properties
   template<class U>
   void register_property_notify(const QString &name, U method);

   template<class U>
   void register_property_reset(const QString &name, U method);
};

template<class T>
void QMetaObject_T<T>::postConstruct()
{
   // calls the first "overload" to ensure the other overloads are processed
   T::cs_regTrigger(cs_number<0>());
}

template<class T>
const QString &QMetaObject_T<T>::className() const
{
   static QString retval = QString::fromUtf8(T::cs_className());
   return retval;
}

template<class T>
inline const QString &qobject_interface_iid();

template<class T>
const QString &QMetaObject_T<T>::getInterface_iid() const
{
   return qobject_interface_iid<T *>();
}

template<class T>
const QMetaObject *QMetaObject_T<T>::superClass() const
{
   if constexpr (std::is_same_v<typename T::cs_parent, CSGadget_Fake_Parent>) {
      return nullptr;

   } else {
      return &T::cs_parent::staticMetaObject();
   }
}

template<>
inline const QMetaObject *QMetaObject_T<QObject>::superClass() const
{
   return nullptr;
}

// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_method_rev(U methodPtr, int revision)
{
   CSBento<U> temp = CSBento<U>(methodPtr);

   QString tokenName;
   QMetaMethod data;

   for (auto k = m_methods.begin(); k != m_methods.end(); ++k)  {

      if ( k.value().compare(temp) )  {
         tokenName = k.key();
         data      = k.value();
         break;
      }
   }

   if (tokenName.isEmpty())  {
      qWarning("Macro CS_REVISION() method has not been registered for class %s", this->className());
      return;
   }

   // retrieve existing obj
   data.setRevision(revision);

   // update master map
   m_methods.insert(tokenName, data);
}

// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_method_s2(const QString &name, U methodPtr, QMetaMethod::MethodType kind)
{
   CSBento<U> *methodBento = new CSBento<U>(methodPtr);

   if (name.isEmpty()) {
      return;
   }

   QString className = T::staticMetaObject().className();
   register_method_s2_part2(className, name, methodBento, kind);
}

// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_method(const QString &name, U methodPtr, QMetaMethod::MethodType kind,
      const QString &va_args, QMetaMethod::Access access)
{
   if (name.isEmpty() || va_args.isEmpty()) {
      return;
   }

   auto [signatures, typeReturn, paramNames] = this->getSignatures(va_args);

   auto count = signatures.size();                              // base method plus number of defaulted parameters
   std::vector<QString> tmpArgNames = paramNames;

   QMetaMethod::Attributes attr = QMetaMethod::Attributes();

   for (std::vector<QString>::size_type k = 0; k < count; ++k)  {

      if (count > 1) {
         // remove defaulted parameter names
         int howMany = paramNames.size() - ((count - 1) - k);
         tmpArgNames = std::vector<QString>(paramNames.begin(), paramNames.begin() + howMany);

         if (k == count - 1) {
            // base method
            attr = QMetaMethod::Attributes();

         }  else {
            attr = QMetaMethod::Cloned;
         }
      }

      // remove all spaces from the key
      QString tokenKey = signatures[k];
      tokenKey.remove(' ');

      // save the key/value into the master map
      QMetaMethod data(typeReturn, tokenKey, tmpArgNames, access, kind, attr, this);

      CSBento<U> *temp = new CSBento<U>(methodPtr);
      data.setBentoBox(temp);

      if (kind == QMetaMethod::Constructor) {
         m_constructor.insert(tokenKey, data);
      } else  {
         m_methods.insert(tokenKey, data);
      }
   }
}

// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_property_notify(const QString &name, U methodPtr)
{
   if (name.isEmpty()) {
      return;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   data.setNotifyMethod(methodPtr);

   // update QMetaProperty
   m_properties.insert(name, data);
}

template<class T>
template<class U>
void QMetaObject_T<T>::register_property_reset(const QString &name, U methodPtr)
{
   if (name.isEmpty()) {
      return;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   //
   data.setResetMethod(methodPtr);

   // update QMetaProperty
   m_properties.insert(name, data);
}

// best way to handle declarations
#include <csmeta_internal_2.h>

#endif   // doxypress

#endif
