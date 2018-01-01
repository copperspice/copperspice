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

#ifndef QSTRINGBUILDER_H
#define QSTRINGBUILDER_H

#include <qstring.h>
#include <qbytearray.h>
#include <string.h>

QT_BEGIN_NAMESPACE

struct Q_CORE_EXPORT QAbstractConcatenable {
 protected:
   static void convertFromAscii(const char *a, int len, QChar *&out);
   static inline void convertFromAscii(char a, QChar *&out) {
      *out++ = QLatin1Char(a);
   }
};

template <typename T> struct QConcatenable {};

template <typename A, typename B>
class QStringBuilder
{
   typedef QConcatenable<QStringBuilder<A, B> > Concatenable;
   typedef typename Concatenable::ConvertTo ConvertTo;

 public:
   QStringBuilder(const A &a_, const B &b_) : a(a_), b(b_) {}

   operator ConvertTo() const {
      return convertTo<ConvertTo>();
   }

   QByteArray toLatin1() const {
      return convertTo<QString>().toLatin1();
   }
   int size() const {
      return Concatenable::size(*this);
   }

   const A &a;
   const B &b;

 private:
   friend class QByteArray;
   friend class QString;
   template <typename T> T convertTo() const {
      const uint len = QConcatenable< QStringBuilder<A, B> >::size(*this);
      T s(len, Qt::Uninitialized);

      typename T::iterator d = s.data();
      typename T::const_iterator const start = d;
      QConcatenable< QStringBuilder<A, B> >::appendTo(*this, d);

      if (!QConcatenable< QStringBuilder<A, B> >::ExactSize && int(len) != d - start) {
         // this resize is necessary since we allocate a bit too much
         // when dealing with variable sized 8-bit encodings
         s.resize(d - start);
      }
      return s;
   }
};

template <>
class QStringBuilder <QString, QString>
{
 public:
   QStringBuilder(const QString &a_, const QString &b_) : a(a_), b(b_) {}

   operator QString() const {
      QString r(a);
      r += b;
      return r;
   }
   QByteArray toLatin1() const {
      return QString(*this).toLatin1();
   }

   const QString &a;
   const QString &b;
};

template <>
class QStringBuilder <QByteArray, QByteArray>
{
 public:
   QStringBuilder(const QByteArray &a_, const QByteArray &b_) : a(a_), b(b_) {}

   operator QByteArray() const {
      QByteArray r(a);
      r += b;
      return r;
   }

   const QByteArray &a;
   const QByteArray &b;
};


template <> struct QConcatenable<char> : private QAbstractConcatenable {
   typedef char type;
   typedef QByteArray ConvertTo;

   enum { ExactSize = true };

   static int size(const char) {
      return 1;
   }

   static inline void appendTo(const char c, QChar *&out) {
      QAbstractConcatenable::convertFromAscii(c, out);
   }

   static inline void appendTo(const char c, char *&out) {
      *out++ = c;
   }
};

template <> struct QConcatenable<QLatin1Char> {
   typedef QLatin1Char type;
   typedef QString ConvertTo;
   enum { ExactSize = true };

   static int size(const QLatin1Char) {
      return 1;
   }
   static inline void appendTo(const QLatin1Char c, QChar *&out) {
      *out++ = c;
   }
   static inline void appendTo(const QLatin1Char c, char *&out) {
      *out++ = c.toLatin1();
   }
};

template <> struct QConcatenable<QChar> : private QAbstractConcatenable {
   typedef QChar type;
   typedef QString ConvertTo;
   enum { ExactSize = true };
   static int size(const QChar) {
      return 1;
   }
   static inline void appendTo(const QChar c, QChar *&out) {
      *out++ = c;
   }
};

template <> struct QConcatenable<QCharRef> : private QAbstractConcatenable {
   typedef QCharRef type;
   typedef QString ConvertTo;
   enum { ExactSize = true };

   static int size(const QCharRef &) {
      return 1;
   }
   static inline void appendTo(const QCharRef &c, QChar *&out) {
      *out++ = QChar(c);
   }
};

