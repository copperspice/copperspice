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
#include <qurl.h>
#include <qvector.h>

class QDebugStateSaverPrivate;

class Q_CORE_EXPORT QDebug
{
   struct Stream {
      static constexpr const int defaultVerbosity = 2;

      enum FormatFlag {
         NoQuotes = 0x1
      };

      Stream(QIODevice *device)
         : m_addSpace(true), message_output(false), m_flags(0), ref(1), m_verbosity(defaultVerbosity),
           ts(device), type(QtDebugMsg)
      { }

      Stream(QString *string)
         : m_addSpace(true), message_output(false), m_flags(0), ref(1), m_verbosity(defaultVerbosity),
           ts(string, QIODevice::WriteOnly), type(QtDebugMsg)
      { }

      Stream(QtMsgType t)
         : m_addSpace(true), message_output(true), m_flags(0), ref(1),  m_verbosity(defaultVerbosity),
           ts(&buffer, QIODevice::WriteOnly), type(t)
      { }

      bool testFlag(FormatFlag flag) const {
         return (m_flags & flag);
      }

      void setFlag(FormatFlag flag) {
         m_flags = m_flags | flag;
      }

      void unsetFlag(FormatFlag flag) {
         m_flags = m_flags & (~flag);
      }

      void setVerbosity(int v) {
         m_verbosity = v;
      }

      int verbosity() const {
         return m_verbosity;
      }

      bool m_addSpace;
      bool message_output;

      int m_flags;
      int ref;
      int m_verbosity;

      QTextStream ts;
      QString buffer;
      QtMsgType type;
   };

 public:
   QDebug(QIODevice *device)
      : m_stream(new Stream(device))
   { }

   QDebug(QString *string)
      : m_stream(new Stream(string))
   { }

   QDebug(QtMsgType type)
      : m_stream(new Stream(type))
   { }

   QDebug(const QDebug &other)
      : m_stream(other.m_stream) {
      ++m_stream->ref;
   }

   ~QDebug();

   inline QDebug &operator=(const QDebug &other);

   bool autoInsertSpaces() const {
      return m_stream->m_addSpace;
   }

   void setAutoInsertSpaces(bool enable) {
      m_stream->m_addSpace = enable;
   }

   QDebug &space() {
      m_stream->m_addSpace = true;
      m_stream->ts << ' ';

      return *this;
   }

   QDebug &nospace() {
      m_stream->m_addSpace = false;
      return *this;
   }

   QDebug &maybeSpace() {
      if (m_stream->m_addSpace) {
         m_stream->ts << ' ';
      }

      return *this;
   }

   QDebug &maybeQuote(char c = '"') {
      if (! (m_stream->testFlag(Stream::NoQuotes))) {
         m_stream->ts << c;
      }

      return *this;
   }

   QDebug &quote()   {
      m_stream->unsetFlag(Stream::NoQuotes);
      return *this;
   }

   QDebug &noquote() {
      m_stream->setFlag(Stream::NoQuotes);
      return *this;
   }

   QDebug &resetFormat();

   void swap(QDebug &other)  {
      qSwap(m_stream, other.m_stream);
   }

   int verbosity() const {
      return m_stream->verbosity();
   }

   void setVerbosity(int verbosityLevel) {
      m_stream->setVerbosity(verbosityLevel);
   }

   QDebug &operator<<(bool value) {
      m_stream->ts << (value ? "true" : "false");
      return maybeSpace();
   }

