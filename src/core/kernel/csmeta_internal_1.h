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

#ifndef CSMETA_INTERNAL_1_H
#define CSMETA_INTERNAL_1_H

// include first, do not move
#include <qstring8.h>

#include <qvariant.h>

#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <tuple>

class QObject;
class QMetaObject;

template<class T1>
const QString &cs_typeToName();

// csArgument
template <typename T>
class CSArgument
{
 public:
   CSArgument(const T &data);
   CSArgument(const T &data, const QString &typeName);

   T getData() const;
   const QString &getTypeName() const;

 private:
   T m_data;
   QString m_typeName;
};

template <typename T>
CSArgument<T>::CSArgument(const T &data)
   : m_data(data), m_typeName(cs_typeToName<T>())
{
}

template <typename T>
CSArgument<T>::CSArgument(const T &data, const QString &typeName)
   : m_data(data), m_typeName(typeName)
{
}

template <typename T>
T CSArgument<T>::getData() const
{
   return m_data;
}

template <typename T>
const QString &CSArgument<T>::getTypeName() const
{
   return m_typeName;
}

inline const QString &cs_argName()
{
   static QString retval("");
   return retval;
}

template <typename T>
const QString &cs_argName(const CSArgument<T> &data)
{
   return data.getTypeName();
}

template<class T1, class T2, class ...Ts>
const QString &cs_argName(const CSArgument<T1> &data1, const CSArgument<T2> &data2, const CSArgument<Ts> &... dataX)
{
   static thread_local QString argName;
   argName = cs_argName(data1) + "," + cs_argName(data2, dataX...);

   return argName;
}

// csGeneric
class CSGenericReturnArgument
{
 public:
   virtual ~CSGenericReturnArgument() = default;
};

template <typename T>
class CSReturnArgument : public CSGenericReturnArgument
{
 public:
   CSReturnArgument(T &data);
   T getData() const;
   void setData(const T &data);

 private:
   typename std::remove_const<T>::type &m_data;
};

template <typename T>
CSReturnArgument<T>::CSReturnArgument(T &data)
   : m_data(data)
{
}

template <typename T>
T CSReturnArgument<T>::getData() const
{
   return m_data;
}

template <typename T>
void CSReturnArgument<T>::setData(const T &data)
{
   m_data = data;
}

template <>
class CSReturnArgument<void> : public CSGenericReturnArgument
{
 public:
   CSReturnArgument();
   void setData(CsSignal::Internal::CSVoidReturn data);
};

inline CSReturnArgument<void>::CSReturnArgument()
{
}

inline void CSReturnArgument<void>::setData(CsSignal::Internal::CSVoidReturn)
{
}

// registration of enums and flags
template<class T>
struct cs_is_enum_or_flag : public std::is_enum<T> {
};

template<class T>
struct cs_is_enum_or_flag<QFlags<T>>
   : public std::integral_constant<bool, true> {
};

template<class T>
struct cs_underlying_type
   : public std::underlying_type<T> {
};

template<class T>
struct cs_underlying_type<QFlags<T>>
   : public std::underlying_type<T> {
};

template<class T>
QVariant cs_convertToQVariant(T data);

template <class T>
std::pair<T, bool> convertFromQVariant(QVariant data);

// ***********

// ** READ
class JarReadAbstract
{
 public:
   virtual ~JarReadAbstract()
   {
   }

   virtual QVariant runV(const QObject *) const = 0;

   template<class R>
   R run(const QObject *) const;

   virtual bool isStatic() const {
      return false;
   }
};

template<class R>
class JarRead : public JarReadAbstract
{
 public:
   virtual R call(const QObject *) const = 0;
};

template<class R>
R JarReadAbstract::run(const QObject *obj) const
{
   // did the user use the correct data type?
   const JarRead<R> *testMe = dynamic_cast<const JarRead<R> *>(this);

   if (testMe) {
      return testMe->call(obj);

   } else {
      throw std::logic_error("QObject::property() Type mismatch in read");

   }
}

template<class T, class R>
class SpiceJarRead : public JarRead<R>
{
 public:
   SpiceJarRead(R (*func)());
   SpiceJarRead(R (T::*method)() const);

   QVariant runV(const QObject *) const override;
   R call(const QObject *) const override;

   bool isStatic() const override {
      return m_func != nullptr;
   }

 private:
   R (T::*m_method)() const;
   R (*m_func)();
};

template<class T, class R>
SpiceJarRead<T, R>::SpiceJarRead(R (*func)())
{
   m_func   = func;
   m_method = nullptr;
}

template<class T, class R>
SpiceJarRead<T, R>::SpiceJarRead(R (T::*method)() const)
{
   m_func   = nullptr;
   m_method = method;
}