template <> struct QConcatenable<QLatin1String> {
   typedef QLatin1String type;
   typedef QString ConvertTo;
   enum { ExactSize = true };
   static int size(const QLatin1String &a) {
      return a.size();
   }
   static inline void appendTo(const QLatin1String &a, QChar *&out) {
      if (a.data()) {
         for (const char *s = a.data(); *s; ) {
            *out++ = QLatin1Char(*s++);
         }
      }
   }
   static inline void appendTo(const QLatin1String &a, char *&out) {
      if (a.data()) {
         for (const char *s = a.data(); *s; ) {
            *out++ = *s++;
         }
      }
   }
};

template <> struct QConcatenable<QString> : private QAbstractConcatenable {
   typedef QString type;
   typedef QString ConvertTo;
   enum { ExactSize = true };
   static int size(const QString &a) {
      return a.size();
   }
   static inline void appendTo(const QString &a, QChar *&out) {
      const int n = a.size();
      memcpy(out, reinterpret_cast<const char *>(a.constData()), sizeof(QChar) * n);
      out += n;
   }
};

template <> struct QConcatenable<QStringDataPtr> : private QAbstractConcatenable {
   typedef QStringDataPtr type;
   typedef QString ConvertTo;
   enum { ExactSize = true };
   static int size(const type &a) {
      return a.ptr->size;
   }
   static inline void appendTo(const type &a, QChar *&out) {
      memcpy(out, reinterpret_cast<const char *>(a.ptr->data()), sizeof(QChar) * a.ptr->size);
      out += a.ptr->size;
   }
};

template <> struct QConcatenable<QStringRef> : private QAbstractConcatenable {
   typedef QStringRef type;
   typedef QString ConvertTo;
   enum { ExactSize = true };
   static int size(const QStringRef &a) {
      return a.size();
   }
   static inline void appendTo(const QStringRef &a, QChar *&out) {
      const int n = a.size();
      memcpy(out, reinterpret_cast<const char *>(a.constData()), sizeof(QChar) * n);
      out += n;
   }
};

template <int N> struct QConcatenable<char[N]> : private QAbstractConcatenable {
   typedef char type[N];
   typedef QByteArray ConvertTo;
   enum { ExactSize = false };
   static int size(const char[N]) {
      return N - 1;
   }

   static inline void appendTo(const char a[N], QChar *&out) {
      QAbstractConcatenable::convertFromAscii(a, N, out);
   }

   static inline void appendTo(const char a[N], char *&out) {
      while (*a) {
         *out++ = *a++;
      }
   }
};

template <int N> struct QConcatenable<const char[N]> : private QAbstractConcatenable {
   typedef const char type[N];
   typedef QByteArray ConvertTo;
   enum { ExactSize = false };
   static int size(const char[N]) {
      return N - 1;
   }

   static inline void  appendTo(const char a[N], QChar *&out) {
      QAbstractConcatenable::convertFromAscii(a, N, out);
   }

   static inline void appendTo(const char a[N], char *&out) {
      while (*a) {
         *out++ = *a++;
      }
   }
};

template <> struct QConcatenable<const char *> : private QAbstractConcatenable {
   typedef char const *type;
   typedef QByteArray ConvertTo;
   enum { ExactSize = false };
   static int size(const char *a) {
      return qstrlen(a);
   }

   static inline void  appendTo(const char *a, QChar *&out) {
      QAbstractConcatenable::convertFromAscii(a, -1, out);
   }

   static inline void appendTo(const char *a, char *&out) {
      if (!a) {
         return;
      }
      while (*a) {
         *out++ = *a++;
      }
   }
};

