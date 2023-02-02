/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qbytearray.h>
#include <qdate.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qstring8.h>
#include <qstring16.h>
#include <qtime.h>

#define CATCH_CONFIG_EXPERIMENTAL_REDIRECT

#include <catch2/catch.hpp>

inline std::unique_ptr<QCoreApplication> initCoreApp()
{
   int argc    = 0;
   auto retval = std::make_unique<QCoreApplication>(argc, nullptr);

   return retval;
}

namespace Catch {

   template <>
   struct StringMaker<QByteArray> {
      static std::string convert(const QByteArray &value) {
         return value.constData();
      }
   };

   template <>
   struct StringMaker<QDate> {
      static std::string convert(const QDate &value) {
         return value.toString().toStdString();
      }
   };

   template <>
   struct StringMaker<QLocale> {
      static std::string convert(const QLocale &value) {
         return value.name().toStdString();
      }
   };

   template <>
   struct StringMaker<QString8> {
      static std::string convert(const QString8 &value) {
         return value.toStdString();
      }
   };

   template <>
   struct StringMaker<QString16> {
      static std::string convert(const QString16 &value) {
         return QString::fromUtf16(value).toStdString();
      }
   };

   template <>
   struct StringMaker<QStringView> {
      static std::string convert(const QStringView &value) {
         return value.toString().toStdString();
      }
   };

   template <>
   struct StringMaker<QMargins> {
      static std::string convert(const QMargins &value) {
         QString retval = QString8("%1 %2 %3 %4")
               .formatArgs(value.left(), value.top(), value.right(), value.bottom());
         return retval.toStdString();
      }
   };

   template <>
   struct StringMaker<QTime> {
      static std::string convert(const QTime &value) {
         return value.toString().toStdString();
      }
   };
}
