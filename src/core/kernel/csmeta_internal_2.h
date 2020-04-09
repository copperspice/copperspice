/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef CSMETA_INTERNAL_2_H
#define CSMETA_INTERNAL_2_H

#include <qvariant.h>

template<class T>
QVariant cs_convertToQVariant(T data)
{
   if constexpr (cs_is_enum_or_flag<T>::value) {
      // used to avoid implicitly converting an enum to an int
      return QVariant::fromValue(data);

   } else if constexpr (std::is_constructible<QVariant, T>::value) {
      return QVariant(data);

   } else {
     return QVariant::fromValue(data);

   }
}

template<class T>
std::pair<T, bool> convertFromQVariant(QVariant data)
{
   if constexpr (! cs_is_enum_or_flag<T>::value) {
      // T is not an enum or a flag, T is a builtin type
      return std::make_pair(data.value<T>(), true);

   } else {
      // T is an enum or a flag
      using intType = typename cs_underlying_type<T>::type;

      intType retval = 0;
      bool ok = true;

      QVariant::Type dataType = data.type();

      if (dataType == QVariant::Int  || dataType == QVariant::LongLong ||
            dataType == QVariant::UInt || dataType == QVariant::ULongLong) {

         // supported integer types
         retval = data.value<intType>();

      } else if (dataType == QVariant::String) {
         // enum or flag

         QMetaEnum obj = QMetaObject::findEnum<T>();

         if (obj.isValid()) {

            if (obj.isFlag()) {
               retval = obj.keysToValue(data.toString());
            } else {
               retval = obj.keyToValue(data.toString());
            }

         } else {
            ok = false;

         }

      } else {
         // not a string or an int
         std::optional<T> tmp;

         if (tmp.has_value()) {
            // emerald: replace with "tmp.value()" when OS X 10.13 is dropped
            retval = *tmp;

         } else  {
            // unable to convert, type mismatch
            ok = false;
         }
      }

      return std::make_pair(static_cast<T>(retval), ok);
   }
}

// classes for these 2 methods, located in csmeta.h around line 405
template<class E>
const QString &CS_ReturnType<E, typename std::enable_if<std::is_enum<E>::value>::type>::getName()
{
   static QMetaEnum obj = QMetaObject::findEnum<E>();

   if (obj.isValid()) {
      static QString tmp = obj.scope() + "::" + obj.name();
      return tmp;

   } else {
      static QString retval("Unknown_Enum");
      return retval;

   }
}

template<class E>
const QString &CS_ReturnType<QFlags<E> >::getName()
{
   static QMetaEnum obj = QMetaObject::findEnum<QFlags<E>>();

   if (obj.isValid()) {
      static QString tmp = obj.scope() + "::" + obj.name();
      return tmp;

   } else {
      static QString retval("Unknown_Enum");
      return retval;
   }
}

template<class T>
void cs_namespace_register_enum(const char *name, std::type_index id, const char *scope)
{
   const_cast<QMetaObject_T<T>&>(T::staticMetaObject()).register_enum(QString::fromUtf8(name), id, QString::fromUtf8(scope));
}

// ** flags
template<class T>
void cs_namespace_register_flag(const char *enumName, const char *scope, const char *flagName, std::type_index id)
{
   const_cast<QMetaObject_T<T>&> (T::staticMetaObject()).register_flag(QString::fromUtf8(enumName),
                  QString::fromUtf8(scope), QString::fromUtf8(flagName), id);
}

#endif