   QDebug &operator<<(char value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(signed short value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(unsigned short value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(char16_t value) {
      return *this << QChar(value);
   }

   QDebug &operator<<(char32_t value) {
      return *this << QChar(value);
   }

   QDebug &operator<<(signed int value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(unsigned int value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(signed long value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(unsigned long value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(qint64 value) {
      m_stream->ts << QString::number(value);
      return maybeSpace();
   }

   QDebug &operator<<(quint64 value) {
      m_stream->ts << QString::number(value);
      return maybeSpace();
   }

   QDebug &operator<<(float value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(double value) {
      m_stream->ts << value;
      return maybeSpace();
   }

   QDebug &operator<<(const char *value) {
      m_stream->ts << QString::fromUtf8(value);
      return maybeSpace();
   }

   QDebug &operator<<(QChar value) {
      m_stream->ts << '\'' << value << '\'';
      return maybeSpace();
   }

   QDebug &operator<<(const void *ptr) {
      m_stream->ts << ptr;
      return maybeSpace();
   }

   QDebug &operator<<(std::nullptr_t) {
      m_stream->ts << "(nullptr)";
      return maybeSpace();
   }

   QDebug &operator<<(const QByteArray &str) {
      putByteArray(str);
      return maybeSpace();
   }

   QDebug &operator<<(const QString &str) {
      putString(str);
      return maybeSpace();
   }

   QDebug &operator<<(QStringView str) {
      return operator<<(QString(str));
   }

   QDebug &operator<<(const QUrl &url) {
      m_stream->ts << "QUrl(" << url.toString() << ')';
      return *this;
   }

   QDebug &operator<<(QTextStreamFunction f) {
      m_stream->ts << f;
      return *this;
   }

   QDebug &operator<<(QTextStreamManipulator m) {
      m_stream->ts << m;
      return *this;
   }

 private:
   friend class QDebugStateSaverPrivate;

   Stream *m_stream;

   void putString(QStringView str);
   void putByteArray(const QByteArray &str);
};

class Q_CORE_EXPORT QDebugStateSaver
{
 public:
   QDebugStateSaver(QDebug &debug);

   QDebugStateSaver(const QDebugStateSaver &) = delete;
   QDebugStateSaver &operator=(const QDebugStateSaver &) = delete;

   ~QDebugStateSaver();

 protected:
   QScopedPointer<QDebugStateSaverPrivate> d_ptr;
};

class QNoDebug
{
 public:
   QNoDebug &operator<<(QTextStreamFunction) {
      return *this;
   }

   QNoDebug &operator<<(QTextStreamManipulator) {
      return *this;
   }

   QNoDebug &space() {
      return *this;
   }

   QNoDebug &nospace() {
      return *this;
   }

   QNoDebug &maybeSpace() {
      return *this;
   }

   QNoDebug &quote() {
      return *this;
   }

   QNoDebug &noquote() {
      return *this;
   }

   QNoDebug &maybeQuote(const char = '"') {
      return *this;
   }

   template <typename T>
   QNoDebug &operator<<(const T &) {
      return *this;
   }
};

inline QDebug &QDebug::operator=(const QDebug &other)
{
   if (this != &other) {
      QDebug copy(other);
      qSwap(m_stream, copy.m_stream);
   }

   return *this;
}

template <class T>
inline QDebug operator<<(QDebug debug, const QList<T> &list)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace();

   debug << '(';

   for (auto cnt = 0; cnt < list.count(); ++cnt) {
      if (cnt) {
         debug << ", ";
      }

      debug << list.at(cnt);
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <typename T>
inline QDebug operator<<(QDebug debug, const QVector<T> &vector)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QVector";
   debug.setAutoInsertSpaces(oldSetting);

   return operator<<(debug, vector.toList());
}

template <class Key, class Val, class C>
inline QDebug operator<<(QDebug debug, const QMap<Key, Val, C> &map)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QMap(";

   for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
inline QDebug operator<<(QDebug debug, const QHash<Key, Val, Hash, KeyEqual> &hash)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QHash(";

   for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <class Key, class Val, class C>
inline QDebug operator<<(QDebug debug, const QMultiMap<Key, Val, C> &map)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QMultiMap(";

   for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
inline QDebug operator<<(QDebug debug, const QMultiHash<Key, Val, Hash, KeyEqual> &hash)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QMultiHash(";

   for (auto it = hash.constBegin(); it != hash.constEnd(); ++it) {
      debug << '(' << it.key() << ", " << it.value() << ')';
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <class T1, class T2>
inline QDebug operator<<(QDebug debug, const QPair<T1, T2> &pair)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QPair(" << pair.first << ',' << pair.second << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <typename T>
inline QDebug operator<<(QDebug debug, const QSet<T> &set)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QSet";
   debug.setAutoInsertSpaces(oldSetting);

   return operator<<(debug, set.toList());
}

template <class T>
inline QDebug operator<<(QDebug debug, const QContiguousCache<T> &cache)
{
   const bool oldSetting = debug.autoInsertSpaces();
   debug.nospace() << "QContiguousCache(";

   for (int i = cache.firstIndex(); i <= cache.lastIndex(); ++i) {
      debug << cache[i];

      if (i != cache.lastIndex()) {
         debug << ", ";
      }
   }

   debug << ')';
   debug.setAutoInsertSpaces(oldSetting);

   return debug.maybeSpace();
}

template <class T>
inline QDebug operator<<(QDebug debug, const QFlags<T> &flags)
{
   QDebugStateSaver saver(debug);
   debug.resetFormat();

   debug.nospace() << "QFlags(";
   bool needSeparator = false;

   for (uint i = 0; i < sizeof(T) * 8; ++i) {

      if (flags.testFlag(T(1 << i))) {
         if (needSeparator) {
            debug << '|';
         } else {
            needSeparator = true;
         }

         debug << "0x" << QByteArray::number(T(1 << i), 16).constData();
      }
   }

   debug << ')';

   return debug;
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