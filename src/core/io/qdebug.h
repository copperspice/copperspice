/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <QtCore/qcontiguouscache.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qvector.h>

class Q_CORE_EXPORT QDebug
{
   struct Stream {
      Stream(QIODevice *device)
         : ts(device), ref(1), type(QtDebugMsg), space(true), message_output(false) {}

      Stream(QString *string)
         : ts(string, QIODevice::WriteOnly), ref(1), type(QtDebugMsg), space(true), message_output(false) {}

      Stream(QtMsgType t)
         : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true), message_output(true) {}

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

   QDebug(const QDebug &o): stream(o.stream) {
      ++stream->ref;
   }

   inline QDebug &operator=(const QDebug &other);

   ~QDebug() {
      if (! --stream->ref) {
         if (stream->message_output) {

            QT_TRY {

#if QDEBUG_USE_LOCAL_ENCODING
               qt_message_output(stream->type, stream->buffer.toLocal8Bit().constData());
#else
               qt_message_output(stream->type, stream->buffer.toLatin1().constData());
#endif

            } QT_CATCH(std::bad_alloc &) {
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

   inline QDebug &operator<<(QChar t) {
      stream->ts << '\'' << t << '\'';
      return maybeSpace();
   }

   inline QDebug &operator<<(bool t) {
      stream->ts << (t ? "true" : "false");
      return maybeSpace();
   }

   inline QDebug &operator<<(char t) {
      stream->ts << t;
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

   inline QDebug &operator<<(const char *t) {
      stream->ts << QString::fromLatin1(t);
      return maybeSpace();
   }

   inline QDebug &operator<<(const QString &t) {
      stream->ts << '\"' << t  << '\"';
      return maybeSpace();
   }

   inline QDebug &operator<<(const QStringRef &t) {
      return operator<<(t.toString());
   }

   inline QDebug &operator<<(const QLatin1String &t) {
      stream->ts << '\"'  << t.latin1() << '\"';
      return maybeSpace();
   }
   inline QDebug &operator<<(const QByteArray &t) {
      stream->ts  << '\"' << t << '\"';
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

//
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

#endif