template<class T, class R>
QVariant SpiceJarRead<T, R>::runV(const QObject *obj) const
{
   QVariant retval;

   if (m_func)  {
      retval = cs_convertToQVariant( m_func() );

   } else {
      const T *testObj = dynamic_cast< const T *>(obj);

      if (! testObj) {
         return QVariant();
      }

      retval = cs_convertToQVariant( ((testObj)->*(m_method))() );
   }

   return retval;
}

template<class T, class R>
R SpiceJarRead<T, R>::call(const QObject *obj) const
{
   if (m_func)  {
      return m_func();

   } else {
      // is the passed obj the correct class
      const T *testObj = dynamic_cast< const T *>(obj);

      if (! testObj) {
         throw std::logic_error("Can not read a property in an object of the wrong class");
      }

      return ((testObj)->*(m_method))();
   }
}

// ** WRITE
class JarWriteAbstract
{
 public:
   virtual ~JarWriteAbstract() {}
   virtual bool runV(QObject *, QVariant data) const = 0;

   template<class V>
   bool run(const QObject *, V data) const;
};

template<class V>
class JarWrite : public JarWriteAbstract
{
 public:
   virtual bool call(QObject *, V data) const = 0;
};

template<class T, class V>
class SpiceJarWrite : public JarWrite<V>
{
 public:
   SpiceJarWrite(void (T::*method)(V));

   bool runV(QObject *, QVariant data) const override;
   bool call(QObject *, V data) const override;

 private:
   void (T::*m_method)(V);

};

template<class T, class V>
SpiceJarWrite<T, V>::SpiceJarWrite(void (T::*method)(V))
{
   m_method = method;
}

template<class T, class V>
bool SpiceJarWrite<T, V>::runV(QObject *obj, QVariant data) const
{
   T *testObj = dynamic_cast<T *>(obj);

   if (! testObj) {
      return false;
   }

   // strip away const and & if they exist
   using bareType = typename std::remove_const<typename std::remove_reference<V>::type>::type;

   // convert data to type bareType
   std::pair<bareType, bool> retval = convertFromQVariant<bareType>(data);

   if (retval.second) {
      ((testObj)->*(m_method))(retval.first);
   }

   return retval.second;
}

template<class T, class V>
bool SpiceJarWrite<T, V>::call(QObject *obj, V data) const
{
   // is the passed obj the correct class
   T *testObj = dynamic_cast<T *>(obj);

   if (! testObj) {
      return false;
   }

   ((testObj)->*(m_method))(data);

   return true;
}

// ** RESET
class JarResetAbstract
{
 public:
   virtual ~JarResetAbstract() {}

   virtual bool runV(QObject *) const = 0;
};

class JarReset : public JarResetAbstract
{
 public:
};

template<class T>
class SpiceJarReset : public JarReset
{
 public:
   SpiceJarReset(void (T::*method)());

   bool runV(QObject *) const override;

 private:
   void (T::*m_method)();

};

template<class T>
SpiceJarReset<T>::SpiceJarReset(void (T::*method)())
{
   m_method = method;
}

template<class T>
bool SpiceJarReset<T>::runV(QObject *obj) const
{
   T *testObj = dynamic_cast<T *>(obj);

   if (! testObj) {
      return false;
   }

   ((testObj)->*(m_method))();

   return true;
}

/*

QMetaObject::activate() creates a TeaCup_Data object to store the slot data

   // real code
   TeaCup_Data<SignalArgTypes...> dataPack{Vs...};

An example of the SignalArgTypes is <int,bool>

class TeaCupAbstract
class Teacup<>                   inherits from TeaCupAbstract
class Teacup<std::tuple<>>       inherits from Teacup<>
class Teacup<int>                inherits from Teacup<std::tuple<>>
class Teacup<std::tuple<int>>    inherits from Teacup<int>
class Teacup<int,bool>           inherits from Teacup<std::tuple<int>>
class Teacup_Data<int,bool>      inherits from Teacup<int,bool>

*/

// ** store method pointer for signals and slots
class CSBentoAbstract : public virtual CsSignal::Internal::BentoAbstract
{
 public:
   using CsSignal::Internal::BentoAbstract::invoke;
   virtual void invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
         CSGenericReturnArgument *retval = nullptr) const = 0;

   virtual bool checkReturnType(CSGenericReturnArgument &retval) const = 0;
};

template<class T>
class CSBento : public CSBentoAbstract, public CsSignal::Internal::Bento<T>
{
 public:
   CSBento(T ptr);

   std::unique_ptr<CsSignal::Internal::BentoAbstract> clone() const override;

   using CsSignal::Internal::Bento<T>::invoke;
   void invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
         CSGenericReturnArgument *retval = nullptr) const override;

   bool checkReturnType(CSGenericReturnArgument &retval) const override;
};

