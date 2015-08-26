/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef CSMETA_INTERNAL_1_H
#define CSMETA_INTERNAL_1_H

#include <qmetatype.h>
#include <qvariant.h>
#include <qstring.h>

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <tuple>

class QObject;
class QMetaObject;


// ***********

template<class T1>
const char *cs_typeName();

template <typename T>
class CSArgument
{
 public:
   CSArgument(const T &data);
   CSArgument(const T &data, const char *typeName);

   T getData() const;
   const char *getTypeName() const;

 private:
   T m_data;
   const char *m_typeName;
};

template <typename T>
CSArgument<T>::CSArgument(const T &data)
   : m_data(data), m_typeName(cs_typeName<T>())
{
}

template <typename T>
CSArgument<T>::CSArgument(const T &data, const char *typeName)
   : m_data(data), m_typeName(typeName)
{
}

template <typename T>
T CSArgument<T>::getData() const
{
   return m_data;
}

template <typename T>
const char *CSArgument<T>::getTypeName() const
{
   return m_typeName;
}

inline const char *cs_argName()
{
   return "";
}

template <typename T>
const char *cs_argName(const CSArgument<T> &data)
{
   return data.getTypeName();
}

template<class T1, class T2, class ...Ts>
const char *cs_argName(const CSArgument<T1> &data1, const CSArgument<T2> &data2, const CSArgument<Ts> &... dataX)
{
   static QByteArray temp = QByteArray( cs_argName(data1) ) + "," + cs_argName(data2, dataX...);
   return temp.constData();
}



// ***********
class CSGenericReturnArgument
{
 public:
   virtual ~CSGenericReturnArgument();
};

inline CSGenericReturnArgument::~CSGenericReturnArgument()
{
}

//
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

// specialization
class CSVoidReturn
{
};

template <>
class CSReturnArgument<void> : public CSGenericReturnArgument
{
 public:
   CSReturnArgument();
   void setData(CSVoidReturn data);
};

inline CSReturnArgument<void>::CSReturnArgument()
{
}

inline void CSReturnArgument<void>::setData(CSVoidReturn)
{
}



// ***********

template<class T>
struct is_enum_or_flag: public std::is_enum<T> {
};

template<class T>
struct is_enum_or_flag<QFlags<T>>: public std::integral_constant<bool, true> {
};

template<class T>
struct cs_underlying_type: public std::underlying_type<T> {
};

template<class T>
struct cs_underlying_type<QFlags<T>>: public std::underlying_type<T> {
};

template < class T, class = void, class = typename std::enable_if < !std::is_constructible<QVariant, T>::value >::type >
QVariant cs_convertToQVariant(T data);

template<class T, class = typename std::enable_if<std::is_constructible<QVariant, T>::value>::type>
QVariant cs_convertToQVariant(T data);

template < class T, class = void, class = void, class = typename std::enable_if < (! is_enum_or_flag<T>::value) &&
           ! QMetaTypeId2<T>::Defined >::type >
std::pair<T, bool> convertFromQVariant(QVariant data);

template < class T, class = void, class = typename std::enable_if < (! is_enum_or_flag<T>::value) &&
           QMetaTypeId2<T>::Defined >::type >
std::pair<T, bool> convertFromQVariant(QVariant data);

template<class T, class = typename std::enable_if<is_enum_or_flag<T>::value>::type>
std::pair<T, bool> convertFromQVariant(QVariant data);



// ***********

// ** READ
class JarReadAbstract
{
 public:
   virtual ~JarReadAbstract() {}

   virtual QVariant runV(const QObject *) const = 0;

   template<class R>
   R run(const QObject *) const;
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

   QVariant runV(const QObject *) const;
   R call(const QObject *) const;

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

   bool runV(QObject *, QVariant data) const;
   bool call(QObject *, V data) const;

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

   // convert data to type V
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

