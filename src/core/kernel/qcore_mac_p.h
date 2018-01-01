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

#ifndef QCORE_MAC_P_H
#define QCORE_MAC_P_H

#ifndef __IMAGECAPTURE__
#  define __IMAGECAPTURE__
#endif

#undef OLD_DEBUG
#ifdef DEBUG
# define OLD_DEBUG DEBUG
# undef DEBUG
#endif
#define DEBUG 0
#ifdef qDebug
#  define old_qDebug qDebug
#  undef qDebug
#endif

#include <CoreFoundation/CoreFoundation.h>

#if !defined(Q_OS_IOS)
#include <CoreServices/CoreServices.h>
#endif

#undef DEBUG
#ifdef OLD_DEBUG
#  define DEBUG OLD_DEBUG
#  undef OLD_DEBUG
#endif

#ifdef old_qDebug
#  undef qDebug
#  define qDebug QT_NO_QDEBUG_MACRO
#  undef old_qDebug
#endif

#include <qstring.h>

QT_BEGIN_NAMESPACE

/*
    Helper class that automates refernce counting for CFtypes.
    After constructing the QCFType object, it can be copied like a
    value-based type.

    Note that you must own the object you are wrapping.
    This is typically the case if you get the object from a Core
    Foundation function with the word "Create" or "Copy" in it. If
    you got the object from a "Get" function, either retain it or use
    constructFromGet(). One exception to this rule is the
    HIThemeGet*Shape functions, which in reality are "Copy" functions.
*/
template <typename T>
class Q_CORE_EXPORT QCFType
{
 public:
   inline QCFType(const T &t = 0) : type(t) {}
   inline QCFType(const QCFType &helper) : type(helper.type) {
      if (type) {
         CFRetain(type);
      }
   }
   inline ~QCFType() {
      if (type) {
         CFRelease(type);
      }
   }
   inline operator T() {
      return type;
   }
   inline QCFType operator =(const QCFType &helper) {
      if (helper.type) {
         CFRetain(helper.type);
      }
      CFTypeRef type2 = type;
      type = helper.type;
      if (type2) {
         CFRelease(type2);
      }
      return *this;
   }
   inline T *operator&() {
      return &type;
   }
   template <typename X> X as() const {
      return reinterpret_cast<X>(type);
   }
   static QCFType constructFromGet(const T &t) {
      CFRetain(t);
      return QCFType<T>(t);
   }
 protected:
   T type;
};

class Q_CORE_EXPORT QCFString : public QCFType<CFStringRef>
{
 public:
   inline QCFString(const QString &str) : QCFType<CFStringRef>(0), string(str) {}
   inline QCFString(const CFStringRef cfstr = 0) : QCFType<CFStringRef>(cfstr) {}
   inline QCFString(const QCFType<CFStringRef> &other) : QCFType<CFStringRef>(other) {}
   operator QString() const;
   operator CFStringRef() const;
   static QString toQString(CFStringRef cfstr);
   static CFStringRef toCFStringRef(const QString &str);

 private:
   QString string;
};


#if ! defined(Q_OS_IOS)
Q_CORE_EXPORT void qt_mac_to_pascal_string(const QString &s, Str255 str, TextEncoding encoding = 0, int len = -1);
Q_CORE_EXPORT QString qt_mac_from_pascal_string(const Str255 pstr);

Q_CORE_EXPORT OSErr qt_mac_create_fsref(const QString &file, FSRef *fsref);

// Don't use this function, it will not work in 10.5 (Leopard) and up
Q_CORE_EXPORT OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec);
#endif

QT_END_NAMESPACE

#endif // QCORE_MAC_P_H