template<class FunctionReturn, class ...FunctionArgs>
class CSBento<FunctionReturn (*)(FunctionArgs...)> : public CSBentoAbstract,
      public CsSignal::Internal::Bento<FunctionReturn (*)(FunctionArgs...)>
{
 public:
   CSBento(FunctionReturn (*ptr)(FunctionArgs...));

   std::unique_ptr<CsSignal::Internal::BentoAbstract> clone() const override;

   using CsSignal::Internal::Bento<FunctionReturn (*)(FunctionArgs...)>::invoke;
   void invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
         CSGenericReturnArgument *retval = nullptr) const override;

   bool checkReturnType(CSGenericReturnArgument &retval) const override;
};

template<class MethodClass, class MethodReturn, class...MethodArgs>
class CSBento<MethodReturn(MethodClass::*)(MethodArgs...)>: public CSBentoAbstract,
      public CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...)>
{
 public:
   CSBento(MethodReturn(MethodClass::*ptr)(MethodArgs...) );

   std::unique_ptr<CsSignal::Internal::BentoAbstract> clone() const override;

   using CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...)>::invoke;
   void invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
         CSGenericReturnArgument *retval = nullptr) const override;

   bool checkReturnType(CSGenericReturnArgument &retval) const override;
};

template<class MethodClass, class MethodReturn, class...MethodArgs>
class CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>: public CSBentoAbstract,
      public CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>
{
   // specialization, pointer to const method

 public:
   CSBento(MethodReturn(MethodClass::*ptr)(MethodArgs...) const);

   std::unique_ptr<CsSignal::Internal::BentoAbstract> clone() const override;

   using CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>::invoke;
   void invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
         CSGenericReturnArgument *retval = nullptr) const override;

   bool checkReturnType(CSGenericReturnArgument &retval) const override;
};

// (1) lambda
template<class T>
CSBento<T>::CSBento(T lambda)
   : CsSignal::Internal::Bento<T>(lambda)
{
}

template<class T>
std::unique_ptr<CsSignal::Internal::BentoAbstract> CSBento<T>::clone() const
{
   return std::make_unique<CSBento<T>>(*this);
}

template<class T>
void CSBento<T>::invoke(QObject *receiver, const CsSignal::Internal::TeaCupAbstract *dataPack,
      CSGenericReturnArgument *retval) const
{
   (void) receiver;
   (void) retval;

   // T must be a callable class
   auto methodPtr = &T::operator();

   this->invoke_internal(dataPack, methodPtr);
}

template<class T>
bool CSBento<T>::checkReturnType(CSGenericReturnArgument &retval) const
{
   // return type of a lambda is not required

   if (dynamic_cast<CSReturnArgument<void> *>(&retval)) {
      // return argument is void
      return true;
   }

   return false;
}

// (2) specialization, function pointer
template<class FunctionReturn, class ...FunctionArgs>
CSBento<FunctionReturn (*)(FunctionArgs...)>::CSBento(FunctionReturn (*ptr)(FunctionArgs...))
   : CsSignal::Internal::Bento<FunctionReturn (*)(FunctionArgs...)>(ptr)
{
}

template<class FunctionReturn, class ...FunctionArgs>
std::unique_ptr<CsSignal::Internal::BentoAbstract> CSBento<FunctionReturn (*)(FunctionArgs...)>::clone() const
{
   return std::make_unique<CSBento<FunctionReturn (*)(FunctionArgs...)>>(*this);
}

template<class FunctionReturn, class ...FunctionArgs>
void CSBento<FunctionReturn (*)(FunctionArgs...)>::invoke(QObject *,
      const CsSignal::Internal::TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   // no need to verify receiver since it is not used

   // dynamic cast will return a valid ptr if the slot has equal or less parameters
   // retrieve ptr to teaCup object, which contains the data
   const CsSignal::Internal::TeaCup<FunctionArgs...> *teaCup = dynamic_cast<const CsSignal::Internal::TeaCup<FunctionArgs...> *>(dataPack);

   if (teaCup) {
      // expand arguments
      std::tuple<FunctionArgs...> args = teaCup->getData();

      CSReturnArgument<FunctionReturn> *returnData = dynamic_cast<CSReturnArgument<FunctionReturn> *>(retval);

      if (returnData) {
         // unpack the tuple, then call the method or slot
         returnData->setData(CsSignal::Internal::cs_unpack_function_args(this->m_methodPtr, args));

      } else {
         // returnData is a nullptr, therefore retval was null or the wrong type
         // assume user does not want a return value

         // unpack the tuple, then call the method or slot
         CsSignal::Internal::cs_unpack_function_args(this->m_methodPtr, args);
      }
   }
}