template <> struct QConcatenable<QByteArray> : private QAbstractConcatenable {
   typedef QByteArray type;
   typedef QByteArray ConvertTo;

   enum { ExactSize = false };

   static int size(const QByteArray &ba) {
      return ba.size();
   }

   static inline void appendTo(const QByteArray &ba, QChar *&out) {
      // adding 1 because convertFromAscii expects the size including the null-termination
      QAbstractConcatenable::convertFromAscii(ba.constData(), ba.size() + 1, out);
   }

   static inline void appendTo(const QByteArray &ba, char *&out) {
      const char *a = ba.constData();
      const char *const end = ba.end();
      while (a != end) {
         *out++ = *a++;
      }
   }
};

template <> struct QConcatenable<QByteArrayDataPtr> : private QAbstractConcatenable {
   typedef QByteArrayDataPtr type;
   typedef QByteArray ConvertTo;
   enum { ExactSize = false };
   static int size(const type &ba) {
      return ba.ptr->size;
   }

   static inline void appendTo(const type &a, QChar *&out) {
      QAbstractConcatenable::convertFromAscii(static_cast<const char *>(a.ptr->data()), a.ptr->size, out);
   }

   static inline void appendTo(const type &ba, char *&out) {
      ::memcpy(out, ba.ptr->data(), ba.ptr->size);
      out += ba.ptr->size;
   }
};

namespace QtStringBuilder {
template <typename A, typename B> struct ConvertToTypeHelper {
   typedef A ConvertTo;
};
template <typename T> struct ConvertToTypeHelper<T, QString> {
   typedef QString ConvertTo;
};
}

template <typename A, typename B>
struct QConcatenable< QStringBuilder<A, B> > {
   typedef QStringBuilder<A, B> type;
   typedef typename
   QtStringBuilder::ConvertToTypeHelper<typename QConcatenable<A>::ConvertTo, typename QConcatenable<B>::ConvertTo>::ConvertTo
   ConvertTo;
   enum { ExactSize = QConcatenable<A>::ExactSize && QConcatenable<B>::ExactSize };
   static int size(const type &p) {
      return QConcatenable<A>::size(p.a) + QConcatenable<B>::size(p.b);
   }
   template<typename T> static inline void appendTo(const type &p, T *&out) {
      QConcatenable<A>::appendTo(p.a, out);
      QConcatenable<B>::appendTo(p.b, out);
   }
};

template <typename A, typename B>
QStringBuilder<typename QConcatenable<A>::type, typename QConcatenable<B>::type>
operator%(const A &a, const B &b)
{
   return QStringBuilder<typename QConcatenable<A>::type, typename QConcatenable<B>::type>(a, b);
}

#if defined(QT_USE_QSTRINGBUILDER)

template <typename A, typename B>
QStringBuilder<typename QConcatenable<A>::type, typename QConcatenable<B>::type>
operator+(const A &a, const B &b)
{
   return QStringBuilder<typename QConcatenable<A>::type, typename QConcatenable<B>::type>(a, b);
}
#endif

template <typename A, typename B>
QByteArray &operator+=(QByteArray &a, const QStringBuilder<A, B> &b)
{
   if (sizeof(typename QConcatenable< QStringBuilder<A, B> >::ConvertTo::value_type) == sizeof(QChar)) {
      // safe but slower
      return a += QString(b);
   }

   int len = a.size() + QConcatenable< QStringBuilder<A, B> >::size(b);
   a.reserve(len);

   char *it = a.data() + a.size();
   QConcatenable< QStringBuilder<A, B> >::appendTo(b, it);

   a.resize(len); // need to resize after the appendTo for the case str+=foo+str

   return a;
}


template <typename A, typename B>
QString &operator+=(QString &a, const QStringBuilder<A, B> &b)
{
   int len = a.size() + QConcatenable< QStringBuilder<A, B> >::size(b);
   a.reserve(len);

   QChar *it = a.data() + a.size();
   QConcatenable< QStringBuilder<A, B> >::appendTo(b, it);
   a.resize(it - a.constData());    // may be smaller than len if there was conversion from utf8

   return a;
}

QT_END_NAMESPACE

#endif // QSTRINGBUILDER_H
