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

   virtual const QString8 &className() const = 0;
   static void connectSlotsByName(QObject *o);

   virtual const QString8 &getInterface_iid() const = 0;

   virtual QMetaMethod constructor(int index) const = 0;
   virtual int constructorCount() const = 0;

   virtual QMetaEnum enumerator(int index) const = 0 ;
   virtual int enumeratorCount() const = 0;
   int enumeratorOffset() const;

   int indexOfClassInfo(const QString8 &name) const;
   int indexOfConstructor(const QString8 &constructor) const;
   int indexOfEnumerator(const QString8 &name) const;
   int indexOfMethod(const QString8 &method) const;
   int indexOfMethod(const CsSignal::Internal::BentoAbstract &temp) const;
   int indexOfProperty(const QString8 &name) const;
   int indexOfSignal(const QString8 &signal) const;
   int indexOfSlot(const QString8 &slot) const;

   virtual QMetaMethod method(int index) const = 0;
   QMetaMethod method(const CSBentoAbstract &temp) const;

   // alternate name for method()
   QMetaMethod lookUpMethod(const CSBentoAbstract &temp) const {
      return method(temp);
   }

   virtual int methodCount() const = 0;
   int methodOffset() const;

   virtual QMetaProperty property(int index) const = 0;
   virtual int propertyCount() const = 0;
   int propertyOffset() const;

   virtual const QMetaObject *superClass() const = 0;

   QString8 tr(const char *s, const char *c = 0, int n = -1) const;
   QString8 trUtf8(const char *s, const char *c = 0, int n = -1) const;

   QMetaProperty userProperty() const;

   //
   static bool checkConnectArgs(const QString8 &signal, const QString8 &method);
   static bool checkConnectArgs(const QMetaMethod &signal, const QMetaMethod &method);
   static QString8 normalizedSignature(const QString8 &method);
   static QString8 normalizedType(const QString8 &type);

   template<class T>
   static QMetaEnum findEnum();

   template<class R, class ...Ts>
   static bool invokeMethod(QObject *object, const QString8 &member, Qt::ConnectionType type, CSReturnArgument<R> retval,
                            CSArgument<Ts>... Vs);

   template<class R, class ...Ts>
   static bool invokeMethod(QObject *object, const QString8 &member, CSReturnArgument<R> retval, CSArgument<Ts>... Vs);

   template<class ...Ts>
   static bool invokeMethod(QObject *object, const QString8 &member, Qt::ConnectionType type, CSArgument<Ts>... Vs);

   template<class ...Ts>
   static bool invokeMethod(QObject *object, const QString8 &member, CSArgument<Ts>... Vs);

   template<class MethodClass, class... MethodArgs>
   QMetaMethod method( void (MethodClass::*methodPtr)(MethodArgs...) ) const;

   template<class ...Ts>
   QObject *newInstance(Ts... Vs) const;

 protected:
   static std::tuple<std::vector<QString8>, QString8, std::vector<QString8>> getSignatures(const QString8 &fullName);

   // global enum list
   static QMap<std::type_index, std::pair<QMetaObject *, QString8> > &m_enumsAll();

   int enum_calculate(QString8 enumData, QMap<QString8, int> valueMap);

 private:
   static QString8 getType(const QString8 &fullName);
};

template<class T>
QMetaEnum QMetaObject::findEnum()
{
   QMetaEnum data;
   std::pair<QMetaObject *, QString8> enumData;

   // look up the enum type
   auto iter_enum = m_enumsAll().find(typeid(T));

   if (iter_enum == m_enumsAll().end()) {
      // no QMetaEnum for T

   } else  {
      enumData = iter_enum.value();

      QMetaObject *metaObj = enumData.first;
      QString8 name        = enumData.second;

      int index = metaObj->indexOfEnumerator(name);
      data = metaObj->enumerator(index);
   }

   return data;
}


template<class MethodClass, class... MethodArgs>
QMetaMethod QMetaObject::method( void (MethodClass::*methodPtr)(MethodArgs...) ) const
{
   QMetaMethod retval;
   const int count = methodCount();

   CSBento<void (MethodClass::*)(MethodArgs...)> temp = methodPtr;

   for (int index = 0; index < count; ++index)  {
      QMetaMethod metaMethod = method(index);

      bool ok = metaMethod.compare(temp);

      if (ok) {
         // found QMetaMethod match
         retval = metaMethod;
         break;
      }
   }

   return retval;
}

