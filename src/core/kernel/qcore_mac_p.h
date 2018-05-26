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

#include <CoreFoundation/CoreFoundation.h>
#include <qglobal.h>

#ifdef __OBJC__
#include <Foundation/Foundation.h>
#endif

#include <qstring.h>

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

   template <typename X>
   X as() const {
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

#ifdef __OBJC__
    static QString toQString(const NSString *nsstr);
    static  NSString *toNSString(const QString &string);
#endif

 private:
   QString string;
};

typedef struct {
    int major, minor, patch;
} QAppleOperatingSystemVersion;

QAppleOperatingSystemVersion qt_apple_os_version();

#if defined(Q_OS_DARWIN) && ! defined(Q_OS_IOS)
Q_CORE_EXPORT QChar qt_mac_qtKey2CocoaKey(Qt::Key key);
Q_CORE_EXPORT Qt::Key qt_mac_cocoaKey2QtKey(QChar keyCode);
#endif


#endif
