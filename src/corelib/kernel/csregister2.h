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

#ifndef CSREGISTER2_H
#define CSREGISTER2_H

// ** decipher a method's return type
template<class T>
class cs_returnType;

template<class T, class R>
class cs_returnType<R (T::*)() const>
{
 public:
   using type = R;
};

template<class T>
class cs_argType;

template<class T, class R, class V>
class cs_argType<R (T::*)(V)>
{
 public:
   using type = V;
};


// ** classInfo
template<class T, class name, class data>
class cs_record_classInfo
{
 public:
   static const int placeholder;
};

template<class T, class name, class data>
int record_classInfo()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_classInfo(name::value, data::value);
   return 0;
}

template<class T, class name, class data>
const int cs_record_classInfo<T, name, data>::placeholder = record_classInfo<T, name, data>();



// ** enum

/* code moved to csregister1.h due to declaration order issues

	template<class T, class E, class name, class scope>
	class cs_record_enum
	{
	   public:
	      static const int placeholder;
	};
*/

template<class T, class E, class name, class scope>
const int cs_record_enum<T, E, name, scope>::placeholder = const_cast<QMetaObject_T<T>&>(T::staticMetaObject())
      .register_enum(name::value, typeid (E), scope::value);


// ** flag

/* code moved to csregister1.h due to declaration order issues

	template<class T, class F, class enumName, class scope, class flagName>
	class cs_record_flag
	{
	   public:
	      static const int placeholder;
	};
*/

template<class T, class F, class enumName, class scope, class flagName>
const int cs_record_flag<T, F, enumName, scope, flagName>::placeholder = const_cast<QMetaObject_T<T>&>
      (T::staticMetaObject())
      . register_flag(enumName::value, scope::value, flagName::value, typeid (F));



// ** enum data
template<class T, class args, class scope>
class cs_record_enum_data
{
 public:
   static const int placeholder;
};

template<class T, class args, class scope>
int record_enum_data()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_enum_data(args::value, scope::value);
   return 0;
}

template<class T, class args, class scope>
const int cs_record_enum_data<T, args, scope>::placeholder = record_enum_data<T, args, scope>();



// ** methods
template<class T, class signature, QMetaMethod::Access scope, QMetaMethod::MethodType kind>
class cs_record_method
{
 public:
   static const int placeholder;
};

template<class T, class signature, QMetaMethod::Access scope, QMetaMethod::MethodType kind>
int record_method()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_method(signature::value, scope, kind);
   return 0;
}

template<class T, class signature, QMetaMethod::Access scope, QMetaMethod::MethodType kind>
const int cs_record_method<T, signature, scope, kind>::placeholder = record_method<T, signature, scope, kind>();


// ** revision
template<class T, class U, U method, int revision>
class cs_record_method_rev
{
 public:
   static const int placeholder;
};

template<class T, class U, U method, int revision>
int record_method_rev()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_method_rev(method, revision);
   return 0;
}

template<class T, class U, U method, int revision>
const int cs_record_method_rev<T, U, method, revision>::placeholder = record_method_rev<T, U, method, revision>();



// ** signals & slots
template<class T, class name, class U, U method, QMetaMethod::MethodType kind>
class cs_record_method_ptr
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaMethod::MethodType kind>
int record_method_ptr()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_method_ptr(name::value, method, kind);
   return 0;
}

template<class T, class name, class U, U method, QMetaMethod::MethodType kind>
const int cs_record_method_ptr<T, name, U, method, kind>::placeholder = record_method_ptr<T, name, U, method, kind>();



// ** metaMetod tag
template<class T, class name, class data>
class cs_record_tag
{
 public:
   static const int placeholder;
};

template<class T, class name, class data>
int record_tag()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_tag(name::value, data::value);
   return 0;
}

template<class T, class name, class data>
const int cs_record_tag<T, name, data>::placeholder = record_tag<T, name, data>();


// ** properties
template<class T, class name, class U, U method, QMetaProperty::Kind K>
class cs_record_property_read
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaProperty::Kind K>
const int cs_record_property_read<T, name, U, method, K>::placeholder = const_cast<QMetaObject_T<T>&>
      (T::staticMetaObject())
      .register_property_read(name::value, cs_typeName<typename cs_returnType<U>::type> (),
                              new SpiceJarRead<T, typename cs_returnType<U>::type> (method));



// **
template<class T, class name, class U, U method, QMetaProperty::Kind K>
class cs_record_property_write
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaProperty::Kind K>
const int cs_record_property_write<T, name, U, method, K>::placeholder =
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject())
   .register_property_write(name::value, new SpiceJarWrite<T, typename cs_argType<U>::type> (method));


// **
template<class T, class name, class U, U method, QMetaProperty::Kind K>
class cs_record_property_reset
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaProperty::Kind K>
int record_property_reset()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_property_reset(name::value, method);
   return 0;
}

template<class T, class name, class U, U method, QMetaProperty::Kind K>
const int cs_record_property_reset<T, name, U, method, K>::placeholder = record_property_reset<T, name, U, method, K>();


// **
template<class T, class name, class U, U method, QMetaProperty::Kind K>
class cs_record_property_notify
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaProperty::Kind K>
int record_property_notify()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_property_notify(name::value, method);
   return 0;
}

template<class T, class name, class U, U method, QMetaProperty::Kind K>
const int cs_record_property_notify<T, name, U, method, K>::placeholder =
   record_property_notify<T, name, U, method, K>();


// **
template<class T, class name, class U, U method, QMetaProperty::Kind K>
class cs_record_property_bool
{
 public:
   static const int placeholder;
};

template<class T, class name, class U, U method, QMetaProperty::Kind K>
const int cs_record_property_bool<T, name, U, method, K>::placeholder =
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject())
   .register_property_bool(name::value, new SpiceJarRead<T, bool> (method), K);


// **
template<class T, class name, int data, QMetaProperty::Kind K>
class cs_record_property_int
{
 public:
   static const int placeholder;
};

template<class T, class name, int data, QMetaProperty::Kind K>
int record_property_int()
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_property_int(name::value, data, K);
   return 0;
}

template<class T, class name, int data, QMetaProperty::Kind K>
const int cs_record_property_int<T, name, data, K>::placeholder = record_property_int<T, name, data, K>();


#endif