template<class ...Ts>
QObject *QMetaObject::newInstance(Ts... Vs) const
{
   QString8 constructorName = className();

   // signature of the method being invoked
   constructorName += "(";
   constructorName += cs_typeName<Ts...>();
   constructorName += ")";

   int index = this->indexOfConstructor(constructorName);

   if (index == -1)  {
      return 0;
   }

   QMetaMethod metaMethod = this->constructor(index);
   QObject *retval = 0;

   // about to call QMetaMethod::invoke()
   metaMethod.invoke(0, Qt::DirectConnection, CSReturnArgument<QObject *>(retval), Vs...);

   return retval;
}

/**   \cond INTERNAL (notation so DoxyPress will not parse this class  */

// **
class Q_CORE_EXPORT QMetaObject_X : public QMetaObject
{
 public:
   void register_classInfo(const QString8 &name, const QString8 &value);
   QMetaClassInfo classInfo(int index) const override;

   int classInfoCount() const override;

   QMetaMethod constructor(int index) const override;
   int constructorCount() const override;

   void register_enum_data(const QString8 &args);

   QMetaEnum enumerator(int index) const override;
   int enumeratorCount() const override;

   QMetaMethod method(int index) const override;
   int methodCount() const override;

   QMetaProperty property(int index) const override;
   int propertyCount() const override;

   //
   int  register_enum(const QString8 &name, std::type_index id, const QString8 &scope);
   int  register_flag(const QString8 &enumName, const QString8 &scope, const QString8 &flagName, std::type_index id);
   void register_tag(const QString8 &name, const QString8 &method);

   void register_method_s1(const QString8 &name, QMetaMethod::Access access, QMetaMethod::MethodType kind);

   // properties
   int register_property_read(const QString8 &name, const QString8 &dataType, JarReadAbstract *readJar);
   int register_property_write(const QString8 &name, JarWriteAbstract *method);
   int register_property_bool(const QString8 &name, JarReadAbstract *method, QMetaProperty::Kind kind);

   void register_property_int(const QString8 &name, int value, QMetaProperty::Kind kind);

 protected:
   QMap<QString8, QMetaClassInfo> m_classInfo;
   QMap<QString8, QMetaMethod>    m_constructor;         // constructor
   QMap<QString8, QMetaMethod>    m_methods;             // methds, signals, slots
   QMap<QString8, QMetaEnum>      m_enums;
   QMap<QString8, QMetaProperty>  m_properties;
   QMultiMap<QString8, QString8>  m_flag;
};

// **
template<class T>
class QMetaObject_T : public QMetaObject_X
{
   public:
      QMetaObject_T();

      void postConstruct();

      const QString8 &className() const override;
      const QString8 &getInterface_iid() const override;
      const QMetaObject *superClass() const override;

      // revision
      template<class U>
      void register_method_rev(U method, int revision);

      // signals
      template<class U>
      void register_method_s2(const QString8 &name, U method, QMetaMethod::MethodType kind);

      // slots, invokables
      template<class U>
      void register_method(const QString8 &name, U method, QMetaMethod::MethodType kind,
                  const QString8 &va_args, QMetaMethod::Access access);

      // properties
      template<class U>
      void register_property_notify(const QString8 &name, U method);

      template<class U>
      void register_property_reset(const QString8 &name, U method);
};


template<class T>
QMetaObject_T<T>::QMetaObject_T()
{
}

template<class T>
void QMetaObject_T<T>::postConstruct()
{
   // calls the overloaded version to ensure the other overloads are processed
   T::cs_regTrigger(cs_number<0>());
}

template<class T>
const QString8 &QMetaObject_T<T>::className() const
{
   static QString8 retval = QString::fromUtf8(T::cs_className());
   return retval;
}

template<class T>
inline const QString8 &qobject_interface_iid();

template<class T>
const QString8 &QMetaObject_T<T>::getInterface_iid() const
{
   return qobject_interface_iid<T *>();
}

template<class T>
const QMetaObject *QMetaObject_T<T>::superClass() const
{
   return &T::cs_parent::staticMetaObject();
}

template<>
inline const QMetaObject *QMetaObject_T<QObject>::superClass() const
{
   return 0;
}

// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_method_rev(U method, int revision)
{
   CSBento<U> temp = CSBento<U>(method);

   QString8 tokenName;
   QMetaMethod data;

   for (auto k = m_methods.begin(); k != m_methods.end(); ++k)  {

      if ( k.value().compare(temp) )  {
         tokenName = k.key();
         data      = k.value();
         break;
      }
   }

   if (tokenName.isEmpty())  {
      qWarning("Macro CS_REVISION() Method has not been registered for class %s", this->className());
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
void QMetaObject_T<T>::register_method_s2(const QString8 &name, U method, QMetaMethod::MethodType kind)
{
   if (name.isEmpty()) {
      return;
   }

   QMap<QString8, QMetaMethod> *map;

   if (kind == QMetaMethod::Constructor) {
      map = &m_constructor;

   } else {
      map = &m_methods;

   }

   QString8 tokenName = name;
   tokenName.remove(' ');

   QMetaMethod data;

   if (tokenName.contains("("))  {
      // has a paren in name, overloaded method

      tokenName = normalizedSignature(name);
      tokenName.remove(' ');

      auto item  = map->find(tokenName);
      bool found = ( item != map->end() );

      if (! found)  {
         // entry not found in QMap

         QString8 msg = T::staticMetaObject().className();
         msg += "::" + name + " Unable to register overloaded method pointer, verify signal/slot";

         qDebug("%s", msg);
         throw std::logic_error(std::string {msg.constData()});

      } else {
         // retrieve existing obj
         data = item.value();
      }

      CSBento<U> *temp = new CSBento<U>(method);
      data.setBentoBox(temp);

      // update master map
      map->insert(tokenName, data);

   } else {
      // no paren in name, set itemU to one past the last matching method

      auto itemL = map->lowerBound(tokenName + '(' );
      auto itemU = map->lowerBound(tokenName + ')' );

      if (itemL == itemU) {
         // no matches found in QMap

         QString8 msg = T::staticMetaObject().className();
         msg += "::" + name + " Unable to register method pointer, verify signal/slot";

         qDebug("%s", msg);
         throw std::logic_error(std::string {msg.constData()});
      }

      for (auto index = itemL; index != itemU; ++index)  {

         // retrieve existing obj
         QString8 key = index.key();
         data = index.value();

         //
         CSBento<U> *temp = new CSBento<U>(method);
         data.setBentoBox(temp);

         // update existing obj
         map->insert(key, data);
      }
   }
}


// **
template<class T>
template<class U>
void QMetaObject_T<T>::register_method(const QString8 &name, U method, QMetaMethod::MethodType kind,
                  const QString8 &va_args, QMetaMethod::Access access)
{
   if (name.isEmpty() || va_args.isEmpty()) {
      return;
   }

   // declare first
   std::vector<QString8> signatures;
   QString8 typeReturn;

   std::vector<QString8> paramNames;
   std::tie(signatures, typeReturn, paramNames) = this->getSignatures(va_args);

   //
   QMetaMethod::Attributes attr = QMetaMethod::Attributes();
   auto count = signatures.size();                              // base method plus number of defaulted parameters

   std::vector<QString8> tmpArgNames = paramNames;

   for (std::vector<QString8>::size_type k = 0; k < count; ++k)  {

      if (count > 1) {
         // remove defaulted parameter names
         int howMany = paramNames.size() - ((count - 1) - k);
         tmpArgNames = std::vector<QString8>(paramNames.begin(), paramNames.begin() + howMany);

         if (k == count - 1) {
            // base method
            attr = QMetaMethod::Attributes();

         }  else {
            attr = QMetaMethod::Cloned;
         }
      }

      // remove all spaces from the key
      QString8 tokenKey = signatures[k];
      tokenKey.remove(' ');

      // save the key/value into the master map
      QMetaMethod data(typeReturn, tokenKey, tmpArgNames, access, kind, attr, this);

      CSBento<U> *temp = new CSBento<U>(method);
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
void QMetaObject_T<T>::register_property_notify(const QString8 &name, U method)
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

   data.setNotifyMethod(method);

   // update QMetaProperty
   m_properties.insert(name, data);
}


template<class T>
template<class U>
void QMetaObject_T<T>::register_property_reset(const QString8 &name, U method)
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
   data.setResetMethod(method);

   // update QMetaProperty
   m_properties.insert(name, data);
}

// best way to handle declarations
#include <csmeta_internal_2.h>

/**   \endcond   */

#endif