template<class FunctionReturn, class ...FunctionArgs>
bool CSBento<FunctionReturn (*)(FunctionArgs...)>::checkReturnType(CSGenericReturnArgument &retval) const
{
   if (dynamic_cast<CSReturnArgument<FunctionReturn> *>(&retval)) {
      return true;
   }

   if (dynamic_cast<CSReturnArgument<void> *>(&retval)) {
      return true;
   }

   return false;
}

// (3) specialization, method pointer
template<class MethodClass, class MethodReturn, class...MethodArgs>
CSBento<MethodReturn(MethodClass::*)(MethodArgs...)>::CSBento(MethodReturn(MethodClass::*ptr)(MethodArgs...))
   : CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...)>(ptr)
{
}

template<class MethodClass, class MethodReturn, class...MethodArgs>
std::unique_ptr<CsSignal::Internal::BentoAbstract> CSBento<MethodReturn(MethodClass::*)(MethodArgs...)>::clone() const
{
   return std::make_unique<CSBento<MethodReturn(MethodClass::*)(MethodArgs...)>>(*this);
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
void CSBento<MethodReturn (MethodClass::*)(MethodArgs...)>::invoke(QObject *receiver,
      const CsSignal::Internal::TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   if (! receiver)  {
      return;
   }

   MethodClass *t_receiver = dynamic_cast<MethodClass *>(receiver);

   if (t_receiver) {

      // dynamic cast will return a valid ptr if the slot has equal or less parameters
      // retrieve ptr to teaCup object, which contains the data
      const CsSignal::Internal::TeaCup<MethodArgs...> *teaCup = dynamic_cast<const CsSignal::Internal::TeaCup<MethodArgs...> *>(dataPack);

      if (teaCup) {
         // expand arguments
         std::tuple<MethodArgs...> args = teaCup->getData();

         CSReturnArgument<MethodReturn> *returnData = dynamic_cast<CSReturnArgument<MethodReturn> *>(retval);

         if (returnData) {
            // unpacks the tuple, then calls the method or slot
            returnData->setData(CsSignal::Internal::cs_unpack_method_args(t_receiver, this->m_methodPtr, args));

         } else {
            // returnData is a null pointer, therefore retval was null or the wrong type
            // assume user does not want a return value

            // unpacks the tuple, then calls the method or slot
            CsSignal::Internal::cs_unpack_method_args(t_receiver, this->m_methodPtr, args);
         }

      }
   }
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
bool CSBento<MethodReturn(MethodClass::*)(MethodArgs...)>::checkReturnType(CSGenericReturnArgument &retval) const
{
   if (dynamic_cast<CSReturnArgument<MethodReturn> *>(&retval)) {
      return true;
   }

   if (dynamic_cast<CSReturnArgument<void> *>(&retval)) {
      return true;
   }

   return false;
}

// (4) specialization, pointer to const method
template<class MethodClass, class MethodReturn, class...MethodArgs>
CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>::CSBento(MethodReturn(MethodClass::*ptr)(MethodArgs...) const)
   : CsSignal::Internal::Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>(ptr)
{
}

template<class MethodClass, class MethodReturn, class...MethodArgs>
std::unique_ptr<CsSignal::Internal::BentoAbstract> CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>::clone() const
{
   return std::make_unique<CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>>(*this);
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
void CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>::invoke(QObject *receiver,
      const CsSignal::Internal::TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   if (! receiver)  {
      return;
   }

   MethodClass *t_receiver = dynamic_cast<MethodClass *>(receiver);

   if (t_receiver) {
      // dynamic cast will return a valid ptr if the slot has equal or less parameters

      // retrieve ptr to teaCup object, which contains the data
      const CsSignal::Internal::TeaCup<MethodArgs...> *teaCup = dynamic_cast<const CsSignal::Internal::TeaCup<MethodArgs...> *>(dataPack);

      if (teaCup) {
         // expand arguments
         std::tuple<MethodArgs...> args = teaCup->getData();

         CSReturnArgument<MethodReturn> *returnData = dynamic_cast<CSReturnArgument<MethodReturn> *>(retval);

         if (returnData) {
            // unpacks the tuple, then calls the method or slot
            returnData->setData(CsSignal::Internal::cs_unpack_method_args(t_receiver, this->m_methodPtr, args));

         } else {
            // returnData is a null pointer, therefore retval was null or the wrong type
            // assume user does not want a return value

            // unpacks the tuple, then calls the method or slot
            CsSignal::Internal::cs_unpack_method_args(t_receiver, this->m_methodPtr, args);

         }
      }
   }
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
bool CSBento<MethodReturn(MethodClass::*)(MethodArgs...) const>::checkReturnType(CSGenericReturnArgument &retval) const
{
   if (dynamic_cast<CSReturnArgument<MethodReturn> *>(&retval)) {
      return true;
   }

   if (dynamic_cast<CSReturnArgument<void> *>(&retval)) {
      return true;
   }

   return false;
}

#endif
