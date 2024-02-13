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

#ifndef QCORE_MAC_P_H
#define QCORE_MAC_P_H

#include <qglobal.h>
#include <qstring.h>

#ifndef __IMAGECAPTURE__
#  define __IMAGECAPTURE__
#endif

#include <CoreFoundation/CoreFoundation.h>

#ifdef __OBJC__
#include <Foundation/Foundation.h>
#endif

template <typename T>
class Q_CORE_EXPORT QCFType
{
 public:
   QCFType(const T &t = nullptr)
      : m_type(t)
   {
   }

   QCFType(const QCFType &helper)
      : m_type(helper.m_type)
   {
      if (m_type) {
         CFRetain(m_type);
      }
   }

   ~QCFType() {
      if (m_type) {
         CFRelease(m_type);
      }
   }

   operator T() {
      return m_type;
   }

   QCFType operator =(const QCFType &helper) {
      if (helper.m_type) {
         CFRetain(helper.m_type);
      }

      CFTypeRef refType = m_type;
      m_type = helper.m_type;

      if (refType) {
         CFRelease(refType);
      }

      return *this;
   }

   T *operator &() {
      return &m_type;
   }

   template <typename U>
   U as() const {
      return reinterpret_cast<U>(m_type);
   }

   static QCFType constructFromGet(const T &t) {
      CFRetain(t);
      return QCFType<T>(t);
   }

 protected:
   T m_type;
};

class Q_CORE_EXPORT QCFString : public QCFType<CFStringRef>
{
 public:
   QCFString(const QString &str)
      : QCFType<CFStringRef>(nullptr), m_string(str)
   {
   }

   QCFString(const CFStringRef cfstr = nullptr)
      : QCFType<CFStringRef>(cfstr)
   {
   }

   QCFString(const QCFType<CFStringRef> &other)
      : QCFType<CFStringRef>(other)
   {
   }

   QString toQString() const;
   CFStringRef toCFStringRef() const;

   operator CFStringRef() = delete;

   static QString toQString(CFStringRef cfstr);
   static CFStringRef toCFStringRef(const QString &str);

#ifdef __OBJC__
   static QString toQString(const NSString *nsstr);
   static NSString *toNSString(const QString &string);
#endif

 private:
   QString m_string;
};

struct QAppleOperatingSystemVersion {
   int major;
   int minor;
   int patch;
};

QAppleOperatingSystemVersion qt_apple_os_version();

#if defined(Q_OS_DARWIN)
Q_CORE_EXPORT QChar qt_mac_qtKey2CocoaKey(Qt::Key key);
Q_CORE_EXPORT Qt::Key qt_mac_cocoaKey2QtKey(QChar keyCode);
#endif

#endif
