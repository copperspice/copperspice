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

#ifndef CSMETA_INTERNAL_2_H
#define CSMETA_INTERNAL_2_H

#include <QVariant>

// **
// template<class T, class=void, class=typename std::enable_if<!std::is_constructible<QVariant, T>::value>::type>
template<class T, class unused_1, class unused_2>
QVariant cs_convertToQVariant(T)
{
   return QVariant();
}

// template<class T, class=typename std::enable_if<std::is_constructible<QVariant, T>::value>::type>
template<class T, class unused_1>
QVariant cs_convertToQVariant(T data)
{
   return QVariant(data);
}


// **
// template<class T, class=void, class=void, class=typename std::enable_if< (! is_enum_or_flag<T>::value) && ! QMetaTypeId2<T>::Defined>::type>
template<class T, class unused_1, class unused_2, class unused_3>
std::pair<T, bool> convertFromQVariant(QVariant)
{
   // T is not an enum, flag, or built in data type
   return std::make_pair(T {}, false);
}

// template<class T, class=void, class=typename std::enable_if< (! is_enum_or_flag<T>::value) && QMetaTypeId2<T>::Defined>::type>
template<class T, class unused_1, class unused_2>
std::pair<T, bool> convertFromQVariant(QVariant data)
{
   // T is not an enum, not a flag   T is only a built in data type
   return std::make_pair(data.value<T>(), true);
}

// template<class T, class=typename std::enable_if<is_enum_or_flag<T>::value>::type>
template<class T, class usused_1>
std::pair<T, bool> convertFromQVariant(QVariant data)
{
   // T is an enum or a flag
   using intType = typename cs_underlying_type<T>::type;

   intType temp = 0;
   bool retval  = true;

   QVariant::Type dataType = data.type();

   if (dataType == QVariant::Int  || dataType == QVariant::LongLong ||
         dataType == QVariant::UInt || dataType == QVariant::ULongLong) {

      // supported integer types
      temp = data.value<intType>();

   } else if (dataType == QVariant::String)   {
      // enum or flag

      QMetaEnum obj = QMetaObject::findEnum<T>();

      if (obj.isValid()) {

         if (obj.isFlag()) {
            temp = obj.keysToValue( data.toString().toLatin1().constData() );
         } else {
            temp = obj.keyToValue( data.toString().toLatin1().constData() );
         }

      } else {
         retval = false;

      }

   } else {
      // not a string or an int
      int enumMetaTypeId = qMetaTypeId_Query<T>();

      if ((enumMetaTypeId == 0) || (data.userType() != enumMetaTypeId) || ! data.constData())  {
         // unable to convert, type mismatch
         retval = false;

      } else  {
         temp = *reinterpret_cast<const intType *>(data.constData()) ;

      }
   }

   return std::make_pair( static_cast<T>(temp), retval);
}

// classes for these 2 methods, located in csmeta.h around line 330
template<class E>
inline const char *cs_typeName_internal<E, typename std::enable_if<std::is_enum<E>::value>::type  >::typeName()
{
   static QMetaEnum obj = QMetaObject::findEnum<E>();

   if (obj.isValid()) {
      static QByteArray temp = obj.scope() + QByteArray("::") + obj.name();
      return temp.constData();

   } else {
      return "Unknown_Enum";

   }
}

template<class E>
inline const char *cs_typeName_internal< QFlags<E> >::typeName()
{
   static QMetaEnum obj = QMetaObject::findEnum<QFlags<E>>();

   if (obj.isValid()) {
      static QByteArray temp = obj.scope() + QByteArray("::") + obj.name();
      return temp.constData();

   } else {
      return "Unknown_Enum";

   }
}

template<class T>
void cs_namespace_register_enum(const char *name, std::type_index id, const char *scope)
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_enum(name, id, scope);
}

  
// ** flags
template<class T>
void cs_namespace_register_flag(const char *enumName, const char *scope, const char *flagName, std::type_index id)
{
   const_cast<QMetaObject_T<T>&> (T::staticMetaObject()).register_flag(enumName, scope, flagName, id); 
}

#endif
