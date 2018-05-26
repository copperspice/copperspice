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

#ifndef QDEBUG_H
#define QDEBUG_H

#include <qcontiguouscache.h>
#include <qhash.h>
#include <qlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qset.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvector.h>

class Q_CORE_EXPORT QDebug
{
   struct Stream {
      Stream(QIODevice *device)
         : ts(device), ref(1), type(QtDebugMsg), space(true), message_output(false)
      {}

      Stream(QString *string)
         : ts(string, QIODevice::WriteOnly), ref(1), type(QtDebugMsg), space(true), message_output(false)
      {}

      Stream(QtMsgType t)
         : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true), message_output(true)
      {}

      QTextStream ts;
      QString buffer;
      int ref;
      QtMsgType type;
      bool space;
      bool message_output;
   } *stream;

 public:
   QDebug(QIODevice *device)
      : stream(new Stream(device)) {}

   QDebug(QString *string)
      : stream(new Stream(string)) {}

   QDebug(QtMsgType t)
      : stream(new Stream(t)) {}

   QDebug(const QDebug &other): stream(other.stream) {
      ++stream->ref;
   }

   inline QDebug &operator=(const QDebug &other);

   ~QDebug() {
      if (! --stream->ref) {
         if (stream->message_output) {

            try {

#if QDEBUG_USE_LOCAL_ENCODING
               qt_message_output(stream->type, stream->buffer.toLocal8Bit().constData());
#else
               qt_message_output(stream->type, stream->buffer.toLatin1().constData());
#endif

            } catch (std::bad_alloc &) {
               // out of memory, give up
            }

         }

         delete stream;
      }
   }

   inline QDebug &space() {
      stream->space = true;
      stream->ts << ' ';
      return *this;
   }

   inline QDebug &nospace() {
      stream->space = false;
      return *this;
   }

   inline QDebug &maybeSpace() {
      if (stream->space) {
         stream->ts << ' ';
      }
      return *this;
   }

   inline QDebug &operator<<(bool t) {
      stream->ts << (t ? "true" : "false");
      return maybeSpace();
   }

   inline QDebug &operator<<(char c) {
      stream->ts << c;
      return maybeSpace();
   }

   inline QDebug &operator<<(signed short t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(unsigned short t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(signed int t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(unsigned int t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(signed long t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(unsigned long t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(qint64 t) {
      stream->ts << QString::number(t);
      return maybeSpace();
   }

   inline QDebug &operator<<(quint64 t) {
      stream->ts << QString::number(t);
      return maybeSpace();
   }

   inline QDebug &operator<<(float t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(double t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(const char *c) {
      stream->ts << QString::fromLatin1(c);
      return maybeSpace();
   }

   inline QDebug &operator<<(QChar c) {
      stream->ts << '\'' << c << '\'';
      return maybeSpace();
   }

   inline QDebug &operator<<(const QString &str) {
      stream->ts << '\"' << str << '\"';
      return maybeSpace();
   }

   inline QDebug &operator<<(QStringView str) {
      return operator<<(QString(str));
   }

   inline QDebug &operator<<(const QByteArray &str) {
      stream->ts  << '\"' << str << '\"';
      return maybeSpace();
   }

   inline QDebug &operator<<(const void *t) {
      stream->ts << t;
      return maybeSpace();
   }

   inline QDebug &operator<<(QTextStreamFunction f) {
      stream->ts << f;
      return *this;
   }

   inline QDebug &operator<<(QTextStreamManipulator m) {
      stream->ts << m;
      return *this;
   }
};

class QNoDebug
{
 public:
   inline QNoDebug() {}
   inline QNoDebug(const QDebug &) {}
   inline ~QNoDebug() {}

   inline QNoDebug &operator<<(QTextStreamFunction) {
      return *this;
   }
   inline QNoDebug &operator<<(QTextStreamManipulator) {
      return *this;
   }

   inline QNoDebug &space() {
      return *this;
   }
   inline QNoDebug &nospace() {
      return *this;
   }
   inline QNoDebug &maybeSpace() {
      return *this;
   }

   template<typename T>
   inline QNoDebug &operator<<(const T &) {
      return *this;
   }
};

inline QDebug &QDebug::operator=(const QDebug &other)
{
   if (this != &other) {
      QDebug copy(other);
      qSwap(stream, copy.stream);
   }

   return *this;
}

template <class T>
inline QDebug operator<<(QDebug debug, const QList<T> &list)
{
   debug.nospace() << '(';

   for (typename QList<T>::size_type i = 0; i < list.count(); ++i) {
      if (i) {
         debug << ", ";
      }
      debug << list.at(i);
   }

   debug << ')';
   return debug.space();
}

template <typename T>
inline QDebug operator<<(QDebug debug, const QVector<T> &vector)
{
   debug.nospace() << "QVector";
   return operator<<(debug, vector.toList());
}

template <class Key, class Val, class C>
inline QDebug operator<<(QDebug debug, const QMap<Key, Val, C> &map)
{
   debug.nospace() << "QMap(";

   for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   return debug.space();
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
inline QDebug operator<<(QDebug debug, const QHash<Key, Val, Hash, KeyEqual> &hash)
{
   debug.nospace() << "QHash(";

   for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   return debug.space();
}

template <class Key, class Val, class C>
inline QDebug operator<<(QDebug debug, const QMultiMap<Key, Val, C> &map)
{
   debug.nospace() << "QMultiMap(";

   for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   return debug.space();
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
inline QDebug operator<<(QDebug debug, const QMultiHash<Key, Val, Hash, KeyEqual> &hash)
{
   debug.nospace() << "QMultiHash(";

   for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   return debug.space();
}

template <class T1, class T2>
inline QDebug operator<<(QDebug debug, const QPair<T1, T2> &pair)
{
   debug.nospace() << "QPair(" << pair.first << ',' << pair.second << ')';
   return debug.space();
}

template <typename T>
inline QDebug operator<<(QDebug debug, const QSet<T> &set)
{
   debug.nospace() << "QSet";
   return operator<<(debug, set.toList());
}

template <class T>
inline QDebug operator<<(QDebug debug, const QContiguousCache<T> &cache)
{
   debug.nospace() << "QContiguousCache(";

   for (int i = cache.firstIndex(); i <= cache.lastIndex(); ++i) {
      debug << cache[i];
      if (i != cache.lastIndex()) {
         debug << ", ";
      }
   }

   debug << ')';
   return debug.space();
}

template <class T>
inline QDebug operator<<(QDebug debug, const QFlags<T> &flags)
{
   debug.nospace() << "QFlags(";
   bool needSeparator = false;

   for (uint i = 0; i < sizeof(T) * 8; ++i) {

      if (flags.testFlag(T(1 << i))) {
         if (needSeparator) {
            debug.nospace() << '|';
         } else {
            needSeparator = true;
         }

         debug.nospace() << "0x" << QByteArray::number(T(1 << i), 16).constData();
      }
   }

   debug << ')';
   return debug.space();
}

inline QDebug qDebug()
{
   return QDebug(QtDebugMsg);
}

inline QDebug qCritical()
{
   return QDebug(QtCriticalMsg);
}

inline QDebug qWarning()
{
   return QDebug(QtWarningMsg);
}


#ifdef Q_OS_DARWIN

// provide QDebug stream operators for commonly used Core Foundation
// and Core Graphics types, as well as NSObject. Additional CF/CG types
// may be added by the user, using Q_DECLARE_QDEBUG_OPERATOR_FOR_CF_TYPE.

#ifdef __OBJC__
   @class NSObject;
#else
   using NSObject = struct objc_object;
#endif

using CFStringRef     = const struct __CFString *;
using CFErrorRef      = struct __CFError *;
using CFBundleRef     = struct __CFBundle *;

using CGPathRef       = const struct CGPath *;

using CFArrayRef      = const struct __CFArray *;
using CFURLRef        = const struct __CFURL *;
using CFDataRef       = const struct __CFData *;
using CFNumberRef     = const struct __CFNumber *;
using CFDictionaryRef = const struct __CFDictionary *;
using CFLocaleRef     = const struct __CFLocale *;
using CFDateRef       = const struct __CFDate *;
using CFBooleanRef    = const struct __CFBoolean *;
using CFTimeZoneRef   = const struct __CFTimeZone *;

using CGColorSpaceRef = struct CGColorSpace *;
using CGImageRef      = struct CGImage *;
using CGFontRef       = struct CGFont *;
using CGColorRef      = struct CGColor *;

Q_CORE_EXPORT QDebug operator<<(QDebug, CFArrayRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFURLRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFDataRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFNumberRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFDictionaryRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFLocaleRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFBooleanRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFTimeZoneRef);

Q_CORE_EXPORT QDebug operator<<(QDebug, CFErrorRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFBundleRef);

Q_CORE_EXPORT QDebug operator<<(QDebug, CGPathRef);

Q_CORE_EXPORT QDebug operator<<(QDebug, CGColorSpaceRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CGImageRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CGFontRef);
Q_CORE_EXPORT QDebug operator<<(QDebug, CGColorRef);

// Defined in qcore_mac_objc.mm
Q_CORE_EXPORT QDebug operator<<(QDebug, const NSObject *);
Q_CORE_EXPORT QDebug operator<<(QDebug, CFStringRef);

#endif // Q_OS_DARWIN

#endif