   bool runV(QObject *) const;

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



// ***********


// ** index sequence (available in C++14)   (1)

// ** generate list of integers corresponding to the number of data types in a parameter pack
template<size_t...Ks>
class Index_Sequence
{
};

template<size_t S, class K1, class K2>
class Concat_Sequence;

template<size_t S, size_t ...M, size_t ...N>
class Concat_Sequence<S, Index_Sequence<M...>, Index_Sequence<N...>>
{
 public:
   using type = Index_Sequence < M... , (N + S)... >;
};

template<size_t N>
class Make_Index_Sequence
{
 public:
   using type = typename Concat_Sequence < (N / 2), typename Make_Index_Sequence < (N / 2) >::type,
         typename Make_Index_Sequence < (N - (N / 2)) >::type >::type;
};

template<>
class Make_Index_Sequence<0>
{
 public:
   using type = Index_Sequence<>;
};

template<>
class Make_Index_Sequence<1>
{
 public:
   using type = Index_Sequence<0>;
};

template<class ...Ts>
class Index_Sequence_For
{
 public:
   using type = typename Make_Index_Sequence<sizeof ...(Ts)>::type;
};


// ** function ptr   (2)

// ** function uses Index_Sequence Class to unpack a tuple into arguments to a function
template<typename ...FunctionArgTypes, typename FunctionReturn, typename ...TupleTypes, size_t ...Ks>
FunctionReturn cs_unpack_function_args_internal(FunctionReturn (*functionPtr)(FunctionArgTypes...),
      const std::tuple<TupleTypes...> &data, Index_Sequence<Ks...>)
{   
   return functionPtr(std::get<Ks>(data)...);
}

// (api) function pointer unpack tuple
template<typename ...FunctionArgTypes, typename FunctionReturn, typename ...TupleTypes>
FunctionReturn cs_unpack_function_args(FunctionReturn (*functionPtr)(FunctionArgTypes...),
                                       const std::tuple<TupleTypes...> &data)
{
   return cs_unpack_function_args_internal(functionPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
}

// specialization when FunctionReturn as type void, force to CSVoidReturn
template<typename ...FunctionArgTypes, typename ...TupleTypes>
CSVoidReturn cs_unpack_function_args(void (*functionPtr)(FunctionArgTypes...), const std::tuple<TupleTypes...> &data)
{
   cs_unpack_function_args_internal(functionPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
   return CSVoidReturn {};
}



// ** method pointer  (3)

// ** function uses Index_Sequence Class to unpack a tuple into arguments to a method
template<typename MethodClass, class MethodReturn, typename ...MethodArgTypes, typename ...TupleTypes, size_t ...Ks>
MethodReturn cs_unpack_method_args_internal(MethodClass *obj, MethodReturn (MethodClass::*methodPtr)(MethodArgTypes...),
      const std::tuple<TupleTypes...> &data, Index_Sequence<Ks...> dummy)
{  
   return (obj->*methodPtr)(std::get<Ks>(data)...);
}

// ** function uses Index_Sequence Class to unpack a tuple into arguments to a method  (duplicate for const)
template<typename MethodClass, class MethodReturn, typename ...MethodArgTypes, typename ...TupleTypes, size_t ...Ks>
MethodReturn cs_unpack_method_args_internal(const MethodClass *obj,
      MethodReturn (MethodClass::*methodPtr)(MethodArgTypes...) const,
      const std::tuple<TupleTypes...> &data, Index_Sequence<Ks...> dummy)
{
   return (obj->*methodPtr)(std::get<Ks>(data)...);
}

// (api) method pointer unpack tuple
template<typename MethodClass, class MethodReturn, typename ...MethodArgTypes, typename ...TupleTypes>
MethodReturn cs_unpack_method_args(MethodClass *obj, MethodReturn (MethodClass::*methodPtr)(MethodArgTypes...),
                                   const std::tuple<TupleTypes...> &data)
{
   return cs_unpack_method_args_internal(obj, methodPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
}

// specialization when MethodReturn as type void, force to CSVoidReturn
template<typename MethodClass, typename ...MethodArgTypes, typename ...TupleTypes>
CSVoidReturn cs_unpack_method_args(MethodClass *obj, void (MethodClass::*methodPtr)(MethodArgTypes...),
                                   const std::tuple<TupleTypes...> &data)
{
   cs_unpack_method_args_internal(obj, methodPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
   return CSVoidReturn {};
}

// (api) method pointer unpack tuple   (duplicate for const)
template<typename MethodClass, class MethodReturn, typename ...MethodArgTypes, typename ...TupleTypes>
MethodReturn cs_unpack_method_args(const MethodClass *obj,
                                   MethodReturn (MethodClass::*methodPtr)(MethodArgTypes...) const,
                                   const std::tuple<TupleTypes...> &data)
{
   return cs_unpack_method_args_internal(obj, methodPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
}

// specialization when MethodReturn as type void, force to CSVoidReturn  (duplicate for const)
template<typename MethodClass, typename ...MethodArgTypes, typename ...TupleTypes>
CSVoidReturn cs_unpack_method_args(const MethodClass *obj, void (MethodClass::*methodPtr)(MethodArgTypes...) const,
                                   const std::tuple<TupleTypes...> &data)
{
   cs_unpack_method_args_internal(obj, methodPtr, data, typename Index_Sequence_For<TupleTypes...>::type {} );
   return CSVoidReturn {};
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


// ***********

template <class T1, class T2>
class prePend
{
   // required dummy class to utilze a specialization
};

template <class T, class ...Ts>
class prePend<T, std::tuple<Ts...>>
{
 public:
   using type = typename std::tuple<T, Ts...>;
};

template <class T1>
class strip
{
 public:
   // contains nothing
   using type = typename std::tuple<>;
};

template <class T1, class T2, class ...Ts>
class strip<std::tuple<T1, T2, Ts...>>
{
 public:
   using type = typename prePend<T1, typename strip<std::tuple<T2, Ts...> >::type>::type;
   static type doRemove(const std::tuple<T1, T2, Ts...> &);
};

template <class ...Ts>
class removeLast
{
 public:
   using type = typename strip< std::tuple<Ts...> >::type;
   static type doRemove(const std::tuple<Ts...> &);
};


/**   \cond INTERNAL (notation so DoxyPress will not parse this class  */

// **
template<unsigned int ...Vs>
class intValues
{
 public:
   using type = intValues<Vs...>;
};

template<unsigned int Max, unsigned int ...Vs>
class makeIntValues : public makeIntValues < Max - 1, Max - 1, Vs... >
{
};

template<unsigned int...Vs>
class makeIntValues<0, Vs...> : public intValues<Vs...>
{
};

template<unsigned int ...Vs, class ...Ts>
typename removeLast<Ts...>::type newFunc(intValues<Vs...>, std::tuple<Ts...> tupleValue)
{
   return std::forward_as_tuple(std::get<Vs>(tupleValue)...);
}

template<class ...Ts>
typename removeLast<Ts...>::type funcRemove(std::tuple<Ts...> tupleValue )
{
   return newFunc(makeIntValues < sizeof...(Ts) - 1 > (), tupleValue);
}

template<class Last, class ...Ts>
std::tuple<Ts...> funcRemove(std::tuple<Ts..., Last> tupleValue, Ts...Vs )
{
   return std::forward_as_tuple(Vs...);
}


// ***********

// ** store slot data in tuple
class TeaCupAbstract
{
 public:
   virtual ~TeaCupAbstract() {}
   //    virtual QList<QVariant> toVariantList() const = 0;
};

// 1
template<class ...Ts>
class TeaCup: public TeaCup< typename removeLast<Ts...>::type >
{
 public:
   template<class T>
   explicit TeaCup(T lambda);

   std::tuple<Ts...> getData() const;

 private:
   std::function<std::tuple<Ts...> ()> m_lambda;
};

template<class ...Ts>
template<class T>
TeaCup<Ts...>::TeaCup(T lambda)
   : TeaCup< typename removeLast<Ts...>::type >( [this]()
{
   return funcRemove(m_lambda());
} ),
m_lambda {lambda} {
}

template<class ...Ts>
std::tuple<Ts...> TeaCup<Ts...>::getData() const
{
   return m_lambda();
}


// 2  specialization, empty data
template<>
class TeaCup<>: public TeaCupAbstract
{
 public:
   template<class T>
   explicit TeaCup(T lambda);

   std::tuple<> getData() const;
};

template<class T>
TeaCup<>::TeaCup(T lambda)
{
}

inline std::tuple<> TeaCup<>::getData() const
{
   // empty tuple
   return std::tuple<> {};
}


// 3  specialization, tuple
template<class ...Ts>
class TeaCup< std::tuple<Ts...> >: public TeaCup<Ts...>
{
 public:
   template<class T>
   explicit TeaCup(T lambda);
};

template<class ...Ts>
template<class T>
TeaCup<std::tuple<Ts...>>::TeaCup(T lambda) 
   : TeaCup<Ts...>(lambda)
{
}

/**   \endcond   */


// ** next two functions use Index_Sequence Class to convert a tuple to
template<class R, class T, size_t ...Ks>
R convert_tuple_internal(T &data, Index_Sequence<Ks...> dummy)
{
   return R {std::get<Ks>(data)...};
}

template<class R, class ...Ts>
R convert_tuple(std::tuple<Ts...> &data)
{
   return convert_tuple_internal<R> (data, typename Index_Sequence_For<Ts...>::type {} );
}


// ** TeaCup Class used to store data for signal activation
template<class ...Ts>
class TeaCup_Data: public TeaCup<Ts...>
{
 public:
   TeaCup_Data(bool needs_Copying, Ts...);
   std::tuple<Ts...> getData() const;
   //    virtual QList<QVariant> toVariantList() const;

 private:
   std::shared_ptr< std::tuple<typename std::remove_reference<Ts>::type...> > m_copyOfData;
   std::tuple<Ts...> m_data;
};

template<class ...Ts>
TeaCup_Data<Ts...>::TeaCup_Data(bool needs_Copying, Ts...Vs)
   : TeaCup<Ts...>( [this]()
{
   return m_data;
} ),
m_copyOfData(needs_Copying ? new std::tuple<typename std::remove_reference<Ts>::type...> (Vs...) : nullptr ),
m_data(needs_Copying ? convert_tuple<std::tuple<Ts...>> (*m_copyOfData) : std::tuple<Ts...> (Vs...) )
{
}

template<class ...Ts>
std::tuple<Ts...> TeaCup_Data<Ts...>::getData() const
{
   return m_data;
}


// ***********

// ** store method pointer for signals and slots
class BentoAbstract
{
 public:
   virtual ~BentoAbstract() {}

   virtual bool operator ==(const BentoAbstract &right) const = 0;
   bool operator !=(const BentoAbstract &right) const;
   virtual void invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval = 0) const = 0;
   virtual bool checkReturnType(CSGenericReturnArgument &retval) const = 0;
};

inline bool BentoAbstract::operator !=(const BentoAbstract &right) const
{
   return ! (*this == right);
}


template<class T>
class Bento : public BentoAbstract
{
 public:
   Bento(T ptr);
   virtual bool operator ==(const BentoAbstract &right) const;
   virtual void invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval = 0) const;
   virtual bool checkReturnType(CSGenericReturnArgument &retval) const;

   template<class MethodReturn, class ...MethodArgs>
   void invoke_internal(const TeaCupAbstract *dataPack, MethodReturn (T::*methodPtr)(MethodArgs...) const) const;

   template<class MethodReturn, class ...MethodArgs>
   void invoke_internal(const TeaCupAbstract *dataPack, MethodReturn (T::*methodPtr)(MethodArgs...)) const;

   T m_lambda;
};

template<class FunctionReturn, class ...FunctionArgs>
class Bento<FunctionReturn (*)(FunctionArgs...)> : public BentoAbstract
{
 public:
   Bento(FunctionReturn (*ptr)(FunctionArgs...));
   virtual bool operator ==(const BentoAbstract &right) const;
   virtual void invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval = 0) const;
   virtual bool checkReturnType(CSGenericReturnArgument &retval) const;

   FunctionReturn (*m_methodPtr)(FunctionArgs...);
};

template<class MethodClass, class MethodReturn, class...MethodArgs>
class Bento<MethodReturn(MethodClass::*)(MethodArgs...)>: public BentoAbstract
{
 public:
   Bento(MethodReturn(MethodClass::*ptr)(MethodArgs...) );
   virtual bool operator ==(const BentoAbstract &right) const;
   virtual void invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval = 0) const;
   virtual bool checkReturnType(CSGenericReturnArgument &retval) const;

   MethodReturn(MethodClass::*m_methodPtr)(MethodArgs...);
};

template<class MethodClass, class MethodReturn, class...MethodArgs>
class Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>: public BentoAbstract
{
   // specialization, pointer to const method

 public:
   Bento(MethodReturn(MethodClass::*ptr)(MethodArgs...) const);
   virtual bool operator ==(const BentoAbstract &right) const;
   virtual void invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval = 0) const;
   virtual bool checkReturnType(CSGenericReturnArgument &retval) const;

   MethodReturn(MethodClass::*m_methodPtr)(MethodArgs...) const;
};



// (1) lambda
template<class T>
Bento<T>::Bento(T lambda)
   : m_lambda(lambda)
{
}

template<class T>
bool Bento<T>::operator ==(const BentoAbstract &) const
{
   // can not compare two lambdas
   return false;
}

template<class T>
void Bento<T>::invoke(QObject *receiver, const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   // T must be a class, will be a compiler error otherwise
   auto methodPtr = &T::operator();

   this->invoke_internal(dataPack, methodPtr);
}

template<class T>
bool Bento<T>::checkReturnType(CSGenericReturnArgument &retval) const
{
   // return type of a lambda is not required

   if (dynamic_cast<CSReturnArgument<void> *>(&retval)) {
      // return argument is void
      return true;
   }

   return false;
}

template<class T>
template<class MethodReturn, class ...MethodArgs>
void Bento<T>::invoke_internal(const TeaCupAbstract *dataPack, MethodReturn (T::*methodPtr)(MethodArgs...) const) const
{
   // handles non-mutable, captures variables are const

   // dynamic cast will return a valid ptr if the slot has equal or less parameters
   // retrieve ptr to teaCup object, which contains the data
   const TeaCup<MethodArgs...> *teaCup = dynamic_cast<const TeaCup<MethodArgs...> *>(dataPack);

   if (teaCup) {
      // expand arguments
      std::tuple<MethodArgs...> args = teaCup->getData();

      // unpacks the tuple, then calls the method or slot
      cs_unpack_method_args(&m_lambda, methodPtr, args);
   }
}

template<class T>
template<class MethodReturn, class ...MethodArgs>
void Bento<T>::invoke_internal(const TeaCupAbstract *dataPack, MethodReturn (T::*methodPtr)(MethodArgs...)) const
{
   // handles mutable, captures variables are non-const

   // dynamic cast will return a valid ptr if the slot has equal or less parameters
   // retrieve ptr to teaCup object, which contains the data
   const TeaCup<MethodArgs...> *teaCup = dynamic_cast<const TeaCup<MethodArgs...> *>(dataPack);

   if (teaCup) {
      // expand arguments
      std::tuple<MethodArgs...> args = teaCup->getData();

      // prep m_lambda
      auto object = const_cast<typename std::remove_const<T>::type *>(&m_lambda);

      // unpacks the tuple, then calls the method or slot
      cs_unpack_method_args(object, methodPtr, args);
   }
}


// (2) specialization, function pointer
template<class FunctionReturn, class ...FunctionArgs>
Bento<FunctionReturn (*)(FunctionArgs...)>::Bento(FunctionReturn (*ptr)(FunctionArgs...)) :
   m_methodPtr(ptr)
{
}

template<class FunctionReturn, class ...FunctionArgs>
bool Bento<FunctionReturn (*)(FunctionArgs...)>::operator ==(const BentoAbstract &right) const
{
   bool retval = false;

   const Bento<FunctionReturn (*)(FunctionArgs...)> *temp;
   temp = dynamic_cast<const Bento <FunctionReturn (*)(FunctionArgs...)> *> (&right);

   if (temp) {
      retval = (this->m_methodPtr == temp->m_methodPtr);
   }

   return retval;
}

template<class FunctionReturn, class ...FunctionArgs>
void Bento<FunctionReturn (*)(FunctionArgs...)>::invoke(QObject *receiver, const TeaCupAbstract *dataPack,
      CSGenericReturnArgument *retval) const
{
   // no need to verify receiver since it is not used

   // dynamic cast will return a valid ptr if the slot has equal or less parameters
   // retrieve ptr to teaCup object, which contains the data
   const TeaCup<FunctionArgs...> *teaCup = dynamic_cast<const TeaCup<FunctionArgs...> *>(dataPack);

   if (teaCup) {
      // expand arguments
      std::tuple<FunctionArgs...> args = teaCup->getData();

      CSReturnArgument<FunctionReturn> *returnData = dynamic_cast<CSReturnArgument<FunctionReturn> *>(retval);

      if (returnData) {
         // unpack the tuple, then call the method or slot
         returnData->setData(cs_unpack_function_args(m_methodPtr, args));

      } else {
         // returnData is a null pointer, therefore retval was null or the wrong type
         // assume user does not want a return value

         // unpack the tuple, then call the method or slot
         cs_unpack_function_args(m_methodPtr, args);
      }
   }

}

template<class FunctionReturn, class ...FunctionArgs>
bool Bento<FunctionReturn (*)(FunctionArgs...)>::checkReturnType(CSGenericReturnArgument &retval) const
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
Bento<MethodReturn(MethodClass::*)(MethodArgs...)>::Bento(MethodReturn(MethodClass::*ptr)(MethodArgs...)) :
   m_methodPtr(ptr)
{
}

template<class MethodClass, class MethodReturn, class...MethodArgs>
bool Bento<MethodReturn(MethodClass::*)(MethodArgs...)>::operator ==(const BentoAbstract &right) const
{
   bool retval = false;

   const Bento<MethodReturn(MethodClass::*)(MethodArgs...)> *temp;
   temp = dynamic_cast<const Bento <MethodReturn(MethodClass::*)(MethodArgs...)> *> (&right);

   if (temp) {
      retval = (this->m_methodPtr == temp->m_methodPtr);
   }

   return retval;
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
void Bento<MethodReturn(MethodClass::*)(MethodArgs...)>::invoke(QObject *receiver,
      const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   if (! receiver)  {
      return;
   }

   MethodClass *t_receiver = dynamic_cast<MethodClass *>(receiver);

   if (t_receiver) {
      // dynamic cast will return a valid ptr if the slot has equal or less parameters
      // retrieve ptr to teaCup object, which contains the data
      const TeaCup<MethodArgs...> *teaCup = dynamic_cast<const TeaCup<MethodArgs...> *>(dataPack);

      if (teaCup) {
         // expand arguments
         std::tuple<MethodArgs...> args = teaCup->getData();

         CSReturnArgument<MethodReturn> *returnData = dynamic_cast<CSReturnArgument<MethodReturn> *>(retval);

         if (returnData) {
            // unpacks the tuple, then calls the method or slot
            returnData->setData(cs_unpack_method_args(t_receiver, m_methodPtr, args));

         } else {
            // returnData is a null pointer, therefore retval was null or the wrong type
            // assume user does not want a return value

            // unpacks the tuple, then calls the method or slot
            cs_unpack_method_args(t_receiver, m_methodPtr, args);

         }

      }
   }
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
bool Bento<MethodReturn(MethodClass::*)(MethodArgs...)>::checkReturnType(CSGenericReturnArgument &retval) const
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
Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>::Bento(MethodReturn(MethodClass::*ptr)(MethodArgs...) const) :
   m_methodPtr(ptr)
{
}

template<class MethodClass, class MethodReturn, class...MethodArgs>
bool Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>::operator ==(const BentoAbstract &right) const
{
   bool retval = false;

   const Bento<MethodReturn(MethodClass::*)(MethodArgs...) const> *temp;
   temp = dynamic_cast<const Bento <MethodReturn(MethodClass::*)(MethodArgs...) const> *> (&right);

   if (temp) {
      retval = (this->m_methodPtr == temp->m_methodPtr);
   }

   return retval;
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
void Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>::invoke(QObject *receiver,
      const TeaCupAbstract *dataPack, CSGenericReturnArgument *retval) const
{
   if (! receiver)  {
      return;
   }

   MethodClass *t_receiver = dynamic_cast<MethodClass *>(receiver);

   if (t_receiver) {
      // dynamic cast will return a valid ptr if the slot has equal or less parameters

      // retrieve ptr to teaCup object, which contains the data
      const TeaCup<MethodArgs...> *teaCup = dynamic_cast<const TeaCup<MethodArgs...> *>(dataPack);

      if (teaCup) {
         // expand arguments
         std::tuple<MethodArgs...> args = teaCup->getData();

         CSReturnArgument<MethodReturn> *returnData = dynamic_cast<CSReturnArgument<MethodReturn> *>(retval);

         if (returnData) {
            // unpacks the tuple, then calls the method or slot
            returnData->setData(cs_unpack_method_args(t_receiver, m_methodPtr, args));

         } else {
            // returnData is a null pointer, therefore retval was null or the wrong type
            // assume user does not want a return value

            // unpacks the tuple, then calls the method or slot
            cs_unpack_method_args(t_receiver, m_methodPtr, args);

         }

      }
   }
}

template<class MethodClass, class MethodReturn, class ...MethodArgs>
bool Bento<MethodReturn(MethodClass::*)(MethodArgs...) const>::checkReturnType(CSGenericReturnArgument &retval) const
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

