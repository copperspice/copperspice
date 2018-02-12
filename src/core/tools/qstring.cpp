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

#include <qstringlist.h>
#include <qregexp.h>
#include <qunicodetables_p.h>

#ifndef QT_NO_TEXTCODEC
#include <qtextcodec.h>
#endif

#include <qutfcodec_p.h>
#include <qsimd_p.h>
#include <qdatastream.h>
#include <qlist.h>
#include <qlocale.h>
#include <qlocale_p.h>
#include <qvarlengtharray.h>
#include <qtools_p.h>
#include <qhash.h>
#include <qdebug.h>
#include <qendian.h>
#include <qmutex.h>

#ifdef Q_OS_MAC
#include <qcore_mac_p.h>
#endif

#include <qfunctions_p.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef truncate
#undef truncate
#endif

#include "qchar.cpp"               // do not change to use < >

#ifndef LLONG_MAX
#define LLONG_MAX qint64_C(9223372036854775807)
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - qint64_C(1))
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX quint64_C(18446744073709551615)
#endif

#define IS_RAW_DATA(d) ((d)->offset != sizeof(QStringData))


// internal
int qFindString(const QChar *haystack, int haystackLen, int from,
                  const QChar *needle, int needleLen, Qt::CaseSensitivity cs);

static inline int qt_last_index_of(const QChar *haystack, int haystackLen, QChar needle,
                  int from, Qt::CaseSensitivity cs);

static inline int qt_string_count(const QChar *haystack, int haystackLen,
                                  const QChar *needle, int needleLen,
                                  Qt::CaseSensitivity cs);
static inline int qt_string_count(const QChar *haystack, int haystackLen,
                                  QChar needle, Qt::CaseSensitivity cs);
static inline int qt_find_latin1_string(const QChar *hay, int size, const QLatin1String &needle,
                                        int from, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(const QChar *haystack, int haystackLen,
                                  const QChar *needle, int needleLen, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(const QChar *haystack, int haystackLen,
                                  const QLatin1String &needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(const QChar *haystack, int haystackLen,
                                const QChar *needle, int needleLen, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(const QChar *haystack, int haystackLen,
                                const QLatin1String &needle, Qt::CaseSensitivity cs);

// Unicode case-insensitive comparison
static int ucstricmp(const ushort *a, const ushort *ae, const ushort *b, const ushort *be)
{
   if (a == b) {
      return (ae - be);
   }
   if (a == 0) {
      return 1;
   }
   if (b == 0) {
      return -1;
   }

   const ushort *e = ae;
   if (be - b < ae - a) {
      e = a + (be - b);
   }

   uint alast = 0;
   uint blast = 0;
   while (a < e) {
      //         qDebug() << hex << alast << blast;
      //         qDebug() << hex << "*a=" << *a << "alast=" << alast << "folded=" << foldCase (*a, alast);
      //         qDebug() << hex << "*b=" << *b << "blast=" << blast << "folded=" << foldCase (*b, blast);
      int diff = foldCase(*a, alast) - foldCase(*b, blast);
      if ((diff)) {
         return diff;
      }
      ++a;
      ++b;
   }
   if (a == ae) {
      if (b == be) {
         return 0;
      }
      return -1;
   }
   return 1;
}

// Case-insensitive comparison between a Unicode string and a QLatin1String
static int ucstricmp(const ushort *a, const ushort *ae, const uchar *b, const uchar *be)
{
   if (a == 0) {
      if (b == 0) {
         return 0;
      }
      return 1;
   }
   if (b == 0) {
      return -1;
   }

   const ushort *e = ae;
   if (be - b < ae - a) {
      e = a + (be - b);
   }

   while (a < e) {
      int diff = foldCase(*a) - foldCase(*b);
      if ((diff)) {
         return diff;
      }
      ++a;
      ++b;
   }
   if (a == ae) {
      if (b == be) {
         return 0;
      }
      return -1;
   }
   return 1;
}

// Unicode case-sensitive compare two same-sized strings
static int ucstrncmp(const QChar *a, const QChar *b, int l)
{
   while (l-- && *a == *b) {
      a++, b++;
   }
   if (l == -1) {
      return 0;
   }
   return a->unicode() - b->unicode();
}

// Unicode case-sensitive comparison
static int ucstrcmp(const QChar *a, int alen, const QChar *b, int blen)
{
   if (a == b && alen == blen) {
      return 0;
   }
   int l = qMin(alen, blen);
   int cmp = ucstrncmp(a, b, l);
   return cmp ? cmp : (alen - blen);
}

// Unicode case-insensitive compare two same-sized strings
static int ucstrnicmp(const ushort *a, const ushort *b, int l)
{
   return ucstricmp(a, a + l, b, b + l);
}

// Benchmarking indicates that doing memcmp is much slower than
// executing the comparison ourselves.
//
// The profiling was done on a population of calls to qMemEquals, generated
// during a run of the demo browser. The profile of the data (32-bit x86
// Linux) was:
//
//  total number of comparisons: 21353
//  longest string compared: 95
//  average comparison length: 14.8786
//  cache-line crosses: 5661 (13.3%)
//  alignment histogram:
//   0xXXX0 = 512 (1.2%) strings, 0 (0.0%) of which same-aligned
//   0xXXX2 = 15087 (35.3%) strings, 5145 (34.1%) of which same-aligned
//   0xXXX4 = 525 (1.2%) strings, 0 (0.0%) of which same-aligned
//   0xXXX6 = 557 (1.3%) strings, 6 (1.1%) of which same-aligned
//   0xXXX8 = 509 (1.2%) strings, 0 (0.0%) of which same-aligned
//   0xXXXa = 24358 (57.0%) strings, 9901 (40.6%) of which same-aligned
//   0xXXXc = 557 (1.3%) strings, 0 (0.0%) of which same-aligned
//   0xXXXe = 601 (1.4%) strings, 15 (2.5%) of which same-aligned
//   total  = 42706 (100%) strings, 15067 (35.3%) of which same-aligned
//
// 92% of the strings have alignment of 2 or 10, which is due to malloc on
// 32-bit Linux returning values aligned to 8 bytes, and offsetof(array, QString::Data) == 18.
//
// The profile on 64-bit will be different since offsetof(array, QString::Data) == 26.
//
// The benchmark results were, for a Core-i7 @ 2.67 GHz 32-bit, compiled with -O3 -funroll-loops:
//   16-bit loads only:           872,301 CPU ticks [Qt 4.5 / memcmp]
//   32- and 16-bit loads:        773,362 CPU ticks [Qt 4.6]
//   SSE2 "movdqu" 128-bit loads: 618,736 CPU ticks
//   SSE3 "lddqu" 128-bit loads:  619,954 CPU ticks
//   SSSE3 "palignr" corrections: 852,147 CPU ticks
//   SSE4.2 "pcmpestrm":          738,702 CPU ticks
//
// The same benchmark on an Atom N450 @ 1.66 GHz, is:
//  16-bit loads only:            2,185,882 CPU ticks
//  32- and 16-bit loads:         1,805,060 CPU ticks
//  SSE2 "movdqu" 128-bit loads:  2,529,843 CPU ticks
//  SSE3 "lddqu" 128-bit loads:   2,514,858 CPU ticks
//  SSSE3 "palignr" corrections:  2,160,325 CPU ticks
//  SSE4.2 not available
//
// The conclusion we reach is that alignment the SSE2 unaligned code can gain
// 20% improvement in performance in some systems, but suffers a penalty due
// to the unaligned loads on others.

static bool qMemEquals(const quint16 *a, const quint16 *b, int length)
{
   if (a == b || !length) {
      return true;
   }

   union {
      const quint16 *w;
      const quint32 *d;
      quintptr value;
   } sa, sb;
   sa.w = a;
   sb.w = b;

   // check alignment
   if ((sa.value & 2) == (sb.value & 2)) {
      // both addresses have the same alignment
      if (sa.value & 2) {
         // both addresses are not aligned to 4-bytes boundaries
         // compare the first character
         if (*sa.w != *sb.w) {
            return false;
         }
         --length;
         ++sa.w;
         ++sb.w;

         // now both addresses are 4-bytes aligned
      }

      // both addresses are 4-bytes aligned
      // do a fast 32-bit comparison
      const quint32 *e = sa.d + (length >> 1);
      for ( ; sa.d != e; ++sa.d, ++sb.d) {
         if (*sa.d != *sb.d) {
            return false;
         }
      }

      // do we have a tail?
      return (length & 1) ? *sa.w == *sb.w : true;
   } else {
      // one of the addresses isn't 4-byte aligned but the other is
      const quint16 *e = sa.w + length;
      for ( ; sa.w != e; ++sa.w, ++sb.w) {
         if (*sa.w != *sb.w) {
            return false;
         }
      }
   }
   return true;
}

/*!
    \internal

    Returns the index position of the first occurrence of the
    character \a ch in the string given by \a str and \a len,
    searching forward from index
    position \a from. Returns -1 if \a ch could not be found.
*/
static int findChar(const QChar *str, int len, QChar ch, int from,
                    Qt::CaseSensitivity cs)
{
   const ushort *s = (const ushort *)str;
   ushort c = ch.unicode();
   if (from < 0) {
      from = qMax(from + len, 0);
   }
   if (from < len) {
      const ushort *n = s + from - 1;
      const ushort *e = s + len;
      if (cs == Qt::CaseSensitive) {
         while (++n != e)
            if (*n == c) {
               return  n - s;
            }
      } else {
         c = foldCase(c);
         while (++n != e)
            if (foldCase(*n) == c) {
               return  n - s;
            }
      }
   }
   return -1;
}

#define REHASH(a) \
    if (sl_minus_1 < (int)sizeof(int) * CHAR_BIT)       \
        hashHaystack -= (a) << sl_minus_1; \
    hashHaystack <<= 1

inline bool qIsUpper(char ch)
{
   return ch >= 'A' && ch <= 'Z';
}

inline bool qIsDigit(char ch)
{
   return ch >= '0' && ch <= '9';
}

inline char qToLower(char ch)
{
   if (ch >= 'A' && ch <= 'Z') {
      return ch - 'A' + 'a';
   } else {
      return ch;
   }
}

const QString::Null QString::null = { };

// ### replace with QCharIterator
int QString::toUcs4_helper(const ushort *uc, int length, uint *out)
{
   int i = 0;
   for (; i < length; ++i) {
      uint u = uc[i];
      if (QChar::isHighSurrogate(u) && i + 1 < length) {
         ushort low = uc[i + 1];
         if (QChar::isLowSurrogate(low)) {
            ++i;
            u = QChar::surrogateToUcs4(u, low);
         }
      }
      *out++ = u;
   }
   return i;
}

QString::QString(const QChar *unicode, int size)
{
   if (!unicode) {
      d = Data::sharedNull();
   } else {
      if (size < 0) {
         size = 0;
         while (unicode[size] != 0) {
            ++size;
         }
      }
      if (!size) {
         d = Data::allocate(0);
      } else {
         d = Data::allocate(size + 1);
         Q_CHECK_PTR(d);
         d->size = size;
         memcpy(d->data(), unicode, size * sizeof(QChar));
         d->data()[size] = '\0';
      }
   }
}

/*!
    Constructs a string of the given \a size with every character set
    to \a ch.

    \sa fill()
*/
QString::QString(int size, QChar ch)
{
   if (size <= 0) {
      d = Data::allocate(0);
   } else {
      d = Data::allocate(size + 1);
      Q_CHECK_PTR(d);
      d->size = size;
      d->data()[size] = '\0';
      ushort *i = d->data() + size;
      ushort *b = d->data();
      const ushort value = ch.unicode();
      while (i != b) {
         *--i = value;
      }
   }
}

/*! \fn QString::QString(int size, Qt::Initialization)
  \internal

  Constructs a string of the given \a size without initializing the
  characters. This is only used in \c QStringBuilder::toString().
*/
QString::QString(int size, Qt::Initialization)
{
   d = Data::allocate(size + 1);
   Q_CHECK_PTR(d);
   d->size = size;
   d->data()[size] = '\0';
}

/*! \fn QString::QString(const QLatin1String &str)

    Constructs a copy of the Latin-1 string \a str.

    \sa fromLatin1()
*/

/*!
    Constructs a string of size 1 containing the character \a ch.
*/
QString::QString(QChar ch)
{
   d = Data::allocate(2);
   Q_CHECK_PTR(d);

   d->size = 1;
   d->data()[0] = ch.unicode();
   d->data()[1] = '\0';
}

void QString::resize(int size)
{
   if (size < 0) {
      size = 0;
   }

   if (IS_RAW_DATA(d) && !d->ref.isShared() && size < d->size) {
      d->size = size;
      return;
   }

   if (size == 0 && !d->capacityReserved) {
      Data *x = Data::allocate(0);
      if (!d->ref.deref()) {
         Data::deallocate(d);
      }
      d = x;
   } else {
      if (d->ref.isShared() || uint(size) + 1u > d->alloc
            || (!d->capacityReserved && size < d->size
                && uint(size) + 1u < uint(d->alloc >> 1))) {
         reallocData(uint(size) + 1u, true);
      }
      if (d->alloc) {
         d->size = size;
         d->data()[size] = '\0';
      }
   }
}

/*! \fn int QString::capacity() const

    Returns the maximum number of characters that can be stored in
    the string without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QString's memory usage. In general, you will rarely ever
    need to call this function. If you want to know how many
    characters are in the string, call size().

    \sa reserve(), squeeze()
*/

/*!
    \fn void QString::reserve(int size)

    Attempts to allocate memory for at least \a size characters. If
    you know in advance how large the string will be, you can call
    this function, and if you resize the string often you are likely
    to get better performance. If \a size is an underestimate, the
    worst that will happen is that the QString will be a bit slower.

    The sole purpose of this function is to provide a means of fine
    tuning QString's memory usage. In general, you will rarely ever
    need to call this function. If you want to change the size of the
    string, call resize().

    This function is useful for code that needs to build up a long
    string and wants to avoid repeated reallocation. In this example,
    we want to add to the string until some condition is true, and
    we're fairly sure that size is large enough to make a call to
    reserve() worthwhile:

    \snippet doc/src/snippets/qstring/main.cpp 44

    \sa squeeze(), capacity()
*/

/*!
    \fn void QString::squeeze()

    Releases any memory not required to store the character data.

    The sole purpose of this function is to provide a means of fine
    tuning QString's memory usage. In general, you will rarely ever
    need to call this function.

    \sa reserve(), capacity()
*/

void QString::reallocData(uint alloc, bool grow)
{
   if (grow)  {
      alloc = qAllocMore(alloc * sizeof(QChar), sizeof(Data)) / sizeof(QChar);
   }

   if (d->ref.isShared() || IS_RAW_DATA(d)) {
      Data::AllocationOptions allocOptions(d->capacityReserved ? Data::CapacityReserved : 0);
      Data *x = Data::allocate(alloc, allocOptions);

      Q_CHECK_PTR(x);

      x->size = qMin(int(alloc) - 1, d->size);
      ::memcpy(x->data(), d->data(), x->size * sizeof(QChar));
      x->data()[x->size] = 0;

      if (! d->ref.deref())   {
         Data::deallocate(d);
      }

      d = x;

   } else {
      Data *p = static_cast<Data *>(::realloc(d, sizeof(Data) + alloc * sizeof(QChar)));

      Q_CHECK_PTR(p);
      d = p;
      d->alloc = alloc;
      d->offset = sizeof(QStringData);
   }
}

void QString::expand(int i)
{
   int sz = d->size;
   resize(qMax(i + 1, sz));
   if (d->size - 1 > sz) {
      ushort *n = d->data() + d->size - 1;
      ushort *e = d->data() + sz;
      while (n != e) {
         * --n = ' ';
      }
   }
}

/*! \fn void QString::clear()

    Clears the contents of the string and makes it empty.

    \sa resize(), isEmpty()
*/

/*! \fn QString &QString::operator=(const QString &other)

    Assigns \a other to this string and returns a reference to this
    string.
*/

QString &QString::operator=(const QString &other)
{
   other.d->ref.ref();
   if (!d->ref.deref()) {
      Data::deallocate(d);
   }
   d = other.d;
   return *this;
}

QString &QString::operator=(QChar ch)
{
   return operator=(QString(ch));
}

QString &QString::insert(int i, const QLatin1String &str)
{
   const uchar *s = (const uchar *)str.latin1();
   if (i < 0 || !s || !(*s)) {
      return *this;
   }

   int len = str.size();
   expand(qMax(d->size, i) + len - 1);

   ::memmove(d->data() + i + len, d->data() + i, (d->size - i - len) * sizeof(QChar));
   for (int j = 0; j < len; ++j) {
      d->data()[i + j] = s[j];
   }
   return *this;
}

/*!
    \fn QString& QString::insert(int position, const QChar *unicode, int size)
    \overload insert()

    Inserts the first \a size characters of the QChar array \a unicode
    at the given index \a position in the string.
*/
QString &QString::insert(int i, const QChar *unicode, int size)
{
   if (i < 0 || size <= 0) {
      return *this;
   }

   const ushort *s = (const ushort *)unicode;
   if (s >= d->data() && s < d->data() + d->alloc) {
      // Part of me - take a copy
      ushort *tmp = static_cast<ushort *>(::malloc(size * sizeof(QChar)));
      Q_CHECK_PTR(tmp);
      memcpy(tmp, s, size * sizeof(QChar));
      insert(i, reinterpret_cast<const QChar *>(tmp), size);
      ::free(tmp);
      return *this;
   }

   expand(qMax(d->size, i) + size - 1);

   ::memmove(d->data() + i + size, d->data() + i, (d->size - i - size) * sizeof(QChar));
   memcpy(d->data() + i, s, size * sizeof(QChar));
   return *this;
}

/*!
    \fn QString& QString::insert(int position, QChar ch)
    \overload insert()

    Inserts \a ch at the given index \a position in the string.
*/

QString &QString::insert(int i, QChar ch)
{
   if (i < 0) {
      i += d->size;
   }
   if (i < 0) {
      return *this;
   }
   expand(qMax(i, d->size));
   ::memmove(d->data() + i + 1, d->data() + i, (d->size - i - 1) * sizeof(QChar));
   d->data()[i] = ch.unicode();
   return *this;
}

QString &QString::append(const QString &str)
{
   if (str.d != Data::sharedNull()) {
      if (d == Data::sharedNull()) {
         operator=(str);
      } else {
         if (d->ref.isShared() || uint(d->size + str.d->size) + 1u > d->alloc) {
            reallocData(uint(d->size + str.d->size) + 1u, true);
         }
         memcpy(d->data() + d->size, str.d->data(), str.d->size * sizeof(QChar));
         d->size += str.d->size;
         d->data()[d->size] = '\0';
      }
   }
   return *this;
}

/*!
  \overload append()

  Appends the Latin-1 string \a str to this string.
*/
QString &QString::append(const QLatin1String &str)
{
   const uchar *s = (const uchar *)str.latin1();
   if (s) {
      int len = str.size();
      if (d->ref.isShared() || uint(d->size + len) + 1u > d->alloc) {
         reallocData(uint(d->size + len) + 1u, true);
      }
      ushort *i = d->data() + d->size;
      while ((*i++ = *s++))
         ;
      d->size += len;
   }
   return *this;
}


QString &QString::append(QChar ch)
{
   if (d->ref.isShared() || uint(d->size) + 2u > d->alloc)  {
      reallocData(uint(d->size) + 2u, true);
   }

   d->data()[d->size++] = ch.unicode();
   d->data()[d->size] = '\0';

   return *this;
}

QString &QString::remove(int pos, int len)
{
   if (pos < 0) { // count from end of string
      pos += d->size;
   }
   if (pos < 0 || pos >= d->size) {
      // range problems
   } else if (len >= d->size - pos) {
      resize(pos); // truncate
   } else if (len > 0) {
      detach();
      memmove(d->data() + pos, d->data() + pos + len,
              (d->size - pos - len + 1) * sizeof(ushort));
      d->size -= len;
   }
   return *this;
}

QString &QString::remove(const QString &str, Qt::CaseSensitivity cs)
{
   if (str.d->size) {
      int i = 0;
      while ((i = indexOf(str, i, cs)) != -1) {
         remove(i, str.d->size);
      }
   }
   return *this;
}

/*!
  Removes every occurrence of the character \a ch in this string, and
  returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 38

  This is the same as \c replace(ch, "", cs).

  \sa replace()
*/
QString &QString::remove(QChar ch, Qt::CaseSensitivity cs)
{
   int i = 0;
   ushort c = ch.unicode();
   if (cs == Qt::CaseSensitive) {
      while (i < d->size)
         if (d->data()[i] == ch) {
            remove(i, 1);
         } else {
            i++;
         }
   } else {
      c = foldCase(c);
      while (i < d->size)
         if (foldCase(d->data()[i]) == c) {
            remove(i, 1);
         } else {
            i++;
         }
   }
   return *this;
}

/*!
  \fn QString &QString::remove(const QRegExp &rx)

  Removes every occurrence of the regular expression \a rx in the
  string, and returns a reference to the string. For example:

  \snippet doc/src/snippets/qstring/main.cpp 39

  \sa indexOf(), lastIndexOf(), replace()
*/

/*!
  \fn QString &QString::replace(int position, int n, const QString &after)

  Replaces \a n characters beginning at index \a position with
  the string \a after and returns a reference to this string.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 40

  \sa insert(), remove()
*/
QString &QString::replace(int pos, int len, const QString &after)
{
   QString copy = after;
   return replace(pos, len, copy.constData(), copy.length());
}

/*!
  \fn QString &QString::replace(int position, int n, const QChar *unicode, int size)
  \overload replace()
  Replaces \a n characters beginning at index \a position with the
  first \a size characters of the QChar array \a unicode and returns a
  reference to this string.
*/
QString &QString::replace(int pos, int len, const QChar *unicode, int size)
{
   if (pos < 0 || pos > d->size) {
      return *this;
   }
   if (pos + len > d->size) {
      len = d->size - pos;
   }

   uint index = pos;
   replace_helper(&index, 1, len, unicode, size);
   return *this;
}

/*!
  \fn QString &QString::replace(int position, int n, QChar after)
  \overload replace()

  Replaces \a n characters beginning at index \a position with the
  character \a after and returns a reference to this string.
*/
QString &QString::replace(int pos, int len, QChar after)
{
   return replace(pos, len, &after, 1);
}

/*!
  \overload replace()
  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 41

  \note The replacement text is not rescanned after it is inserted.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 86
*/
QString &QString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
   return replace(before.constData(), before.size(), after.constData(), after.size(), cs);
}

/*!
  \internal
 */
void QString::replace_helper(uint *indices, int nIndices, int blen, const QChar *after, int alen)
{
   // copy *after in case it lies inside our own d->data() area
   // (which we could possibly invalidate via a realloc or corrupt via memcpy operations.)
   QChar *afterBuffer = const_cast<QChar *>(after);
   if (after >= reinterpret_cast<QChar *>(d->data()) && after < reinterpret_cast<QChar *>(d->data()) + d->size) {
      afterBuffer = static_cast<QChar *>(::malloc(alen * sizeof(QChar)));
      Q_CHECK_PTR(afterBuffer);
      ::memcpy(afterBuffer, after, alen * sizeof(QChar));
   }

   QT_TRY {
      if (blen == alen)
      {
         // replace in place
         detach();
         for (int i = 0; i < nIndices; ++i) {
            memcpy(d->data() + indices[i], afterBuffer, alen * sizeof(QChar));
         }
      } else if (alen < blen)
      {
         // replace from front
         detach();
         uint to = indices[0];
         if (alen) {
            memcpy(d->data() + to, after, alen * sizeof(QChar));
         }
         to += alen;
         uint movestart = indices[0] + blen;
         for (int i = 1; i < nIndices; ++i) {
            int msize = indices[i] - movestart;
            if (msize > 0) {
               memmove(d->data() + to, d->data() + movestart, msize * sizeof(QChar));
               to += msize;
            }
            if (alen) {
               memcpy(d->data() + to, afterBuffer, alen * sizeof(QChar));
               to += alen;
            }
            movestart = indices[i] + blen;
         }
         int msize = d->size - movestart;
         if (msize > 0) {
            memmove(d->data() + to, d->data() + movestart, msize * sizeof(QChar));
         }
         resize(d->size - nIndices * (blen - alen));
      } else {
         // replace from back
         int adjust = nIndices * (alen - blen);
         int newLen = d->size + adjust;
         int moveend = d->size;
         resize(newLen);

         while (nIndices)
         {
            --nIndices;
            int movestart = indices[nIndices] + blen;
            int insertstart = indices[nIndices] + nIndices * (alen - blen);
            int moveto = insertstart + alen;
            memmove(d->data() + moveto, d->data() + movestart,
            (moveend - movestart)*sizeof(QChar));
            memcpy(d->data() + insertstart, afterBuffer, alen * sizeof(QChar));
            moveend = movestart - blen;
         }
      }
   } QT_CATCH(const std::bad_alloc &) {
      if (afterBuffer != after) {
         ::free(afterBuffer);
      }
      QT_RETHROW;
   }
   if (afterBuffer != after) {
      ::free(afterBuffer);
   }
}

QString &QString::replace(const QChar *before, int blen, const QChar *after, int alen, Qt::CaseSensitivity cs)
{
   if (d->size == 0) {
      if (blen) {
         return *this;
      }

   } else {
      if (cs == Qt::CaseSensitive && before == after && blen == alen) {
         return *this;
      }
   }

   if (alen == 0 && blen == 0) {
      return *this;
   }

   int index = 0;

   QString tmpBefore(before, blen);

   while (1) {
      uint indices[1024];
      uint pos = 0;

      while (pos < 1023) {
         index = this->indexOf(tmpBefore, index, cs);

         if (index == -1) {
            break;
         }

         indices[pos++] = index;
         index += blen;

         // avoid infinite loop
         if (! blen) {
            index++;
         }
      }

      if (! pos) {
         break;
      }

      replace_helper(indices, pos, blen, after, alen);

      if (index == -1) {
         break;
      }

      // index has to be adjusted in case we get back into the loop above
      index += pos * (alen - blen);
   }

   return *this;
}

/*!
  \overload replace()
  Replaces every occurrence of the character \a ch in the string with
  \a after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.
*/
QString &QString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
   if (after.d->size == 0) {
      return remove(ch, cs);
   }

   if (after.d->size == 1) {
      return replace(ch, after.d->data()[0], cs);
   }

   if (d->size == 0) {
      return *this;
   }

   ushort cc = (cs == Qt::CaseSensitive ? ch.unicode() : ch.toCaseFolded().unicode());

   int index = 0;
   while (1) {
      uint indices[1024];
      uint pos = 0;
      if (cs == Qt::CaseSensitive) {
         while (pos < 1023 && index < d->size) {
            if (d->data()[index] == cc) {
               indices[pos++] = index;
            }
            index++;
         }
      } else {
         while (pos < 1023 && index < d->size) {
            if (QChar::toCaseFolded(d->data()[index]) == cc) {
               indices[pos++] = index;
            }
            index++;
         }
      }
      if (!pos) {
         break;
      }

      replace_helper(indices, pos, 1, after.constData(), after.d->size);

      if (index == -1) {
         break;
      }
      // index has to be adjusted in case we get back into the loop above.
      index += pos * (after.d->size - 1);
   }
   return *this;
}

/*!
  \overload replace()
  Replaces every occurrence of the character \a before with the
  character \a after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.
*/
QString &QString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
   ushort a = after.unicode();
   ushort b = before.unicode();
   if (d->size) {
      detach();
      ushort *i = d->data();
      const ushort *e = i + d->size;
      if (cs == Qt::CaseSensitive) {
         for (; i != e; ++i)
            if (*i == b) {
               *i = a;
            }
      } else {
         b = foldCase(b);
         for (; i != e; ++i)
            if (foldCase(*i) == b) {
               *i = a;
            }
      }
   }
   return *this;
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(const QLatin1String &before,
                          const QLatin1String &after,
                          Qt::CaseSensitivity cs)
{
   int alen = after.size();
   QVarLengthArray<ushort> a(alen);
   for (int i = 0; i < alen; ++i) {
      a[i] = (uchar)after.latin1()[i];
   }
   int blen = before.size();
   QVarLengthArray<ushort> b(blen);
   for (int i = 0; i < blen; ++i) {
      b[i] = (uchar)before.latin1()[i];
   }
   return replace((const QChar *)b.data(), blen, (const QChar *)a.data(), alen, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(const QLatin1String &before,
                          const QString &after,
                          Qt::CaseSensitivity cs)
{
   int blen = before.size();
   QVarLengthArray<ushort> b(blen);
   for (int i = 0; i < blen; ++i) {
      b[i] = (uchar)before.latin1()[i];
   }
   return replace((const QChar *)b.data(), blen, after.constData(), after.d->size, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(const QString &before,
                          const QLatin1String &after,
                          Qt::CaseSensitivity cs)
{
   int alen = after.size();
   QVarLengthArray<ushort> a(alen);
   for (int i = 0; i < alen; ++i) {
      a[i] = (uchar)after.latin1()[i];
   }
   return replace(before.constData(), before.d->size, (const QChar *)a.data(), alen, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the character \a c with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(QChar c, const QLatin1String &after, Qt::CaseSensitivity cs)
{
   int alen = after.size();
   QVarLengthArray<ushort> a(alen);
   for (int i = 0; i < alen; ++i) {
      a[i] = (uchar)after.latin1()[i];
   }
   return replace(&c, 1, (const QChar *)a.data(), alen, cs);
}


/*!
  \relates QString
  Returns true if string \a s1 is equal to string \a s2; otherwise
  returns false.

  The comparison is based exclusively on the numeric Unicode values of
  the characters and is very fast, but is not what a human would
  expect. Consider sorting user-interface strings with
  localeAwareCompare().
*/
bool operator==(const QString &s1, const QString &s2)
{
   if (s1.d->size != s2.d->size) {
      return false;
   }

   return qMemEquals(s1.d->data(), s2.d->data(), s1.d->size);
}

/*!
    \overload operator==()
*/
bool QString::operator==(const QLatin1String &other) const
{
   if (d->size != other.size()) {
      return false;
   }

   if (!other.size()) {
      return isEmpty();
   }

   const ushort *uc = d->data();
   const ushort *e = uc + d->size;
   const uchar *c = (uchar *)other.latin1();

   while (uc < e) {
      if (*uc != *c) {
         return false;
      }
      ++uc;
      ++c;
   }
   return true;
}

bool operator<(const QString &s1, const QString &s2)
{
   return ucstrcmp(s1.constData(), s1.length(), s2.constData(), s2.length()) < 0;
}

bool QString::operator<(const QLatin1String &other) const
{
   const uchar *c = (uchar *) other.latin1();
   if (!c || *c == 0) {
      return false;
   }

   const ushort *uc = d->data();
   const ushort *e = uc + qMin(d->size, other.size());

   while (uc < e) {
      if (*uc != *c) {
         break;
      }
      ++uc;
      ++c;
   }
   return (uc == e ? d->size < other.size() : *uc < *c);
}

bool QString::operator>(const QLatin1String &other) const
{
   const uchar *c = (uchar *) other.latin1();
   if (!c || *c == '\0') {
      return !isEmpty();
   }

   const ushort *uc = d->data();
   const ushort *e = uc + qMin(d->size, other.size());

   while (uc < e) {
      if (*uc != *c) {
         break;
      }
      ++uc;
      ++c;
   }
   return (uc == e) ? d->size > other.size() : *uc > *c;
}

int QString::indexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
   return qFindString(unicode(), length(), from, str.unicode(), str.length(), cs);
}

int QString::indexOf(const QLatin1String &str, int from, Qt::CaseSensitivity cs) const
{
   return qt_find_latin1_string(unicode(), size(), str, from, cs);
}

int qFindString(
   const QChar *haystack0, int haystackLen, int from,
   const QChar *needle0, int needleLen, Qt::CaseSensitivity cs)
{
   const int l = haystackLen;
   const int sl = needleLen;
   if (from < 0) {
      from += l;
   }
   if (uint(sl + from) > (uint)l) {
      return -1;
   }
   if (!sl) {
      return from;
   }
   if (!l) {
      return -1;
   }

   if (sl == 1) {
      return findChar(haystack0, haystackLen, needle0[0], from, cs);
   }

   const ushort *needle = (const ushort *)needle0;
   const ushort *haystack = (const ushort *)haystack0 + from;
   const ushort *end = (const ushort *)haystack0 + (l - sl);
   const int sl_minus_1 = sl - 1;

   int hashNeedle = 0, hashHaystack = 0, idx;

   if (cs == Qt::CaseSensitive) {
      for (idx = 0; idx < sl; ++idx) {
         hashNeedle = ((hashNeedle << 1) + needle[idx]);
         hashHaystack = ((hashHaystack << 1) + haystack[idx]);
      }
      hashHaystack -= haystack[sl_minus_1];

      while (haystack <= end) {
         hashHaystack += haystack[sl_minus_1];
         if (hashHaystack == hashNeedle
               && ucstrncmp((const QChar *)needle, (const QChar *)haystack, sl) == 0) {
            return haystack - (const ushort *)haystack0;
         }

         REHASH(*haystack);
         ++haystack;
      }
   } else {
      const ushort *haystack_start = (const ushort *)haystack0;
      for (idx = 0; idx < sl; ++idx) {
         hashNeedle = (hashNeedle << 1) + foldCase(needle + idx, needle);
         hashHaystack = (hashHaystack << 1) + foldCase(haystack + idx, haystack_start);
      }
      hashHaystack -= foldCase(haystack + sl_minus_1, haystack_start);

      while (haystack <= end) {
         hashHaystack += foldCase(haystack + sl_minus_1, haystack_start);
         if (hashHaystack == hashNeedle && ucstrnicmp(needle, haystack, sl) == 0) {
            return haystack - (const ushort *)haystack0;
         }

         REHASH(foldCase(haystack, haystack_start));
         ++haystack;
      }
   }
   return -1;
}

/*!
    \overload indexOf()

    Returns the index position of the first occurrence of the
    character \a ch in the string, searching forward from index
    position \a from. Returns -1 if \a ch could not be found.
*/
int QString::indexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
   return findChar(unicode(), length(), ch, from, cs);
}

/*!
    \since 4.8

    \overload indexOf()

    Returns the index position of the first occurrence of the string
    reference \a str in this string, searching forward from index
    position \a from. Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.
*/
int QString::indexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
   return qFindString(unicode(), length(), from, str.unicode(), str.length(), cs);
}

static int lastIndexOfHelper(const ushort *haystack, int from, const ushort *needle, int sl, Qt::CaseSensitivity cs)
{
   /*
       See indexOf() for explanations.
   */

   const ushort *end = haystack;
   haystack += from;
   const int sl_minus_1 = sl - 1;
   const ushort *n = needle + sl_minus_1;
   const ushort *h = haystack + sl_minus_1;
   int hashNeedle = 0, hashHaystack = 0, idx;

   if (cs == Qt::CaseSensitive) {
      for (idx = 0; idx < sl; ++idx) {
         hashNeedle = ((hashNeedle << 1) + * (n - idx));
         hashHaystack = ((hashHaystack << 1) + * (h - idx));
      }
      hashHaystack -= *haystack;

      while (haystack >= end) {
         hashHaystack += *haystack;
         if (hashHaystack == hashNeedle
               && ucstrncmp((const QChar *)needle, (const QChar *)haystack, sl) == 0) {
            return haystack - end;
         }
         --haystack;
         REHASH(haystack[sl]);
      }
   } else {
      for (idx = 0; idx < sl; ++idx) {
         hashNeedle = ((hashNeedle << 1) + foldCase(n - idx, needle));
         hashHaystack = ((hashHaystack << 1) + foldCase(h - idx, end));
      }
      hashHaystack -= foldCase(haystack, end);

      while (haystack >= end) {
         hashHaystack += foldCase(haystack, end);
         if (hashHaystack == hashNeedle && ucstrnicmp(needle, haystack, sl) == 0) {
            return haystack - end;
         }
         --haystack;
         REHASH(foldCase(haystack + sl, end));
      }
   }
   return -1;
}

/*!
  Returns the index position of the last occurrence of the string \a
  str in this string, searching backward from index position \a
  from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 29

  \sa indexOf(), contains(), count()
*/
int QString::lastIndexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.d->size;
   if (sl == 1) {
      return lastIndexOf(QChar(str.d->data()[0]), from, cs);
   }

   const int l = d->size;
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   return lastIndexOfHelper(d->data(), from, str.d->data(), str.d->size, cs);
}

/*!
  \since 4.5
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string, searching backward from index position \a
  from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet doc/src/snippets/qstring/main.cpp 29

  \sa indexOf(), contains(), count()
*/
int QString::lastIndexOf(const QLatin1String &str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.size();
   if (sl == 1) {
      return lastIndexOf(QLatin1Char(str.latin1()[0]), from, cs);
   }

   const int l = d->size;
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   QVarLengthArray<ushort> s(sl);
   for (int i = 0; i < sl; ++i) {
      s[i] = str.latin1()[i];
   }

   return lastIndexOfHelper(d->data(), from, s.data(), sl, cs);
}

/*!
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the character
  \a ch, searching backward from position \a from.
*/
int QString::lastIndexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
   return qt_last_index_of(unicode(), size(), ch, from, cs);
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string
  reference \a str in this string, searching backward from index
  position \a from. If \a from is -1 (default), the search starts at
  the last character; if \a from is -2, at the next to last character
  and so on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa indexOf(), contains(), count()
*/
int QString::lastIndexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.size();
   if (sl == 1) {
      return lastIndexOf(str.at(0), from, cs);
   }

   const int l = d->size;
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   return lastIndexOfHelper(d->data(), from, reinterpret_cast<const ushort *>(str.unicode()),
                            str.size(), cs);
}

#ifndef QT_NO_REGEXP
struct QStringCapture {
   int pos;
   int len;
   int no;
};

/*!
  \overload replace()

  Replaces every occurrence of the regular expression \a rx in the
  string with \a after. Returns a reference to the string. For
  example:

  \snippet doc/src/snippets/qstring/main.cpp 42

  For regular expressions containing \l{capturing parentheses},
  occurrences of \bold{\\1}, \bold{\\2}, ..., in \a after are replaced
  with \a{rx}.cap(1), cap(2), ...

  \snippet doc/src/snippets/qstring/main.cpp 43

  \sa indexOf(), lastIndexOf(), remove(), QRegExp::cap()
*/
QString &QString::replace(const QRegExp &rx, const QString &after)
{
   QRegExp rx2(rx);

   if (isEmpty() && rx2.indexIn(*this) == -1) {
      return *this;
   }

   reallocData(uint(d->size) + 1u);

   int index = 0;
   int numCaptures = rx2.captureCount();
   int al = after.length();
   QRegExp::CaretMode caretMode = QRegExp::CaretAtZero;

   if (numCaptures > 0) {
      const QChar *uc = after.unicode();
      int numBackRefs = 0;

      for (int i = 0; i < al - 1; i++) {
         if (uc[i] == QLatin1Char('\\')) {
            int no = uc[i + 1].digitValue();
            if (no > 0 && no <= numCaptures) {
               numBackRefs++;
            }
         }
      }

      /*
          This is the harder case where we have back-references.
      */
      if (numBackRefs > 0) {
         QVarLengthArray<QStringCapture, 16> captures(numBackRefs);
         int j = 0;

         for (int i = 0; i < al - 1; i++) {
            if (uc[i] == QLatin1Char('\\')) {
               int no = uc[i + 1].digitValue();
               if (no > 0 && no <= numCaptures) {
                  QStringCapture capture;
                  capture.pos = i;
                  capture.len = 2;

                  if (i < al - 2) {
                     int secondDigit = uc[i + 2].digitValue();
                     if (secondDigit != -1 && ((no * 10) + secondDigit) <= numCaptures) {
                        no = (no * 10) + secondDigit;
                        ++capture.len;
                     }
                  }

                  capture.no = no;
                  captures[j++] = capture;
               }
            }
         }

         while (index <= length()) {
            index = rx2.indexIn(*this, index, caretMode);
            if (index == -1) {
               break;
            }

            QString after2(after);
            for (j = numBackRefs - 1; j >= 0; j--) {
               const QStringCapture &capture = captures[j];
               after2.replace(capture.pos, capture.len, rx2.cap(capture.no));
            }

            replace(index, rx2.matchedLength(), after2);
            index += after2.length();

            // avoid infinite loop on 0-length matches (e.g., QRegExp("[a-z]*"))
            if (rx2.matchedLength() == 0) {
               ++index;
            }

            caretMode = QRegExp::CaretWontMatch;
         }
         return *this;
      }
   }

   /*
       This is the simple and optimized case where we don't have
       back-references.
   */
   while (index != -1) {
      struct {
         int pos;
         int length;
      } replacements[2048];

      int pos = 0;
      int adjust = 0;
      while (pos < 2047) {
         index = rx2.indexIn(*this, index, caretMode);
         if (index == -1) {
            break;
         }
         int ml = rx2.matchedLength();
         replacements[pos].pos = index;
         replacements[pos++].length = ml;
         index += ml;
         adjust += al - ml;
         // avoid infinite loop
         if (!ml) {
            index++;
         }
      }
      if (!pos) {
         break;
      }
      replacements[pos].pos = d->size;
      int newlen = d->size + adjust;

      // to continue searching at the right position after we did
      // the first round of replacements
      if (index != -1) {
         index += adjust;
      }
      QString newstring;
      newstring.reserve(newlen + 1);
      QChar *newuc = newstring.data();
      QChar *uc = newuc;
      int copystart = 0;
      int i = 0;
      while (i < pos) {
         int copyend = replacements[i].pos;
         int size = copyend - copystart;
         memcpy(uc, d->data() + copystart, size * sizeof(QChar));
         uc += size;
         memcpy(uc, after.d->data(), al * sizeof(QChar));
         uc += al;
         copystart = copyend + replacements[i].length;
         i++;
      }
      memcpy(uc, d->data() + copystart, (d->size - copystart) * sizeof(QChar));
      newstring.resize(newlen);
      *this = newstring;
      caretMode = QRegExp::CaretWontMatch;
   }
   return *this;
}
#endif

/*!
    Returns the number of (potentially overlapping) occurrences of
    the string \a str in this string.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa contains(), indexOf()
*/

int QString::count(const QString &str, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), str.unicode(), str.size(), cs);
}

/*!
  \overload count()

  Returns the number of occurrences of character \a ch in the string.
*/

int QString::count(QChar ch, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), ch, cs);
}

/*!
    \since 4.8
    \overload count()
    Returns the number of (potentially overlapping) occurrences of the
    string reference \a str in this string.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa contains(), indexOf()
*/
int QString::count(const QStringRef &str, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), str.unicode(), str.size(), cs);
}


/*! \fn bool QString::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    Returns true if this string contains an occurrence of the string
    \a str; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    Example:
    \snippet doc/src/snippets/qstring/main.cpp 17

    \sa indexOf(), count()
*/

/*! \fn bool QString::contains(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \overload contains()

    Returns true if this string contains an occurrence of the
    character \a ch; otherwise returns false.
*/

/*! \fn bool QString::contains(const QStringRef &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.8

    Returns true if this string contains an occurrence of the string
    reference \a str; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QString::contains(const QRegExp &rx) const

    \overload contains()

    Returns true if the regular expression \a rx matches somewhere in
    this string; otherwise returns false.
*/

/*! \fn bool QString::contains(QRegExp &rx) const
    \overload contains()
    \since 4.5

    Returns true if the regular expression \a rx matches somewhere in
    this string; otherwise returns false.

    If there is a match, the \a rx regular expression will contain the
    matched captures (see QRegExp::matchedLength, QRegExp::cap).
*/

#ifndef QT_NO_REGEXP
/*!
    \overload indexOf()

    Returns the index position of the first match of the regular
    expression \a rx in the string, searching forward from index
    position \a from. Returns -1 if \a rx didn't match anywhere.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 25
*/
int QString::indexOf(const QRegExp &rx, int from) const
{
   QRegExp rx2(rx);
   return rx2.indexIn(*this, from);
}

/*!
    \overload indexOf()
    \since 4.5

    Returns the index position of the first match of the regular
    expression \a rx in the string, searching forward from index
    position \a from. Returns -1 if \a rx didn't match anywhere.

    If there is a match, the \a rx regular expression will contain the
    matched captures (see QRegExp::matchedLength, QRegExp::cap).

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 25
*/
int QString::indexOf(QRegExp &rx, int from) const
{
   return rx.indexIn(*this, from);
}

/*!
    \overload lastIndexOf()

    Returns the index position of the last match of the regular
    expression \a rx in the string, searching backward from index
    position \a from. Returns -1 if \a rx didn't match anywhere.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 30
*/
int QString::lastIndexOf(const QRegExp &rx, int from) const
{
   QRegExp rx2(rx);
   return rx2.lastIndexIn(*this, from);
}

/*!
    \overload lastIndexOf()
    \since 4.5

    Returns the index position of the last match of the regular
    expression \a rx in the string, searching backward from index
    position \a from. Returns -1 if \a rx didn't match anywhere.

    If there is a match, the \a rx regular expression will contain the
    matched captures (see QRegExp::matchedLength, QRegExp::cap).

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 30
*/
int QString::lastIndexOf(QRegExp &rx, int from) const
{
   return rx.lastIndexIn(*this, from);
}

/*!
    \overload count()

    Returns the number of times the regular expression \a rx matches
    in the string.

    This function counts overlapping matches, so in the example
    below, there are four instances of "ana" or "ama":

    \snippet doc/src/snippets/qstring/main.cpp 18

*/
int QString::count(const QRegExp &rx) const
{
   QRegExp rx2(rx);
   int count = 0;
   int index = -1;
   int len = length();
   while (index < len - 1) {                 // count overlapping matches
      index = rx2.indexIn(*this, index + 1);
      if (index == -1) {
         break;
      }
      count++;
   }
   return count;
}
#endif // QT_NO_REGEXP

/*! \fn int QString::count() const

    \overload count()

    Same as size().
*/


/*!
    \enum QString::SectionFlag

    This enum specifies flags that can be used to affect various
    aspects of the section() function's behavior with respect to
    separators and empty fields.

    \value SectionDefault Empty fields are counted, leading and
    trailing separators are not included, and the separator is
    compared case sensitively.

    \value SectionSkipEmpty Treat empty fields as if they don't exist,
    i.e. they are not considered as far as \e start and \e end are
    concerned.

    \value SectionIncludeLeadingSep Include the leading separator (if
    any) in the result string.

    \value SectionIncludeTrailingSep Include the trailing separator
    (if any) in the result string.

    \value SectionCaseInsensitiveSeps Compare the separator
    case-insensitively.

    \sa section()
*/

/*!
    \fn QString QString::section(QChar sep, int start, int end = -1, SectionFlags flags) const

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    character, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behavior, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \snippet doc/src/snippets/qstring/main.cpp 52

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \snippet doc/src/snippets/qstring/main.cpp 53

    \sa split()
*/

/*!
    \overload section()

    \snippet doc/src/snippets/qstring/main.cpp 51
    \snippet doc/src/snippets/qstring/main.cpp 54

    \sa split()
*/

QString QString::section(const QString &sep, int start, int end, SectionFlags flags) const
{
   QStringList sections = split(sep, KeepEmptyParts,
                                (flags & SectionCaseInsensitiveSeps) ? Qt::CaseInsensitive : Qt::CaseSensitive);
   if (sections.isEmpty()) {
      return QString();
   }
   if (!(flags & SectionSkipEmpty)) {
      if (start < 0) {
         start += sections.count();
      }
      if (end < 0) {
         end += sections.count();
      }
   } else {
      int skip = 0;
      for (int k = 0; k < sections.size(); ++k) {
         if (sections.at(k).isEmpty()) {
            skip++;
         }
      }
      if (start < 0) {
         start += sections.count() - skip;
      }
      if (end < 0) {
         end += sections.count() - skip;
      }
   }
   int x = 0;
   QString ret;
   int first_i = start, last_i = end;
   for (int i = 0; x <= end && i < sections.size(); ++i) {
      QString section = sections.at(i);
      const bool empty = section.isEmpty();
      if (x >= start) {
         if (x == start) {
            first_i = i;
         }
         if (x == end) {
            last_i = i;
         }
         if (x > start) {
            ret += sep;
         }
         ret += section;
      }
      if (!empty || !(flags & SectionSkipEmpty)) {
         x++;
      }
   }
   if ((flags & SectionIncludeLeadingSep) && first_i) {
      ret.prepend(sep);
   }
   if ((flags & SectionIncludeTrailingSep) && last_i < sections.size() - 1) {
      ret += sep;
   }
   return ret;
}

#ifndef QT_NO_REGEXP
class qt_section_chunk
{
 public:
   qt_section_chunk(int l, QString s) {
      length = l;
      string = s;
   }
   int length;
   QString string;
};

static QString extractSections(const QList<qt_section_chunk> &sections,
                               int start,
                               int end,
                               QString::SectionFlags flags)
{
   if (start < 0) {
      start += sections.count();
   }
   if (end < 0) {
      end += sections.count();
   }

   QString ret;
   int x = 0;
   int first_i = start, last_i = end;
   for (int i = 0; x <= end && i < sections.size(); ++i) {
      const qt_section_chunk &section = sections.at(i);
      const bool empty = (section.length == section.string.length());
      if (x >= start) {
         if (x == start) {
            first_i = i;
         }
         if (x == end) {
            last_i = i;
         }
         if (x != start) {
            ret += section.string;
         } else {
            ret += section.string.mid(section.length);
         }
      }
      if (!empty || !(flags & QString::SectionSkipEmpty)) {
         x++;
      }
   }

   if ((flags & QString::SectionIncludeLeadingSep) && first_i < sections.size()) {
      const qt_section_chunk &section = sections.at(first_i);
      ret.prepend(section.string.left(section.length));
   }

   if ((flags & QString::SectionIncludeTrailingSep) && last_i + 1 <= sections.size() - 1) {
      const qt_section_chunk &section = sections.at(last_i + 1);
      ret += section.string.left(section.length);
   }

   return ret;
}

QString QString::section(const QRegExp &reg, int start, int end, SectionFlags flags) const
{
   const QChar *uc = unicode();
   if (!uc) {
      return QString();
   }

   QRegExp sep(reg);
   sep.setCaseSensitivity((flags & SectionCaseInsensitiveSeps) ? Qt::CaseInsensitive
                          : Qt::CaseSensitive);

   QList<qt_section_chunk> sections;
   int n = length(), m = 0, last_m = 0, last_len = 0;
   while ((m = sep.indexIn(*this, m)) != -1) {
      sections.append(qt_section_chunk(last_len, QString(uc + last_m, m - last_m)));
      last_m = m;
      last_len = sep.matchedLength();
      m += qMax(sep.matchedLength(), 1);
   }
   sections.append(qt_section_chunk(last_len, QString(uc + last_m, n - last_m)));

   return extractSections(sections, start, end, flags);
}
#endif

/*!
    Returns a substring that contains the \a n leftmost characters
    of the string.

    The entire string is returned if \a n is greater than size() or
    less than zero.

    \snippet doc/src/snippets/qstring/main.cpp 31

    \sa right(), mid(), startsWith()
*/
QString QString::left(int n)  const
{
   if (n >= d->size || n < 0) {
      return *this;
   }
   return QString((const QChar *) d->data(), n);
}

/*!
    Returns a substring that contains the \a n rightmost characters
    of the string.

    The entire string is returned if \a n is greater than size() or
    less than zero.

    \snippet doc/src/snippets/qstring/main.cpp 48

    \sa left(), mid(), endsWith()
*/
QString QString::right(int n) const
{
   if (n >= d->size || n < 0) {
      return *this;
   }
   return QString((const QChar *) d->data() + d->size - n, n);
}

/*!
    Returns a string that contains \a n characters of this string,
    starting at the specified \a position index.

    Returns a null string if the \a position index exceeds the
    length of the string. If there are less than \a n characters
    available in the string starting at the given \a position, or if
    \a n is -1 (default), the function returns all characters that
    are available from the specified \a position.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 34

    \sa left(), right()
*/

QString QString::mid(int position, int n) const
{
   if (position > d->size) {
      return QString();
   }
   if (position < 0) {
      if (n < 0 || n + position >= d->size) {
         return *this;
      }
      if (n + position <= 0) {
         return QString();
      }

      n += position;
      position = 0;
   } else if (n < 0 || n > d->size - position) {
      n = d->size - position;
   }
   if (position == 0 && n == d->size) {
      return *this;
   }
   return QString((const QChar *) d->data() + position, n);
}

/*!
    Returns true if the string starts with \a s; otherwise returns
    false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \snippet doc/src/snippets/qstring/main.cpp 65

    \sa endsWith()
*/
bool QString::startsWith(const QString &s, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(),
                         s.isNull() ? 0 : s.unicode(), s.size(), cs);
}

/*!
  \overload startsWith()
 */
bool QString::startsWith(const QLatin1String &s, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(), s, cs);
}

/*!
  \overload startsWith()

  Returns true if the string starts with \a c; otherwise returns
  false.
*/
bool QString::startsWith(QChar c, Qt::CaseSensitivity cs) const
{
   return d->size
          && (cs == Qt::CaseSensitive
              ? d->data()[0] == c
              : foldCase(d->data()[0]) == foldCase(c.unicode()));
}

/*!
    \since 4.8
    \overload
    Returns true if the string starts with the string reference \a s;
    otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa endsWith()
*/
bool QString::startsWith(const QStringRef &s, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(),
                         s.isNull() ? 0 : s.unicode(), s.size(), cs);
}

/*!
    Returns true if the string ends with \a s; otherwise returns
    false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \snippet doc/src/snippets/qstring/main.cpp 20

    \sa startsWith()
*/
bool QString::endsWith(const QString &s, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(),
                       s.isNull() ? 0 : s.unicode(), s.size(), cs);
}

/*!
    \since 4.8
    \overload endsWith()
    Returns true if the string ends with the string reference \a s;
    otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa startsWith()
*/
bool QString::endsWith(const QStringRef &s, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(),
                       s.isNull() ? 0 : s.unicode(), s.size(), cs);
}


/*!
    \overload endsWith()
*/
bool QString::endsWith(const QLatin1String &s, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(), s, cs);
}

/*!
  Returns true if the string ends with \a c; otherwise returns
  false.

  \overload endsWith()
 */
bool QString::endsWith(QChar c, Qt::CaseSensitivity cs) const
{
   return d->size
          && (cs == Qt::CaseSensitive
              ? d->data()[d->size - 1] == c
              : foldCase(d->data()[d->size - 1]) == foldCase(c.unicode()));
}

#if defined(QT_ALWAYS_HAVE_SSE2)
static inline __m128i mergeQuestionMarks(__m128i chunk)
{
   const __m128i questionMark = _mm_set1_epi16('?');

# ifdef __SSE4_2__
   // compare the unsigned shorts for the range 0x0100-0xFFFF
   // note on the use of _mm_cmpestrm:
   //  The MSDN documentation online (http://technet.microsoft.com/en-us/library/bb514080.aspx)
   //  says for range search the following:
   //    For each character c in a, determine whether b0 <= c <= b1 or b2 <= c <= b3
   //
   //  However, all examples on the Internet, including from Intel
   //  (see http://software.intel.com/en-us/articles/xml-parsing-accelerator-with-intel-streaming-simd-extensions-4-intel-sse4/)
   //  put the range to be searched first
   //
   //  Disassembly and instruction-level debugging with GCC and ICC show
   //  that they are doing the right thing. Inverting the arguments in the
   //  instruction does cause a bunch of test failures.

   const int mode = _SIDD_UWORD_OPS | _SIDD_CMP_RANGES | _SIDD_UNIT_MASK;
   const __m128i rangeMatch = _mm_cvtsi32_si128(0xffff0100);
   const __m128i offLimitMask = _mm_cmpestrm(rangeMatch, 2, chunk, 8, mode);

   // replace the non-Latin 1 characters in the chunk with question marks
   chunk = _mm_blendv_epi8(chunk, questionMark, offLimitMask);
# else
   // SSE has no compare instruction for unsigned comparison.
   // The variables must be shiffted + 0x8000 to be compared
   const __m128i signedBitOffset = _mm_set1_epi16(0x8000);
   const __m128i thresholdMask = _mm_set1_epi16(0xff + 0x8000);

   const __m128i signedChunk = _mm_add_epi16(chunk, signedBitOffset);
   const __m128i offLimitMask = _mm_cmpgt_epi16(signedChunk, thresholdMask);

#  ifdef __SSE4_1__
   // replace the non-Latin 1 characters in the chunk with question marks
   chunk = _mm_blendv_epi8(chunk, questionMark, offLimitMask);
#  else
   // offLimitQuestionMark contains '?' for each 16 bits that was off-limit
   // the 16 bits that were correct contains zeros
   const __m128i offLimitQuestionMark = _mm_and_si128(offLimitMask, questionMark);

   // correctBytes contains the bytes that were in limit
   // the 16 bits that were off limits contains zeros
   const __m128i correctBytes = _mm_andnot_si128(offLimitMask, chunk);

   // merge offLimitQuestionMark and correctBytes to have the result
   chunk = _mm_or_si128(correctBytes, offLimitQuestionMark);
#  endif
# endif
   return chunk;
}
#endif

static QByteArray toLatin1_helper(const QChar *data, int length)
{
   QByteArray ba;
   if (length) {
      ba.resize(length);
      const ushort *src = reinterpret_cast<const ushort *>(data);
      uchar *dst = (uchar *) ba.data();
#if defined(QT_ALWAYS_HAVE_SSE2)
      if (length >= 16) {
         const int chunkCount = length >> 4; // divided by 16

         for (int i = 0; i < chunkCount; ++i) {
            __m128i chunk1 = _mm_loadu_si128((__m128i *)src); // load
            chunk1 = mergeQuestionMarks(chunk1);
            src += 8;

            __m128i chunk2 = _mm_loadu_si128((__m128i *)src); // load
            chunk2 = mergeQuestionMarks(chunk2);
            src += 8;

            // pack the two vector to 16 x 8bits elements
            const __m128i result = _mm_packus_epi16(chunk1, chunk2);

            _mm_storeu_si128((__m128i *)dst, result); // store
            dst += 16;
         }
         length = length % 16;
      }
#elif defined(QT_ALWAYS_HAVE_NEON)
      // Refer to the documentation of the SSE2 implementation
      // this use eactly the same method as for SSE except:
      // 1) neon has unsigned comparison
      // 2) packing is done to 64 bits (8 x 8bits component).
      if (length >= 16) {
         const int chunkCount = length >> 3; // divided by 8
         const uint16x8_t questionMark = vdupq_n_u16('?'); // set
         const uint16x8_t thresholdMask = vdupq_n_u16(0xff); // set
         for (int i = 0; i < chunkCount; ++i) {
            uint16x8_t chunk = vld1q_u16((uint16_t *)src); // load
            src += 8;

            const uint16x8_t offLimitMask = vcgtq_u16(chunk, thresholdMask); // chunk > thresholdMask
            const uint16x8_t offLimitQuestionMark = vandq_u16(offLimitMask, questionMark); // offLimitMask & questionMark
            const uint16x8_t correctBytes = vbicq_u16(chunk, offLimitMask); // !offLimitMask & chunk
            chunk = vorrq_u16(correctBytes, offLimitQuestionMark); // correctBytes | offLimitQuestionMark
            const uint8x8_t result = vmovn_u16(chunk); // narrowing move->packing
            vst1_u8(dst, result); // store
            dst += 8;
         }
         length = length % 8;
      }
#endif
      while (length--) {
         *dst++ = (*src > 0xff) ? '?' : (uchar) * src;
         ++src;
      }
   }
   return ba;
}

QByteArray QString::toLatin1() const
{
   return toLatin1_helper(unicode(), length());
}

QByteArray QString::toAscii() const
{
   return toLatin1();
}

#if !defined(Q_OS_MAC) && defined(Q_OS_UNIX)
static QByteArray toLocal8Bit_helper(const QChar *data, int length)
{
#ifndef QT_NO_TEXTCODEC
   if (QTextCodec::codecForLocale()) {
      return QTextCodec::codecForLocale()->fromUnicode(data, length);
   }
#endif // QT_NO_TEXTCODEC
   return toLatin1_helper(data, length);
}
#endif

QByteArray QString::toLocal8Bit() const
{

#ifndef QT_NO_TEXTCODEC

   if (QTextCodec::codecForLocale())  {
      return QTextCodec::codecForLocale()->fromUnicode(*this);
   }
#endif

   return toLatin1();
}

QByteArray QString::toUtf8() const
{
   if (isNull()) {
      return QByteArray();
   }

   return QUtf8::convertFromUnicode(constData(), length(), 0);
}

QVector<uint> QString::toUcs4() const
{
   QVector<uint> v(length());
   uint *a = v.data();
   int len = toUcs4_helper(d->data(), length(), a);
   v.resize(len);
   return v;
}

QString::Data *QString::fromLatin1_helper(const char *str, int size)
{
   Data *d;
   if (!str) {
      d = Data::sharedNull();
   } else if (size == 0 || (!*str && size < 0)) {
      d = Data::allocate(0);
   } else {
      if (size < 0) {
         size = qstrlen(str);
      }
      d = Data::allocate(size + 1);
      Q_CHECK_PTR(d);
      d->size = size;
      d->data()[size] = '\0';
      ushort *dst = d->data();
      /* SIMD:
       * Unpacking with SSE has been shown to improve performance on recent CPUs
       * The same method gives no improvement with NEON.
       */
#if defined(QT_ALWAYS_HAVE_SSE2)
      if (size >= 16) {
         int chunkCount = size >> 4; // divided by 16
         const __m128i nullMask = _mm_set1_epi32(0);
         for (int i = 0; i < chunkCount; ++i) {
            const __m128i chunk = _mm_loadu_si128((__m128i *)str); // load
            str += 16;

            // unpack the first 8 bytes, padding with zeros
            const __m128i firstHalf = _mm_unpacklo_epi8(chunk, nullMask);
            _mm_storeu_si128((__m128i *)dst, firstHalf); // store
            dst += 8;

            // unpack the last 8 bytes, padding with zeros
            const __m128i secondHalf = _mm_unpackhi_epi8 (chunk, nullMask);
            _mm_storeu_si128((__m128i *)dst, secondHalf); // store
            dst += 8;
         }
         size = size % 16;
      }
#endif
      while (size--) {
         *dst++ = (uchar) * str++;
      }
   }
   return d;
}

QString::Data *QString::fromAscii_helper(const char *str, int size)
{
   return fromLatin1_helper(str, size);
}

/*! \fn QString QString::fromLatin1(const char *str, int size)
    Returns a QString initialized with the first \a size characters
    of the Latin-1 string \a str.

    If \a size is -1 (default), it is taken to be strlen(\a
    str).

    \sa toLatin1(), fromAscii(), fromUtf8(), fromLocal8Bit()
*/


/*! \fn QString QString::fromLocal8Bit(const char *str, int size)
    Returns a QString initialized with the first \a size characters
    of the 8-bit string \a str.

    If \a size is -1 (default), it is taken to be strlen(\a
    str).

    QTextCodec::codecForLocale() is used to perform the conversion.

    \sa toLocal8Bit(), fromAscii(), fromLatin1(), fromUtf8()
*/
QString QString::fromLocal8Bit_helper(const char *str, int size)
{
   if (!str) {
      return QString();
   }
   if (size == 0 || (!*str && size < 0)) {
      QStringDataPtr empty = { Data::allocate(0) };
      return QString(empty);
   }
#if !defined(QT_NO_TEXTCODEC)
   if (size < 0) {
      size = qstrlen(str);
   }
   QTextCodec *codec = QTextCodec::codecForLocale();
   if (codec) {
      return codec->toUnicode(str, size);
   }
#endif // !QT_NO_TEXTCODEC
   return fromLatin1(str, size);
}

QString QString::fromUtf8_helper(const char *str, int size)
{
   if (!str) {
      return QString();
   }

   Q_ASSERT(size != -1);
   return QUtf8::convertToUnicode(str, size, 0);
}

QString QString::fromUtf16(const ushort *unicode, int size)
{
   if (!unicode) {
      return QString();
   }
   if (size < 0) {
      size = 0;
      while (unicode[size] != 0) {
         ++size;
      }
   }
   return QUtf16::convertToUnicode((const char *)unicode, size * 2, 0);
}

QString QString::fromUcs4(const uint *unicode, int size)
{
   if (!unicode) {
      return QString();
   }
   if (size < 0) {
      size = 0;
      while (unicode[size] != 0) {
         ++size;
      }
   }
   return QUtf32::convertToUnicode((const char *)unicode, size * 4, 0);
}

QString &QString::setUnicode(const QChar *unicode, int size)
{
   resize(size);
   if (unicode && size) {
      memcpy(d->data(), unicode, size * sizeof(QChar));
   }
   return *this;
}

QString QString::simplified() const
{
   if (d->size == 0) {
      return *this;
   }

   const QChar *const start = reinterpret_cast<QChar *>(d->data());
   const QChar *from = start;
   const QChar *fromEnd = start + d->size;
   forever {
      QChar ch = *from;
      if (!ch.isSpace())
      {
         break;
      }
      if (++from == fromEnd)
      {
         // All-whitespace string
         QStringDataPtr empty = { Data::allocate(0) };
         return QString(empty);
      }
   }
   // This loop needs no underflow check, as we already determined that
   // the string contains non-whitespace. If the string has exactly one
   // non-whitespace, it will be checked twice - we can live with that.
   while (fromEnd[-1].isSpace()) {
      fromEnd--;
   }
   // The rest of the function depends on the fact that we already know
   // that the last character in the source is no whitespace.
   const QChar *copyFrom = from;
   int copyCount;
   forever {
      if (++from == fromEnd)
      {
         // Only leading and/or trailing whitespace, if any at all
         return mid(copyFrom - start, from - copyFrom);
      }
      QChar ch = *from;
      if (!ch.isSpace())
      {
         continue;
      }
      if (ch != QLatin1Char(' '))
      {
         copyCount = from - copyFrom;
         break;
      }
      ch = *++from;
      if (ch.isSpace())
      {
         copyCount = from - copyFrom - 1;
         break;
      }
   }
   // 'from' now points at the non-trailing whitespace which made the
   // string not simplified in the first place. 'copyCount' is the number
   // of already simplified characters - at least one, obviously -
   // without a trailing space.
   QString result((fromEnd - from) + copyCount, Qt::Uninitialized);
   QChar *to = reinterpret_cast<QChar *>(result.d->data());
   ::memcpy(to, copyFrom, copyCount * 2);
   to += copyCount;
   fromEnd--;
   QChar ch;
   forever {
      *to++ = QLatin1Char(' ');
      do {
         ch = *++from;
      } while (ch.isSpace());
      if (from == fromEnd)
      {
         break;
      }
      do {
         *to++ = ch;
         ch = *++from;
         if (from == fromEnd)
         {
            goto done;
         }
      } while (!ch.isSpace());

   }
done:
   *to++ = ch;
   result.truncate(to - reinterpret_cast<QChar *>(result.d->data()));
   return result;
}

/*!
    Returns a string that has whitespace removed from the start and
    the end.

    Whitespace means any character for which QChar::isSpace() returns
    true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 82

    Unlike simplified(), trimmed() leaves internal whitespace alone.

    \sa simplified()
*/
QString QString::trimmed() const
{
   if (d->size == 0) {
      return *this;
   }
   const QChar *s = (const QChar *)d->data();
   if (!s->isSpace() && !s[d->size - 1].isSpace()) {
      return *this;
   }
   int start = 0;
   int end = d->size - 1;
   while (start <= end && s[start].isSpace()) { // skip white space from start
      start++;
   }
   if (start <= end) {                          // only white space
      while (end && s[end].isSpace()) {         // skip white space from end
         end--;
      }
   }
   int l = end - start + 1;
   if (l <= 0) {
      QStringDataPtr empty = { Data::allocate(0) };
      return QString(empty);
   }
   return QString(s + start, l);
}

/*! \fn const QChar QString::at(int position) const

    Returns the character at the given index \a position in the
    string.

    The \a position must be a valid index position in the string
    (i.e., 0 <= \a position < size()).

    \sa operator[]()
*/

/*!
    \fn QCharRef QString::operator[](int position)

    Returns the character at the specified \a position in the string as a
    modifiable reference.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 85

    The return value is of type QCharRef, a helper class for QString.
    When you get an object of type QCharRef, you can use it as if it
    were a QChar &. If you assign to it, the assignment will apply to
    the character in the QString from which you got the reference.

    \sa at()
*/

/*!
    \fn const QChar QString::operator[](int position) const

    \overload operator[]()
*/

/*! \fn QCharRef QString::operator[](uint position)

\overload operator[]()

Returns the character at the specified \a position in the string as a
modifiable reference. Equivalent to \c at(position).
*/

/*! \fn const QChar QString::operator[](uint position) const

\overload operator[]()
*/

/*!
    \fn void QString::truncate(int position)

    Truncates the string at the given \a position index.

    If the specified \a position index is beyond the end of the
    string, nothing happens.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 83

    If \a position is negative, it is equivalent to passing zero.

    \sa chop(), resize(), left()
*/

void QString::truncate(int pos)
{
   if (pos < d->size) {
      resize(pos);
   }
}


/*!
    Removes \a n characters from the end of the string.

    If \a n is greater than size(), the result is an empty string.

    Example:
    \snippet doc/src/snippets/qstring/main.cpp 15

    If you want to remove characters from the \e beginning of the
    string, use remove() instead.

    \sa truncate(), resize(), remove()
*/
void QString::chop(int n)
{
   if (n > 0) {
      resize(d->size - n);
   }
}

/*!
    Sets every character in the string to character \a ch. If \a size
    is different from -1 (default), the string is resized to \a
    size beforehand.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 21

    \sa resize()
*/

QString &QString::fill(QChar ch, int size)
{
   resize(size < 0 ? d->size : size);
   if (d->size) {
      QChar *i = (QChar *)d->data() + d->size;
      QChar *b = (QChar *)d->data();
      while (i != b) {
         *--i = ch;
      }
   }
   return *this;
}

int QString::compare(const QString &other) const
{
   return ucstrcmp(constData(), length(), other.constData(), other.length());
}

/*!
    \overload compare()
    \since 4.2

    Same as compare(*this, \a other, \a cs).
*/
int QString::compare(const QString &other, Qt::CaseSensitivity cs) const
{
   if (cs == Qt::CaseSensitive) {
      return ucstrcmp(constData(), length(), other.constData(), other.length());
   }
   return ucstricmp(d->data(), d->data() + d->size, other.d->data(), other.d->data() + other.d->size);
}

/*!
    \internal
    \since 4.5
*/
int QString::compare_helper(const QChar *data1, int length1, const QChar *data2, int length2,
                            Qt::CaseSensitivity cs)
{
   if (cs == Qt::CaseSensitive) {
      return ucstrcmp(data1, length1, data2, length2);
   }
   const ushort *s1 = reinterpret_cast<const ushort *>(data1);
   const ushort *s2 = reinterpret_cast<const ushort *>(data2);
   return ucstricmp(s1, s1 + length1, s2, s2 + length2);
}

/*!
    \overload compare()
    \since 4.2

    Same as compare(*this, \a other, \a cs).
*/
int QString::compare(const QLatin1String &other, Qt::CaseSensitivity cs) const
{
   return compare_helper(unicode(), length(), other, cs);
}

/*!
  \fn int QString::compare(const QStringRef &ref, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
  \overload compare()

  Compares the string reference, \a ref, with the string and returns
  an integer less than, equal to, or greater than zero if the string
  is less than, equal to, or greater than \a ref.
*/

/*!
  \fn int QString::compare(const QString &s1, const QStringRef &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
  \overload compare()
*/

/*!
    \internal
    \since 4.5
*/
int QString::compare_helper(const QChar *data1, int length1, QLatin1String s2,
                            Qt::CaseSensitivity cs)
{
   const ushort *uc = reinterpret_cast<const ushort *>(data1);
   const ushort *uce = uc + length1;
   const uchar *c = (uchar *)s2.latin1();

   if (!c) {
      return length1;
   }

   if (cs == Qt::CaseSensitive) {
      const ushort *e = uc + length1;
      if (s2.size() < length1) {
         e = uc + s2.size();
      }
      while (uc < e) {
         int diff = *uc - *c;
         if (diff) {
            return diff;
         }
         uc++, c++;
      }

      if (uc == uce) {
         if (c == (const uchar *)s2.latin1() + s2.size()) {
            return 0;
         }
         return -1;
      }
      return 1;
   } else {
      return ucstricmp(uc, uce, c, c + s2.size());
   }
}

/*!
    \fn int QString::localeAwareCompare(const QString & s1, const QString & s2)

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    On Mac OS X since Qt 4.3, this function compares according the
    "Order for sorted lists" setting in the International prefereces panel.

    \sa compare(), QTextCodec::locale()
*/

/*!
    \fn int QString::localeAwareCompare(const QStringRef &other) const
    \since 4.5
    \overload localeAwareCompare()

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    Same as \c {localeAwareCompare(*this, other)}.
*/

/*!
    \fn int QString::localeAwareCompare(const QString &s1, const QStringRef &s2)
    \since 4.5
    \overload localeAwareCompare()

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.
*/


#if !defined(CSTR_LESS_THAN)
#define CSTR_LESS_THAN    1
#define CSTR_EQUAL        2
#define CSTR_GREATER_THAN 3
#endif

int QString::localeAwareCompare(const QString &other) const
{
   return localeAwareCompare_helper(constData(), length(), other.constData(), other.length());
}

#if defined(Q_OS_WIN32)
#include <qt_windows.h>
#endif

int QString::localeAwareCompare_helper(const QChar *data1, int length1, const QChar *data2, int length2)
{
   // do the right thing for null and empty
   if (length1 == 0 || length2 == 0) {
      return ucstrcmp(data1, length1, data2, length2);
   }

#if defined(Q_OS_WIN32)
   int res = CompareString(GetUserDefaultLCID(), 0, (wchar_t *)data1, length1, (wchar_t *)data2, length2);

   switch (res) {
      case CSTR_LESS_THAN:
         return -1;
      case CSTR_GREATER_THAN:
         return 1;
      default:
         return 0;
   }
#elif defined (Q_OS_MAC)
   // Use CFStringCompare for comparing strings on Mac. This makes Qt order
   // strings the same way as native applications do, and also respects
   // the "Order for sorted lists" setting in the International preferences
   // panel.
   const CFStringRef thisString =
      CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
                                         reinterpret_cast<const UniChar *>(data1), length1, kCFAllocatorNull);
   const CFStringRef otherString =
      CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
                                         reinterpret_cast<const UniChar *>(data2), length2, kCFAllocatorNull);

   const int result = CFStringCompare(thisString, otherString, kCFCompareLocalized);
   CFRelease(thisString);
   CFRelease(otherString);
   return result;
#elif defined(Q_OS_UNIX)
#  if defined(QT_USE_ICU)
   int res;
   if (qt_ucol_strcoll(data1, length1, data2, length2, &res)) {
      if (res == 0) {
         res = ucstrcmp(data1, length1, data2, length2);
      }
      return res;
   } // else fall through
#  endif
   // declared in <string.h>
   int delta = strcoll(toLocal8Bit_helper(data1, length1).constData(), toLocal8Bit_helper(data2, length2).constData());
   if (delta == 0) {
      delta = ucstrcmp(data1, length1, data2, length2);
   }
   return delta;
#else
   return ucstrcmp(data1, length1, data2, length2);
#endif
}


/*!
    \fn const QChar *QString::unicode() const

    Returns a '\\0'-terminated Unicode representation of the string.
    The result remains valid until the string is modified.

    \sa utf16()
*/

/*!
    \fn const ushort *QString::utf16() const

    Returns the QString as a '\\0\'-terminated array of unsigned
    shorts. The result remains valid until the string is modified.

    The returned string is in host byte order.

    \sa unicode()
*/

const ushort *QString::utf16() const
{
   if (IS_RAW_DATA(d)) {
      // ensure '\0'-termination for ::fromRawData strings
      const_cast<QString *>(this)->reallocData(uint(d->size) + 1u);
   }
   return d->data();
}

/*!
    Returns a string of size \a width that contains this string
    padded by the \a fill character.

    If \a truncate is false and the size() of the string is more than
    \a width, then the returned string is a copy of the string.

    \snippet doc/src/snippets/qstring/main.cpp 32

    If \a truncate is true and the size() of the string is more than
    \a width, then any characters in a copy of the string after
    position \a width are removed, and the copy is returned.

    \snippet doc/src/snippets/qstring/main.cpp 33

    \sa rightJustified()
*/

QString QString::leftJustified(int width, QChar fill, bool truncate) const
{
   QString result;
   int len = length();
   int padlen = width - len;
   if (padlen > 0) {
      result.resize(len + padlen);
      if (len) {
         memcpy(result.d->data(), d->data(), sizeof(QChar)*len);
      }
      QChar *uc = (QChar *)result.d->data() + len;
      while (padlen--) {
         * uc++ = fill;
      }
   } else {
      if (truncate) {
         result = left(width);
      } else {
         result = *this;
      }
   }
   return result;
}

/*!
    Returns a string of size() \a width that contains the \a fill
    character followed by the string. For example:

    \snippet doc/src/snippets/qstring/main.cpp 49

    If \a truncate is false and the size() of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is true and the size() of the string is more than
    \a width, then the resulting string is truncated at position \a
    width.

    \snippet doc/src/snippets/qstring/main.cpp 50

    \sa leftJustified()
*/

QString QString::rightJustified(int width, QChar fill, bool truncate) const
{
   QString result;
   int len = length();
   int padlen = width - len;
   if (padlen > 0) {
      result.resize(len + padlen);
      QChar *uc = (QChar *)result.d->data();
      while (padlen--) {
         * uc++ = fill;
      }
      if (len) {
         memcpy(uc, d->data(), sizeof(QChar)*len);
      }
   } else {
      if (truncate) {
         result = left(width);
      } else {
         result = *this;
      }
   }
   return result;
}

/*!
    Returns a lowercase copy of the string.

    \snippet doc/src/snippets/qstring/main.cpp 75

    The case conversion will always happen in the 'C' locale. For locale dependent
    case folding use QLocale::toLower()

    \sa toUpper(), QLocale::toLower()
*/

QString QString::toLower() const
{
   const ushort *p = d->data();
   if (!p) {
      return *this;
   }

   const ushort *e = p + d->size;
   // this avoids out of bounds check in the loop
   while (e != p && QChar::isHighSurrogate(*(e - 1))) {
      --e;
   }

   const QUnicodeTables::Properties *prop;
   while (p != e) {
      if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
         ushort high = *p++;
         prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(high, *p));
      } else {
         prop = QUnicodeTables::qGetProp(*p);
      }

      if (prop->lowerCaseDiff) {
         if (QChar::isLowSurrogate(*p)) {
            --p;   // safe; diff is 0 for surrogates
         }
         QString s(d->size, Qt::Uninitialized);
         memcpy(s.d->data(), d->data(), (p - d->data())*sizeof(ushort));
         ushort *pp = s.d->data() + (p - d->data());

         while (p != e) {
            if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
               *pp = *p++;
               prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(*pp++, *p));
            } else {
               prop = QUnicodeTables::qGetProp(*p);
            }

            if (prop->lowerCaseSpecial) {
               const ushort *specialCase = QUnicodeTables::specialCaseMap + prop->lowerCaseDiff;
               ushort length = *specialCase++;
               int pos = pp - s.d->data();
               s.resize(s.d->size + length - 1);
               pp = s.d->data() + pos;
               while (length--) {
                  *pp++ = *specialCase++;
               }
            } else {
               *pp++ = *p + prop->lowerCaseDiff;
            }
            ++p;
         }

         // this restores high surrogate parts eaten above, if any
         while (e != d->data() + d->size) {
            *pp++ = *e++;
         }

         return s;
      }
      ++p;
   }
   return *this;
}

/*!
    Returns the case folded equivalent of the string. For most Unicode
    characters this is the same as toLower().
*/
QString QString::toCaseFolded() const
{
   const ushort *p = d->data();
   if (!p) {
      return *this;
   }

   const ushort *e = p + d->size;
   // this avoids out of bounds check in the loop
   while (e != p && QChar::isHighSurrogate(*(e - 1))) {
      --e;
   }

   const QUnicodeTables::Properties *prop;
   while (p != e) {
      if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
         ushort high = *p++;
         prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(high, *p));
      } else {
         prop = QUnicodeTables::qGetProp(*p);
      }
      if (prop->caseFoldDiff) {
         if (QChar::isLowSurrogate(*p)) {
            --p;   // safe; diff is 0 for surrogates
         }

         QString s(d->size, Qt::Uninitialized);
         memcpy(s.d->data(), d->data(), (p - d->data())*sizeof(ushort));
         ushort *pp = s.d->data() + (p - d->data());
         while (p != e) {
            if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
               *pp = *p++;
               prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(*pp++, *p));
            } else {
               prop = QUnicodeTables::qGetProp(*p);
            }

            if (prop->caseFoldSpecial) {
               //### we currently don't support full case foldings
            } else {
               *pp++ = *p + prop->caseFoldDiff;
            }
            ++p;
         }

         // this restores high surrogate parts eaten above, if any
         while (e != d->data() + d->size) {
            *pp++ = *e++;
         }

         return s;
      }
      ++p;
   }
   return *this;
}

/*!
    Returns an uppercase copy of the string.

    \snippet doc/src/snippets/qstring/main.cpp 81

    The case conversion will always happen in the 'C' locale. For locale dependent
    case folding use QLocale::toUpper()

    \sa toLower(), QLocale::toLower()
*/
QString QString::toUpper() const
{
   const ushort *p = d->data();
   if (!p) {
      return *this;
   }

   const ushort *e = p + d->size;
   // this avoids out of bounds check in the loop
   while (e != p && QChar::isHighSurrogate(*(e - 1))) {
      --e;
   }

   const QUnicodeTables::Properties *prop;
   while (p != e) {
      if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
         ushort high = *p++;
         prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(high, *p));
      } else {
         prop = QUnicodeTables::qGetProp(*p);
      }

      if (prop->upperCaseDiff) {
         if (QChar::isLowSurrogate(*p)) {
            --p;   // safe; diff is 0 for surrogates
         }
         QString s(d->size, Qt::Uninitialized);
         memcpy(s.d->data(), d->data(), (p - d->data())*sizeof(ushort));
         ushort *pp = s.d->data() + (p - d->data());
         while (p != e) {
            if (QChar::isHighSurrogate(*p) && QChar::isLowSurrogate(p[1])) {
               *pp = *p++;
               prop = QUnicodeTables::qGetProp(QChar::surrogateToUcs4(*pp++, *p));
            } else {
               prop = QUnicodeTables::qGetProp(*p);
            }
            if (prop->upperCaseSpecial) {
               const ushort *specialCase = QUnicodeTables::specialCaseMap + prop->upperCaseDiff;
               ushort length = *specialCase++;
               int pos = pp - s.d->data();
               s.resize(s.d->size + length - 1);
               pp = s.d->data() + pos;
               while (length--) {
                  *pp++ = *specialCase++;
               }
            } else {
               *pp++ = *p + prop->upperCaseDiff;
            }
            ++p;
         }

         // this restores high surrogate parts eaten above, if any
         while (e != d->data() + d->size) {
            *pp++ = *e++;
         }

         return s;
      }
      ++p;
   }
   return *this;
}

// ### Qt5: remove

QString &QString::sprintf(const char *cformat, ...)
{
   va_list ap;
   va_start(ap, cformat);
   QString &s = vsprintf(cformat, ap);
   va_end(ap);
   return s;
}

/*!
    Equivalent method to sprintf(), but takes a va_list \a ap
    instead a list of variable arguments. See the sprintf()
    documentation for an explanation of \a cformat.

    This method does not call the va_end macro, the caller
    is responsible to call va_end on \a ap.

    \sa sprintf()
*/

QString &QString::vsprintf(const char *cformat, va_list ap)
{
   QLocale locale(QLocale::C);

   if (!cformat || !*cformat) {
      // Qt 1.x compat
      *this = fromLatin1("");
      return *this;
   }

   // Parse cformat

   QString result;
   const char *c = cformat;
   for (;;) {
      // Copy non-escape chars to result
      while (*c != '\0' && *c != '%') {
         result.append(QLatin1Char(*c++));
      }

      if (*c == '\0') {
         break;
      }

      // Found '%'
      const char *escape_start = c;
      ++c;

      if (*c == '\0') {
         result.append(QLatin1Char('%')); // a % at the end of the string - treat as non-escape text
         break;
      }
      if (*c == '%') {
         result.append(QLatin1Char('%')); // %%
         ++c;
         continue;
      }

      // Parse flag characters
      uint flags = 0;
      bool no_more_flags = false;
      do {
         switch (*c) {
            case '#':
               flags |= QLocalePrivate::Alternate;
               break;
            case '0':
               flags |= QLocalePrivate::ZeroPadded;
               break;
            case '-':
               flags |= QLocalePrivate::LeftAdjusted;
               break;
            case ' ':
               flags |= QLocalePrivate::BlankBeforePositive;
               break;
            case '+':
               flags |= QLocalePrivate::AlwaysShowSign;
               break;
            case '\'':
               flags |= QLocalePrivate::ThousandsGroup;
               break;
            default:
               no_more_flags = true;
               break;
         }

         if (!no_more_flags) {
            ++c;
         }
      } while (!no_more_flags);

      if (*c == '\0') {
         result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
         break;
      }

      // Parse field width
      int width = -1; // -1 means unspecified
      if (qIsDigit(*c)) {
         QString width_str;
         while (*c != '\0' && qIsDigit(*c)) {
            width_str.append(QLatin1Char(*c++));
         }

         // can't be negative - started with a digit
         // contains at least one digit
         width = width_str.toInt();
      } else if (*c == '*') {
         width = va_arg(ap, int);
         if (width < 0) {
            width = -1;   // treat all negative numbers as unspecified
         }
         ++c;
      }

      if (*c == '\0') {
         result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
         break;
      }

      // Parse precision
      int precision = -1; // -1 means unspecified
      if (*c == '.') {
         ++c;
         if (qIsDigit(*c)) {
            QString precision_str;
            while (*c != '\0' && qIsDigit(*c)) {
               precision_str.append(QLatin1Char(*c++));
            }

            // can't be negative - started with a digit
            // contains at least one digit
            precision = precision_str.toInt();
         } else if (*c == '*') {
            precision = va_arg(ap, int);
            if (precision < 0) {
               precision = -1;   // treat all negative numbers as unspecified
            }
            ++c;
         }
      }

      if (*c == '\0') {
         result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
         break;
      }

      // Parse the length modifier
      enum LengthMod { lm_none, lm_hh, lm_h, lm_l, lm_ll, lm_L, lm_j, lm_z, lm_t };
      LengthMod length_mod = lm_none;
      switch (*c) {
         case 'h':
            ++c;
            if (*c == 'h') {
               length_mod = lm_hh;
               ++c;
            } else {
               length_mod = lm_h;
            }
            break;

         case 'l':
            ++c;
            if (*c == 'l') {
               length_mod = lm_ll;
               ++c;
            } else {
               length_mod = lm_l;
            }
            break;

         case 'L':
            ++c;
            length_mod = lm_L;
            break;

         case 'j':
            ++c;
            length_mod = lm_j;
            break;

         case 'z':
         case 'Z':
            ++c;
            length_mod = lm_z;
            break;

         case 't':
            ++c;
            length_mod = lm_t;
            break;

         default:
            break;
      }

      if (*c == '\0') {
         result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
         break;
      }

      // Parse the conversion specifier and do the conversion
      QString subst;
      switch (*c) {
         case 'd':
         case 'i': {
            qint64 i;
            switch (length_mod) {
               case lm_none:
                  i = va_arg(ap, int);
                  break;
               case lm_hh:
                  i = va_arg(ap, int);
                  break;
               case lm_h:
                  i = va_arg(ap, int);
                  break;
               case lm_l:
                  i = va_arg(ap, long int);
                  break;
               case lm_ll:
                  i = va_arg(ap, qint64);
                  break;
               case lm_j:
                  i = va_arg(ap, long int);
                  break;
               case lm_z:
                  i = va_arg(ap, size_t);
                  break;
               case lm_t:
                  i = va_arg(ap, int);
                  break;
               default:
                  i = 0;
                  break;
            }
            subst = locale.d()->longLongToString(i, precision, 10, width, flags);
            ++c;
            break;
         }
         case 'o':
         case 'u':
         case 'x':
         case 'X': {
            quint64 u;
            switch (length_mod) {
               case lm_none:
                  u = va_arg(ap, uint);
                  break;
               case lm_hh:
                  u = va_arg(ap, uint);
                  break;
               case lm_h:
                  u = va_arg(ap, uint);
                  break;
               case lm_l:
                  u = va_arg(ap, ulong);
                  break;
               case lm_ll:
                  u = va_arg(ap, quint64);
                  break;
               case lm_z:
                  u = va_arg(ap, size_t);
                  break;
               default:
                  u = 0;
                  break;
            }

            if (qIsUpper(*c)) {
               flags |= QLocalePrivate::CapitalEorX;
            }

            int base = 10;
            switch (qToLower(*c)) {
               case 'o':
                  base = 8;
                  break;
               case 'u':
                  base = 10;
                  break;
               case 'x':
                  base = 16;
                  break;
               default:
                  break;
            }
            subst = locale.d()->unsLongLongToString(u, precision, base, width, flags);
            ++c;
            break;
         }
         case 'E':
         case 'e':
         case 'F':
         case 'f':
         case 'G':
         case 'g':
         case 'A':
         case 'a': {
            double d;
            if (length_mod == lm_L) {
               d = va_arg(ap, long double);   // not supported - converted to a double
            } else {
               d = va_arg(ap, double);
            }

            if (qIsUpper(*c)) {
               flags |= QLocalePrivate::CapitalEorX;
            }

            QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
            switch (qToLower(*c)) {
               case 'e':
                  form = QLocalePrivate::DFExponent;
                  break;
               case 'a':                             // not supported - decimal form used instead
               case 'f':
                  form = QLocalePrivate::DFDecimal;
                  break;
               case 'g':
                  form = QLocalePrivate::DFSignificantDigits;
                  break;
               default:
                  break;
            }
            subst = locale.d()->doubleToString(d, precision, form, width, flags);
            ++c;
            break;
         }
         case 'c': {
            if (length_mod == lm_l) {
               subst = QChar((ushort) va_arg(ap, int));
            } else {
               subst = QLatin1Char((uchar) va_arg(ap, int));
            }
            ++c;
            break;
         }
         case 's': {
            if (length_mod == lm_l) {
               const ushort *buff = va_arg(ap, const ushort *);
               const ushort *ch = buff;
               while (*ch != 0) {
                  ++ch;
               }
               subst.setUtf16(buff, ch - buff);
            } else {
               subst = QString::fromUtf8(va_arg(ap, const char *));
            }
            if (precision != -1) {
               subst.truncate(precision);
            }
            ++c;
            break;
         }
         case 'p': {
            void *arg = va_arg(ap, void *);
#ifdef Q_OS_WIN64
            quint64 i = reinterpret_cast<quint64>(arg);
#else
            quint64 i = reinterpret_cast<unsigned long>(arg);
#endif
            flags |= QLocalePrivate::Alternate;
            subst = locale.d()->unsLongLongToString(i, precision, 16, width, flags);
            ++c;
            break;
         }
         case 'n':
            switch (length_mod) {
               case lm_hh: {
                  signed char *n = va_arg(ap, signed char *);
                  *n = result.length();
                  break;
               }
               case lm_h: {
                  short int *n = va_arg(ap, short int *);
                  *n = result.length();
                  break;
               }
               case lm_l: {
                  long int *n = va_arg(ap, long int *);
                  *n = result.length();
                  break;
               }
               case lm_ll: {
                  qint64 *n = va_arg(ap, qint64 *);
                  volatile uint tmp = result.length(); // egcs-2.91.66 gets internal
                  *n = tmp;                             // compiler error without volatile
                  break;
               }
               default: {
                  int *n = va_arg(ap, int *);
                  *n = result.length();
                  break;
               }
            }
            ++c;
            break;

         default: // bad escape, treat as non-escape text
            for (const char *cc = escape_start; cc != c; ++cc) {
               result.append(QLatin1Char(*cc));
            }
            continue;
      }

      if (flags & QLocalePrivate::LeftAdjusted) {
         result.append(subst.leftJustified(width));
      } else {
         result.append(subst.rightJustified(width));
      }
   }

   *this = result;

   return *this;
}

/*!
    Returns the string converted to a \c{long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toLongLong()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 74

    \sa number(), toULongLong(), toInt(), QLocale::toLongLong()
*/

qint64 QString::toLongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
   if (base != 0 && (base < 2 || base > 36)) {
      qWarning("Warning: QString::toLongLong() Invalid base (%d)", base);
      base = 10;
   }
#endif

   QLocale c_locale(QLocale::C);
   return c_locale.d()->stringToLongLong(*this, base, ok, QLocalePrivate::FailOnGroupSeparators);
}

/*!
    Returns the string converted to an \c{unsigned long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toULongLong()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 79

    \sa number(), toLongLong(), QLocale::toULongLong()
*/

quint64 QString::toULongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
   if (base != 0 && (base < 2 || base > 36)) {
      qWarning("Warning: QString::toULongLong() Invalid base (%d)", base);
      base = 10;
   }
#endif

   QLocale c_locale(QLocale::C);
   return c_locale.d()->stringToUnsLongLong(*this, base, ok, QLocalePrivate::FailOnGroupSeparators);
}

/*!
    \fn long QString::toLong(bool *ok, int base) const

    Returns the string converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toLong()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 73

    \sa number(), toULong(), toInt(), QLocale::toLong()
*/

long QString::toLong(bool *ok, int base) const
{
   qint64 v = toLongLong(ok, base);
   if (v < LONG_MIN || v > LONG_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return (long)v;
}

/*!
    \fn ulong QString::toULong(bool *ok, int base) const

    Returns the string converted to an \c{unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toULong()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 78

    \sa number(), QLocale::toULong()
*/

ulong QString::toULong(bool *ok, int base) const
{
   quint64 v = toULongLong(ok, base);
   if (v > ULONG_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return (ulong)v;
}


/*!
    Returns the string converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toInt()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 72

    \sa number(), toUInt(), toDouble(), QLocale::toInt()
*/

int QString::toInt(bool *ok, int base) const
{
   qint64 v = toLongLong(ok, base);
   if (v < INT_MIN || v > INT_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return v;
}

/*!
    Returns the string converted to an \c{unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toUInt()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 77

    \sa number(), toInt(), QLocale::toUInt()
*/

uint QString::toUInt(bool *ok, int base) const
{
   quint64 v = toULongLong(ok, base);
   if (v > UINT_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return (uint)v;
}

/*!
    Returns the string converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toShort()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 76

    \sa number(), toUShort(), toInt(), QLocale::toShort()
*/

short QString::toShort(bool *ok, int base) const
{
   long v = toLongLong(ok, base);
   if (v < SHRT_MIN || v > SHRT_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return (short)v;
}

/*!
    Returns the string converted to an \c{unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toUShort()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 80

    \sa number(), toShort(), QLocale::toUShort()
*/

ushort QString::toUShort(bool *ok, int base) const
{
   ulong v = toULongLong(ok, base);
   if (v > USHRT_MAX) {
      if (ok) {
         *ok = false;
      }
      v = 0;
   }
   return (ushort)v;
}


/*!
    Returns the string converted to a \c double value.

    Returns 0.0 if the conversion fails.

    If a conversion error occurs, \c{*}\a{ok} is set to false;
    otherwise \c{*}\a{ok} is set to true.

    \snippet doc/src/snippets/qstring/main.cpp 66

    Various string formats for floating point numbers can be converted
    to double values:

    \snippet doc/src/snippets/qstring/main.cpp 67

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toDouble()

    \snippet doc/src/snippets/qstring/main.cpp 68

    For historic reasons, this function does not handle
    thousands group separators. If you need to convert such numbers,
    use QLocale::toDouble().

    \snippet doc/src/snippets/qstring/main.cpp 69

    \sa number() QLocale::setDefault() QLocale::toDouble() trimmed()
*/

double QString::toDouble(bool *ok) const
{
   QLocale c_locale(QLocale::C);
   return c_locale.d()->stringToDouble(*this, ok, QLocalePrivate::FailOnGroupSeparators);
}

/*!
    Returns the string converted to a \c float value.

    If a conversion error occurs, *\a{ok} is set to false; otherwise
    *\a{ok} is set to true. Returns 0.0 if the conversion fails.

    The string conversion will always happen in the 'C' locale. For locale
    dependent conversion use QLocale::toFloat()

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 71

    \sa number(), toDouble(), toInt(), QLocale::toFloat()
*/

#define QT_MAX_FLOAT 3.4028234663852886e+38

float QString::toFloat(bool *ok) const
{
   bool myOk;
   double d = toDouble(&myOk);
   if (!myOk || d > QT_MAX_FLOAT || d < -QT_MAX_FLOAT) {
      if (ok != 0) {
         *ok = false;
      }
      return 0.0;
   }
   if (ok != 0) {
      *ok = true;
   }
   return (float) d;
}

/*! \fn QString &QString::setNum(int n, int base)

    Sets the string to the printed value of \a n in the specified \a
    base, and returns a reference to the string.

    The base is 10 by default and must be between 2 and 36. For bases
    other than 10, \a n is treated as an unsigned integer.

    \snippet doc/src/snippets/qstring/main.cpp 56

   The formatting always uses QLocale::C, i.e., English/UnitedStates.
   To get a localized string representation of a number, use
   QLocale::toString() with the appropriate locale.
*/

/*! \fn QString &QString::setNum(uint n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(long n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(ulong n, int base)

    \overload
*/

/*!
    \overload
*/
QString &QString::setNum(qint64 n, int base)
{
#if defined(QT_CHECK_RANGE)
   if (base < 2 || base > 36) {
      qWarning("Warning: QString::setNum() Invalid base (%d)", base);
      base = 10;
   }
#endif

   QLocale locale(QLocale::C);
   *this = locale.d()->longLongToString(n, -1, base);
   return *this;
}

/*!
    \overload
*/
QString &QString::setNum(quint64 n, int base)
{
#if defined(QT_CHECK_RANGE)
   if (base < 2 || base > 36) {
      qWarning("Warning: QString::setNum() Invalid base (%d)", base);
      base = 10;
   }
#endif
   QLocale locale(QLocale::C);
   *this = locale.d()->unsLongLongToString(n, -1, base);
   return *this;
}

/*! \fn QString &QString::setNum(short n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(ushort n, int base)

    \overload
*/

/*!
    \fn QString &QString::setNum(double n, char format, int precision)
    \overload

    Sets the string to the printed value of \a n, formatted according
    to the given \a format and \a precision, and returns a reference
    to the string.

    The \a format can be 'f', 'F', 'e', 'E', 'g' or 'G' (see the
    arg() function documentation for an explanation of the formats).

    The formatting always uses QLocale::C, i.e., English/UnitedStates.
    To get a localized string representation of a number, use
    QLocale::toString() with the appropriate locale.
*/

QString &QString::setNum(double n, char f, int prec)
{
   QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
   uint flags = 0;

   if (qIsUpper(f)) {
      flags = QLocalePrivate::CapitalEorX;
   }
   f = qToLower(f);

   switch (f) {
      case 'f':
         form = QLocalePrivate::DFDecimal;
         break;
      case 'e':
         form = QLocalePrivate::DFExponent;
         break;
      case 'g':
         form = QLocalePrivate::DFSignificantDigits;
         break;
      default:
#if defined(QT_CHECK_RANGE)
         qWarning("Warning: QString::setNum() Invalid format char '%c'", f);
#endif
         break;
   }

   QLocale locale(QLocale::C);
   *this = locale.d()->doubleToString(n, prec, form, -1, flags);
   return *this;
}

/*!
    \fn QString &QString::setNum(float n, char format, int precision)
    \overload

    Sets the string to the printed value of \a n, formatted according
    to the given \a format and \a precision, and returns a reference
    to the string.

    The formatting always uses QLocale::C, i.e., English/UnitedStates.
    To get a localized string representation of a number, use
    QLocale::toString() with the appropriate locale.
*/


/*!
    \fn QString QString::number(long n, int base)

    Returns a string equivalent of the number \a n according to the
    specified \a base.

    The base is 10 by default and must be between 2
    and 36. For bases other than 10, \a n is treated as an
    unsigned integer.

    The formatting always uses QLocale::C, i.e., English/UnitedStates.
    To get a localized string representation of a number, use
    QLocale::toString() with the appropriate locale.

    \snippet doc/src/snippets/qstring/main.cpp 35

    \sa setNum()
*/

QString QString::number(long n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}

/*!
  \fn QString QString::number(ulong n, int base)

    \overload
*/
QString QString::number(ulong n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}

/*!
    \overload
*/
QString QString::number(int n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}

/*!
    \overload
*/
QString QString::number(uint n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}

/*!
    \overload
*/
QString QString::number(qint64 n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}

/*!
    \overload
*/
QString QString::number(quint64 n, int base)
{
   QString s;
   s.setNum(n, base);
   return s;
}


/*!
    \fn QString QString::number(double n, char format, int precision)

    Returns a string equivalent of the number \a n, formatted
    according to the specified \a format and \a precision. See
    \l{Argument Formats} for details.

    Unlike QLocale::toString(), this function does not honor the
    user's locale settings.

    \sa setNum(), QLocale::toString()
*/
QString QString::number(double n, char f, int prec)
{
   QString s;
   s.setNum(n, f, prec);
   return s;
}

/*!
    Splits the string into substrings wherever \a sep occurs, and
    returns the list of those strings. If \a sep does not match
    anywhere in the string, split() returns a single-element list
    containing this string.

    \a cs specifies whether \a sep should be matched case
    sensitively or case insensitively.

    If \a behavior is QString::SkipEmptyParts, empty entries don't
    appear in the result. By default, empty entries are kept.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp 62

    \sa QStringList::join(), section()
*/
QStringList QString::split(const QString &sep, SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
   QStringList list;
   int start = 0;
   int extra = 0;
   int end;
   while ((end = indexOf(sep, start + extra, cs)) != -1) {
      if (start != end || behavior == KeepEmptyParts) {
         list.append(mid(start, end - start));
      }
      start = end + sep.size();
      extra = (sep.size() == 0 ? 1 : 0);
   }
   if (start != size() || behavior == KeepEmptyParts) {
      list.append(mid(start));
   }
   return list;
}

/*!
    \overload
*/
QStringList QString::split(QChar sep, SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
   QStringList list;
   int start = 0;
   int end;
   while ((end = indexOf(sep, start, cs)) != -1) {
      if (start != end || behavior == KeepEmptyParts) {
         list.append(mid(start, end - start));
      }
      start = end + 1;
   }
   if (start != size() || behavior == KeepEmptyParts) {
      list.append(mid(start));
   }
   return list;
}

#ifndef QT_NO_REGEXP
QStringList QString::split(const QRegExp &rx, SplitBehavior behavior) const
{
   QRegExp rx2(rx);

   QStringList list;
   int start = 0;
   int extra = 0;
   int end;

   while ((end = rx2.indexIn(*this, start + extra)) != -1) {
      int matchedLen = rx2.matchedLength();

      if (start != end || behavior == KeepEmptyParts) {
         list.append(mid(start, end - start));
      }

      start = end + matchedLen;
      extra = (matchedLen == 0) ? 1 : 0;
   }

   if (start != size() || behavior == KeepEmptyParts) {
      list.append(mid(start));
   }
   return list;
}
#endif

QString QString::repeated(int times) const
{
   if (d->size == 0) {
      return *this;
   }

   if (times <= 1) {
      if (times == 1) {
         return *this;
      }
      return QString();
   }

   const int resultSize = times * d->size;

   QString result;
   result.reserve(resultSize);
   if (result.d->alloc != uint(resultSize) + 1u) {
      return QString();   // not enough memory
   }

   memcpy(result.d->data(), d->data(), d->size * sizeof(ushort));

   int sizeSoFar = d->size;
   ushort *end = result.d->data() + sizeSoFar;

   const int halfResultSize = resultSize >> 1;
   while (sizeSoFar <= halfResultSize) {
      memcpy(end, result.d->data(), sizeSoFar * sizeof(ushort));
      end += sizeSoFar;
      sizeSoFar <<= 1;
   }
   memcpy(end, result.d->data(), (resultSize - sizeSoFar) * sizeof(ushort));
   result.d->data()[resultSize] = '\0';
   result.d->size = resultSize;
   return result;
}

void qt_string_normalize(QString *data, QString::NormalizationForm mode, QChar::UnicodeVersion version, int from)
{
   bool simple = true;
   const QChar *p = data->constData();
   int len = data->length();
   for (int i = from; i < len; ++i) {
      if (p[i].unicode() >= 0x80) {
         simple = false;
         break;
      }
   }
   if (simple) {
      return;
   }

   if (version == QChar::Unicode_Unassigned) {
      version = UNICODE_DATA_VERSION;

   } else if (version != UNICODE_DATA_VERSION) {
      const QString &s = *data;
      QChar *d = 0;

      for (int i = 0; i < QUnicodeTables::NumNormalizationCorrections; ++i) {
         const QUnicodeTables::NormalizationCorrection &n = QUnicodeTables::uc_normalization_corrections[i];

         if (n.version > version) {
            int pos = from;

            if (QChar::requiresSurrogates(n.ucs4)) {
               ushort ucs4High = QChar::highSurrogate(n.ucs4);
               ushort ucs4Low  = QChar::lowSurrogate(n.ucs4);
               ushort oldHigh  = QChar::highSurrogate(n.old_mapping);
               ushort oldLow   = QChar::lowSurrogate(n.old_mapping);

               while (pos < s.length() - 1) {
                  if (s.at(pos).unicode() == ucs4High && s.at(pos + 1).unicode() == ucs4Low) {
                     if (!d) {
                        d = data->data();
                     }
                     d[pos] = QChar(oldHigh);
                     d[++pos] = QChar(oldLow);
                  }
                  ++pos;
               }
            } else {
               while (pos < s.length()) {
                  if (s.at(pos).unicode() == n.ucs4) {
                     if (!d) {
                        d = data->data();
                     }
                     d[pos] = QChar(n.old_mapping);
                  }
                  ++pos;
               }
            }
         }
      }
   }
   decomposeHelper(data, mode < QString::NormalizationForm_KD, version, from);

   canonicalOrderHelper(data, version, from);

   if (mode == QString::NormalizationForm_D || mode == QString::NormalizationForm_KD) {
      return;
   }

   composeHelper(data, version, from);
}

/*!
    Returns the string in the given Unicode normalization \a mode,
    according to the given \a version of the Unicode standard.
*/
QString QString::normalized(QString::NormalizationForm mode, QChar::UnicodeVersion version) const
{
   QString copy = *this;
   qt_string_normalize(&copy, mode, version, 0);
   return copy;
}


struct ArgEscapeData {
   int min_escape;            // lowest escape sequence number
   int occurrences;           // number of occurrences of the lowest escape sequence number
   int locale_occurrences;    // number of occurrences of the lowest escape sequence number that
   // contain 'L'
   int escape_len;            // total length of escape sequences which will be replaced
};

static ArgEscapeData findArgEscapes(const QString &s)
{
   const QChar *uc_begin = s.unicode();
   const QChar *uc_end = uc_begin + s.length();

   ArgEscapeData d;

   d.min_escape = INT_MAX;
   d.occurrences = 0;
   d.escape_len = 0;
   d.locale_occurrences = 0;

   const QChar *c = uc_begin;
   while (c != uc_end) {
      while (c != uc_end && c->unicode() != '%') {
         ++c;
      }

      if (c == uc_end) {
         break;
      }
      const QChar *escape_start = c;
      if (++c == uc_end) {
         break;
      }

      bool locale_arg = false;
      if (c->unicode() == 'L') {
         locale_arg = true;
         if (++c == uc_end) {
            break;
         }
      }

      if (c->digitValue() == -1) {
         continue;
      }

      int escape = c->digitValue();
      ++c;

      if (c != uc_end && c->digitValue() != -1) {
         escape = (10 * escape) + c->digitValue();
         ++c;
      }

      if (escape > d.min_escape) {
         continue;
      }

      if (escape < d.min_escape) {
         d.min_escape = escape;
         d.occurrences = 0;
         d.escape_len = 0;
         d.locale_occurrences = 0;
      }

      ++d.occurrences;
      if (locale_arg) {
         ++d.locale_occurrences;
      }
      d.escape_len += c - escape_start;
   }
   return d;
}

static QString replaceArgEscapes(const QString &s, const ArgEscapeData &d, int field_width,
                                 const QString &arg, const QString &larg, QChar fillChar = QLatin1Char(' '))
{
   const QChar *uc_begin = s.unicode();
   const QChar *uc_end = uc_begin + s.length();

   int abs_field_width = qAbs(field_width);
   int result_len = s.length()
                    - d.escape_len
                    + (d.occurrences - d.locale_occurrences)
                    * qMax(abs_field_width, arg.length())
                    + d.locale_occurrences
                    * qMax(abs_field_width, larg.length());

   QString result(result_len, Qt::Uninitialized);
   QChar *result_buff = (QChar *) result.unicode();

   QChar *rc = result_buff;
   const QChar *c = uc_begin;
   int repl_cnt = 0;
   while (c != uc_end) {
      /* We don't have to check if we run off the end of the string with c,
         because as long as d.occurrences > 0 we KNOW there are valid escape
         sequences. */

      const QChar *text_start = c;

      while (c->unicode() != '%') {
         ++c;
      }

      const QChar *escape_start = c++;

      bool locale_arg = false;
      if (c->unicode() == 'L') {
         locale_arg = true;
         ++c;
      }

      int escape = c->digitValue();
      if (escape != -1) {
         if (c + 1 != uc_end && (c + 1)->digitValue() != -1) {
            escape = (10 * escape) + (c + 1)->digitValue();
            ++c;
         }
      }

      if (escape != d.min_escape) {
         memcpy(rc, text_start, (c - text_start)*sizeof(QChar));
         rc += c - text_start;
      } else {
         ++c;

         memcpy(rc, text_start, (escape_start - text_start)*sizeof(QChar));
         rc += escape_start - text_start;

         uint pad_chars;
         if (locale_arg) {
            pad_chars = qMax(abs_field_width, larg.length()) - larg.length();
         } else {
            pad_chars = qMax(abs_field_width, arg.length()) - arg.length();
         }

         if (field_width > 0) { // left padded
            for (uint i = 0; i < pad_chars; ++i) {
               (rc++)->unicode() = fillChar.unicode();
            }
         }

         if (locale_arg) {
            memcpy(rc, larg.unicode(), larg.length()*sizeof(QChar));
            rc += larg.length();
         } else {
            memcpy(rc, arg.unicode(), arg.length()*sizeof(QChar));
            rc += arg.length();
         }

         if (field_width < 0) { // right padded
            for (uint i = 0; i < pad_chars; ++i) {
               (rc++)->unicode() = fillChar.unicode();
            }
         }

         if (++repl_cnt == d.occurrences) {
            memcpy(rc, c, (uc_end - c)*sizeof(QChar));
            rc += uc_end - c;
            Q_ASSERT(rc - result_buff == result_len);
            c = uc_end;
         }
      }
   }
   Q_ASSERT(rc == result_buff + result_len);

   return result;
}

/*!
  Returns a copy of this string with the lowest numbered place marker
  replaced by string \a a, i.e., \c %1, \c %2, ..., \c %99.

  \a fieldWidth specifies the minimum amount of space that argument \a
  a shall occupy. If \a a requires less space than \a fieldWidth, it
  is padded to \a fieldWidth with character \a fillChar.  A positive
  \a fieldWidth produces right-aligned text. A negative \a fieldWidth
  produces left-aligned text.

  This example shows how we might create a \c status string for
  reporting progress while processing a list of files:

  \snippet doc/src/snippets/qstring/main.cpp 11

  First, \c arg(i) replaces \c %1. Then \c arg(total) replaces \c
  %2. Finally, \c arg(fileName) replaces \c %3.

  One advantage of using arg() over sprintf() is that the order of the
  numbered place markers can change, if the application's strings are
  translated into other languages, but each arg() will still replace
  the lowest numbered unreplaced place marker, no matter where it
  appears. Also, if place marker \c %i appears more than once in the
  string, the arg() replaces all of them.

  If there is no unreplaced place marker remaining, a warning message
  is output and the result is undefined. Place marker numbers must be
  in the range 1 to 99.
*/
QString QString::arg(const QString &a, int fieldWidth, QChar fillChar) const
{
   ArgEscapeData d = findArgEscapes(*this);

   if (d.occurrences == 0) {
      qWarning("Warning: QString::arg() Missing place marker '%%n'\n"
         "Format String: %s     Argument Value: %s\n", this->toLocal8Bit().data(), a.toLocal8Bit().data());

      return *this;
   }
   return replaceArgEscapes(*this, d, fieldWidth, a, a, fillChar);
}

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2) const
  \overload arg()

  This is the same as \c {str.arg(a1).arg(a2)}, except that the
  strings \a a1 and \a a2 are replaced in one pass. This can make a
  difference if \a a1 contains e.g. \c{%1}:

  \snippet doc/src/snippets/qstring/main.cpp 13
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3) const
  \overload arg()

  This is the same as calling \c str.arg(a1).arg(a2).arg(a3), except
  that the strings \a a1, \a a2 and \a a3 are replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4)}, except that the strings \a
  a1, \a a2, \a a3 and \a a4 are replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4, const QString& a5) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4).arg(a5)}, except that the strings
  \a a1, \a a2, \a a3, \a a4, and \a a5 are replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4, const QString& a5, const QString& a6) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4).arg(a5).arg(a6))}, except that
  the strings \a a1, \a a2, \a a3, \a a4, \a a5, and \a a6 are
  replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4, const QString& a5, const QString& a6, const QString& a7) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4).arg(a5).arg(a6).arg(a7)},
  except that the strings \a a1, \a a2, \a a3, \a a4, \a a5, \a a6,
  and \a a7 are replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4, const QString& a5, const QString& a6, const QString& a7, const QString& a8) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4).arg(a5).arg(a6).arg(a7).arg(a8)},
  except that the strings \a a1, \a a2, \a a3, \a a4, \a a5, \a a6, \a
  a7, and \a a8 are replaced in one pass.
*/

/*!
  \fn QString QString::arg(const QString& a1, const QString& a2, const QString& a3, const QString& a4, const QString& a5, const QString& a6, const QString& a7, const QString& a8, const QString& a9) const
  \overload arg()

  This is the same as calling \c
  {str.arg(a1).arg(a2).arg(a3).arg(a4).arg(a5).arg(a6).arg(a7).arg(a8).arg(a9)},
  except that the strings \a a1, \a a2, \a a3, \a a4, \a a5, \a a6, \a
  a7, \a a8, and \a a9 are replaced in one pass.
*/

/*! \fn QString QString::arg(int a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  The \a a argument is expressed in base \a base, which is 10 by
  default and must be between 2 and 36. For bases other than 10, \a a
  is treated as an unsigned integer.

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The '%' can be followed by an 'L', in which case the sequence is
  replaced with a localized representation of \a a. The conversion
  uses the default locale, set by QLocale::setDefault(). If no default
  locale was specified, the "C" locale is used. The 'L' flag is
  ignored if \a base is not 10.

  \snippet doc/src/snippets/qstring/main.cpp 12
  \snippet doc/src/snippets/qstring/main.cpp 14

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*! \fn QString QString::arg(uint a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*! \fn QString QString::arg(long a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a a argument is expressed in the given \a base, which is 10 by
  default and must be between 2 and 36.

  The '%' can be followed by an 'L', in which case the sequence is
  replaced with a localized representation of \a a. The conversion
  uses the default locale. The default locale is determined from the
  system's locale settings at application startup. It can be changed
  using QLocale::setDefault(). The 'L' flag is ignored if \a base is
  not 10.

  \snippet doc/src/snippets/qstring/main.cpp 12
  \snippet doc/src/snippets/qstring/main.cpp 14

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*! \fn QString QString::arg(ulong a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a to a string. The base must be between 2 and 36, with 8
  giving octal, 10 decimal, and 16 hexadecimal numbers.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*!
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/
QString QString::arg(qint64 a, int fieldWidth, int base, QChar fillChar) const
{
   ArgEscapeData d = findArgEscapes(*this);

   if (d.occurrences == 0) {
      qWarning("Warning: QString::arg() Missing place marker '%%n'\n"
               "Format String: %s     Argument Value: %lld\n", this->toLocal8Bit().data(), a);

      return *this;
   }

   unsigned flags = QLocalePrivate::NoFlags;
   if (fillChar == QLatin1Char('0')) {
      flags = QLocalePrivate::ZeroPadded;
   }

   QString arg;
   if (d.occurrences > d.locale_occurrences) {
      arg = QLocale::c().d()->longLongToString(a, -1, base, fieldWidth, flags);
   }

   QString locale_arg;
   if (d.locale_occurrences > 0) {
      QLocale locale;
      if (!locale.numberOptions() & QLocale::OmitGroupSeparator) {
         flags |= QLocalePrivate::ThousandsGroup;
      }
      locale_arg = locale.d()->longLongToString(a, -1, base, fieldWidth, flags);
   }

   return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg, fillChar);
}

/*!
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. \a base must be between 2 and 36, with 8
  giving octal, 10 decimal, and 16 hexadecimal numbers.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/
QString QString::arg(quint64 a, int fieldWidth, int base, QChar fillChar) const
{
   ArgEscapeData d = findArgEscapes(*this);

   if (d.occurrences == 0) {
      qWarning("Warning: QString::arg() Missing place marker '%%n'\n"
               "Format String: %s     Argument Value: %llu\n", this->toLocal8Bit().data(), a);

      return *this;
   }

   unsigned flags = QLocalePrivate::NoFlags;
   if (fillChar == QLatin1Char('0')) {
      flags = QLocalePrivate::ZeroPadded;
   }

   QString arg;
   if (d.occurrences > d.locale_occurrences) {
      arg = QLocale::c().d()->unsLongLongToString(a, -1, base, fieldWidth, flags);
   }

   QString locale_arg;
   if (d.locale_occurrences > 0) {
      QLocale locale;
      if (!locale.numberOptions() & QLocale::OmitGroupSeparator) {
         flags |= QLocalePrivate::ThousandsGroup;
      }
      locale_arg = locale.d()->unsLongLongToString(a, -1, base, fieldWidth, flags);
   }

   return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg, fillChar);
}

/*!
  \overload arg()

  \fn QString QString::arg(short a, int fieldWidth, int base, QChar fillChar) const

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*!
  \fn QString QString::arg(ushort a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  If \a fillChar is '0' (the number 0, ASCII 48), the locale's zero is
  used. For negative numbers, zero padding might appear before the
  minus sign.
*/

/*!
    \overload arg()
*/
QString QString::arg(QChar a, int fieldWidth, QChar fillChar) const
{
   QString c;
   c += a;
   return arg(c, fieldWidth, fillChar);
}

/*!
  \overload arg()

  The \a a argument is interpreted as a Latin-1 character.
*/
QString QString::arg(char a, int fieldWidth, QChar fillChar) const
{
   QString c;
   c += QLatin1Char(a);
   return arg(c, fieldWidth, fillChar);
}

QString QString::arg(double a, int fieldWidth, char fmt, int prec, QChar fillChar) const
{
   ArgEscapeData d = findArgEscapes(*this);

   if (d.occurrences == 0) {
      qWarning("Warning: QString::arg() Missing place marker '%%n'\n"
               "Format String: %s     Argument Value: %f\n", this->toLocal8Bit().data(), a);
      return *this;
   }

   unsigned flags = QLocalePrivate::NoFlags;
   if (fillChar == QLatin1Char('0')) {
      flags = QLocalePrivate::ZeroPadded;
   }

   if (qIsUpper(fmt)) {
      flags |= QLocalePrivate::CapitalEorX;
   }
   fmt = qToLower(fmt);

   QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
   switch (fmt) {
      case 'f':
         form = QLocalePrivate::DFDecimal;
         break;
      case 'e':
         form = QLocalePrivate::DFExponent;
         break;
      case 'g':
         form = QLocalePrivate::DFSignificantDigits;
         break;
      default:
#if defined(QT_CHECK_RANGE)
         qWarning("Warning: QString::arg() Invalid format char '%c'", fmt);
#endif
         break;
   }

   QString arg;
   if (d.occurrences > d.locale_occurrences) {
      arg = QLocale::c().d()->doubleToString(a, prec, form, fieldWidth, flags);
   }

   QString locale_arg;
   if (d.locale_occurrences > 0) {
      QLocale locale;

      if (!locale.numberOptions() & QLocale::OmitGroupSeparator) {
         flags |= QLocalePrivate::ThousandsGroup;
      }
      locale_arg = locale.d()->doubleToString(a, prec, form, fieldWidth, flags);
   }

   return replaceArgEscapes(*this, d, fieldWidth, arg, locale_arg, fillChar);
}

static int getEscape(const QChar *uc, int *pos, int len, int maxNumber = 999)
{
   int i = *pos;
   ++i;
   if (i < len && uc[i] == QLatin1Char('L')) {
      ++i;
   }
   if (i < len) {
      int escape = uc[i].unicode() - '0';
      if (uint(escape) >= 10U) {
         return -1;
      }
      ++i;
      while (i < len) {
         int digit = uc[i].unicode() - '0';
         if (uint(digit) >= 10U) {
            break;
         }
         escape = (escape * 10) + digit;
         ++i;
      }
      if (escape <= maxNumber) {
         *pos = i;
         return escape;
      }
   }
   return -1;
}

QString QString::multiArg(int numArgs, const QString **args) const
{
   QString result;
   QMap<int, int> numbersUsed;
   const QChar *uc = (const QChar *) d->data();
   const int len = d->size;
   const int end = len - 1;
   int lastNumber = -1;
   int i = 0;

   // populate the numbersUsed map with the %n's that actually occur in the string
   while (i < end) {
      if (uc[i] == QLatin1Char('%')) {
         int number = getEscape(uc, &i, len);
         if (number != -1) {
            numbersUsed.insert(number, -1);
            continue;
         }
      }
      ++i;
   }

   // assign an argument number to each of the %n's
   QMap<int, int>::iterator j = numbersUsed.begin();
   QMap<int, int>::iterator jend = numbersUsed.end();
   int arg = 0;
   while (j != jend && arg < numArgs) {
      *j = arg++;
      lastNumber = j.key();
      ++j;
   }

   // sanity
   if (numArgs > arg) {
      qWarning("Warning: QString::multiArg() Missing place marker '%%n'\n"
               "Format String: %s\n", this->toLocal8Bit().data());
      numArgs = arg;
   }

   i = 0;
   while (i < len) {
      if (uc[i] == QLatin1Char('%') && i != end) {
         int number = getEscape(uc, &i, len, lastNumber);
         int arg = numbersUsed[number];
         if (number != -1 && arg != -1) {
            result += *args[arg];
            continue;
         }
      }
      result += uc[i++];
   }
   return result;
}


/*! \fn bool QString::isSimpleText() const

    \internal
*/
bool QString::isSimpleText() const
{
   const ushort *p = d->data();
   const ushort *const end = p + d->size;
   while (p < end) {
      ushort uc = *p;
      // sort out regions of complex text formatting
      if (uc > 0x058f && (uc < 0x1100 || uc > 0xfb0f)) {
         return false;
      }
      p++;
   }

   return true;
}

/*! \fn bool QString::isRightToLeft() const

    Returns true if the string is read right to left.
*/
bool QString::isRightToLeft() const
{
   const ushort *p = d->data();
   const ushort *const end = p + d->size;
   while (p < end) {
      switch (QChar::direction(*p)) {
         case QChar::DirL:
            return false;
         case QChar::DirR:
         case QChar::DirAL:
            return true;
         default:
            break;
      }
      ++p;
   }
   return false;
}

QString QString::fromRawData(const QChar *unicode, int size)
{
   Data *x;
   if (!unicode) {
      x = Data::sharedNull();
   } else if (!size) {
      x = Data::allocate(0);
   } else {
      x = Data::fromRawData(reinterpret_cast<const ushort *>(unicode), size);
      Q_CHECK_PTR(x);
   }
   QStringDataPtr dataPtr = { x };
   return QString(dataPtr);
}

QString &QString::setRawData(const QChar *unicode, int size)
{
   if (d->ref.isShared() || d->alloc) {
      *this = fromRawData(unicode, size);
   } else {
      if (unicode) {
         d->size = size;
         d->offset = reinterpret_cast<const char *>(unicode) - reinterpret_cast<char *>(d);
      } else {
         d->offset = sizeof(QStringData);
         d->size = 0;
      }
   }
   return *this;
}

QDataStream &operator<<(QDataStream &out, const QString &str)
{
   if (! str.isNull()) {

      if ((out.byteOrder() == QDataStream::BigEndian) == (QSysInfo::ByteOrder == QSysInfo::BigEndian)) {
         out.writeBytes(reinterpret_cast<const char *>(str.unicode()), sizeof(QChar) * str.length());

      } else {
         QVarLengthArray<ushort> buffer(str.length());
         const ushort *data = reinterpret_cast<const ushort *>(str.constData());
         for (int i = 0; i < str.length(); i++) {
            buffer[i] = qbswap(*data);
            ++data;
         }
         out.writeBytes(reinterpret_cast<const char *>(buffer.data()), sizeof(ushort) * buffer.size());
      }

   } else {
      // write null marker
      out << (quint32)0xffffffff;
   }

   return out;
}

QDataStream &operator>>(QDataStream &in, QString &str)
{


   quint32 bytes = 0;
   in >> bytes;                                  // read size of string

   if (bytes == 0xffffffff) {                    // null string
      str.clear();

   } else if (bytes > 0) {                       // not empty
      if (bytes & 0x1) {
         str.clear();
         in.setStatus(QDataStream::ReadCorruptData);
         return in;
      }

      const quint32 Step = 1024 * 1024;
      quint32 len = bytes / 2;
      quint32 allocated = 0;

      while (allocated < len) {
         int blockSize = qMin(Step, len - allocated);
         str.resize(allocated + blockSize);
         if (in.readRawData(reinterpret_cast<char *>(str.data()) + allocated * 2,
                            blockSize * 2) != blockSize * 2) {
            str.clear();
            in.setStatus(QDataStream::ReadPastEnd);
            return in;
         }
         allocated += blockSize;
      }

      if ((in.byteOrder() == QDataStream::BigEndian)
            != (QSysInfo::ByteOrder == QSysInfo::BigEndian)) {
         ushort *data = reinterpret_cast<ushort *>(str.data());
         while (len--) {
            *data = qbswap(*data);
            ++data;
         }
      }

   } else {
      str = QString(QLatin1String(""));
   }

   return in;
}

QString QStringRef::toString() const
{
   if (!m_string) {
      return QString();
   }
   if (m_size && m_position == 0 && m_size == m_string->size()) {
      return *m_string;
   }
   return QString(m_string->unicode() + m_position, m_size);
}


/*! \relates QStringRef

   Returns true if string reference \a s1 is lexically equal to string reference \a s2; otherwise
   returns false.
*/
bool operator==(const QStringRef &s1, const QStringRef &s2)
{
   return (s1.size() == s2.size() &&
           qMemEquals((const ushort *)s1.unicode(), (const ushort *)s2.unicode(), s1.size()));
}

/*! \relates QStringRef

   Returns true if string \a s1 is lexically equal to string reference \a s2; otherwise
   returns false.
*/
bool operator==(const QString &s1, const QStringRef &s2)
{
   return (s1.size() == s2.size() &&
           qMemEquals((const ushort *)s1.unicode(), (const ushort *)s2.unicode(), s1.size()));
}

/*! \relates QStringRef

   Returns true if string  \a s1 is lexically equal to string reference \a s2; otherwise
   returns false.
*/
bool operator==(const QLatin1String &s1, const QStringRef &s2)
{
   if (s1.size() != s2.size()) {
      return false;
   }

   const ushort *uc = reinterpret_cast<const ushort *>(s2.unicode());
   const ushort *e = uc + s2.size();
   const uchar *c = reinterpret_cast<const uchar *>(s1.latin1());
   if (!c) {
      return s2.isEmpty();
   }

   while (*c) {
      if (uc == e || *uc != *c) {
         return false;
      }
      ++uc;
      ++c;
   }
   return (uc == e);
}

/*!
   \relates QStringRef

    Returns true if string reference \a s1 is lexically less than
    string reference \a s2; otherwise returns false.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings using the
    QString::localeAwareCompare() function.
*/
bool operator<(const QStringRef &s1, const QStringRef &s2)
{
   return ucstrcmp(s1.constData(), s1.length(), s2.constData(), s2.length()) < 0;
}

/*!\fn bool operator<=(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns true if string reference \a s1 is lexically less than
    or equal to string reference \a s2; otherwise returns false.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings using the
    QString::localeAwareCompare() function.
*/

/*!\fn bool operator>=(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns true if string reference \a s1 is lexically greater than
    or equal to string reference \a s2; otherwise returns false.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings using the
    QString::localeAwareCompare() function.
*/

/*!\fn bool operator>(const QStringRef &s1,const QStringRef &s2)

   \relates QStringRef

    Returns true if string reference \a s1 is lexically greater than
    string reference \a s2; otherwise returns false.

    The comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would
    expect. Consider sorting user-interface strings using the
    QString::localeAwareCompare() function.
*/


/*!
    \fn const QChar QStringRef::at(int position) const

    Returns the character at the given index \a position in the
    string reference.

    The \a position must be a valid index position in the string
    (i.e., 0 <= \a position < size()).
*/

/*!
    \fn void QStringRef::clear()

    Clears the contents of the string reference by making it null and empty.

    \sa isEmpty(), isNull()
*/

/*!
    \fn QStringRef &QStringRef::operator=(const QStringRef &other)

    Assigns the \a other string reference to this string reference, and
    returns the result.
*/

/*!
    \fn QStringRef &QStringRef::operator=(const QString *string)

    Constructs a string reference to the given \a string and assigns it to
    this string reference, returning the result.
*/

/*!
    \typedef QString::DataPtr
    \internal
*/

/*!
    \fn DataPtr & QString::data_ptr()
    \internal
*/



/*!  Appends the string reference to \a string, and returns a new
reference to the combined string data.
 */
QStringRef QStringRef::appendTo(QString *string) const
{
   if (!string) {
      return QStringRef();
   }
   int pos = string->size();
   string->insert(pos, unicode(), size());
   return QStringRef(string, pos, size());
}

/*!
    \fn int QStringRef::compare(const QStringRef &s1, const QString &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5

    Compares the string \a s1 with the string \a s2 and returns an
    integer less than, equal to, or greater than zero if \a s1
    is less than, equal to, or greater than \a s2.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \fn int QStringRef::compare(const QStringRef &s1, const QStringRef &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5
    \overload

    Compares the string \a s1 with the string \a s2 and returns an
    integer less than, equal to, or greater than zero if \a s1
    is less than, equal to, or greater than \a s2.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \fn int QStringRef::compare(const QStringRef &s1, QLatin1String s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
    \since 4.5
    \overload

    Compares the string \a s1 with the string \a s2 and returns an
    integer less than, equal to, or greater than zero if \a s1
    is less than, equal to, or greater than \a s2.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.
*/

/*!
    \overload
    \fn int QStringRef::compare(const QString &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.

    \sa QString::compare()
*/

/*!
    \overload
    \fn int QStringRef::compare(const QStringRef &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.

    \sa QString::compare()
*/

/*!
    \overload
    \fn int QStringRef::compare(QLatin1String other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 4.5

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Equivalent to \c {compare(*this, other, cs)}.

    \sa QString::compare()
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &s1, const QString & s2)
    \since 4.5

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    On Mac OS X, this function compares according the
    "Order for sorted lists" setting in the International prefereces panel.

    \sa compare(), QTextCodec::locale()
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &s1, const QStringRef & s2)
    \since 4.5
    \overload

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

*/

/*!
    \fn int QStringRef::localeAwareCompare(const QString &other) const
    \since 4.5
    \overload

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.
*/

/*!
    \fn int QStringRef::localeAwareCompare(const QStringRef &other) const
    \since 4.5
    \overload

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.
*/

/*!
    \fn QString &QString::append(const QStringRef &reference)
    \since 4.4

    Appends the given string \a reference to this string and returns the result.
 */
QString &QString::append(const QStringRef &str)
{
   if (str.string() == this) {
      str.appendTo(this);
   } else if (str.string()) {
      int oldSize = size();
      resize(oldSize + str.size());
      memcpy(data() + oldSize, str.unicode(), str.size() * sizeof(QChar));
   }
   return *this;
}

/*!
    \since 4.4

    Returns a substring reference to the \a n leftmost characters
    of the string.

    If \a n is greater than size() or less than zero, a reference to the entire
    string is returned.

    \snippet doc/src/snippets/qstring/main.cpp leftRef

    \sa left(), rightRef(), midRef(), startsWith()
*/
QStringRef QString::leftRef(int n)  const
{
   if (n >= d->size || n < 0) {
      n = d->size;
   }
   return QStringRef(this, 0, n);
}

/*!
    \since 4.4

    Returns a substring reference to the \a n rightmost characters
    of the string.

    If \a n is greater than size() or less than zero, a reference to the entire
    string is returned.

    \snippet doc/src/snippets/qstring/main.cpp rightRef

    \sa right(), leftRef(), midRef(), endsWith()
*/
QStringRef QString::rightRef(int n) const
{
   if (n >= d->size || n < 0) {
      n = d->size;
   }
   return QStringRef(this, d->size - n, n);
}

/*!
    \since 4.4

    Returns a substring reference to \a n characters of this string,
    starting at the specified \a position.

    If the \a position exceeds the length of the string, a null
    reference is returned.

    If there are less than \a n characters available in the string,
    starting at the given \a position, or if \a n is -1 (default), the
    function returns all characters from the specified \a position
    onwards.

    Example:

    \snippet doc/src/snippets/qstring/main.cpp midRef

    \sa mid(), leftRef(), rightRef()
*/

QStringRef QString::midRef(int position, int n) const
{
   if (position > d->size) {
      return QStringRef();
   }
   if (position < 0) {
      if (n < 0 || n + position >= d->size) {
         return QStringRef(this, 0, d->size);
      }
      if (n + position <= 0) {
         return QStringRef();
      }

      n += position;
      position = 0;
   } else if (n < 0 || n > d->size - position) {
      n = d->size - position;
   }
   return QStringRef(this, position, n);
}

/*!
  \since 4.8

  Returns the index position of the first occurrence of the string \a
  str in this string reference, searching forward from index position
  \a from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
   return qFindString(unicode(), length(), from, str.unicode(), str.length(), cs);
}

/*!
    \since 4.8
    \overload indexOf()

    Returns the index position of the first occurrence of the
    character \a ch in the string reference, searching forward from
    index position \a from. Returns -1 if \a ch could not be found.

    \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
   return findChar(unicode(), length(), ch, from, cs);
}

/*!
  \since 4.8

  Returns the index position of the first occurrence of the string \a
  str in this string reference, searching forward from index position
  \a from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(QLatin1String str, int from, Qt::CaseSensitivity cs) const
{
   return qt_find_latin1_string(unicode(), size(), str, from, cs);
}

/*!
    \since 4.8

    \overload indexOf()

    Returns the index position of the first occurrence of the string
    reference \a str in this string reference, searching forward from
    index position \a from. Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::indexOf(), lastIndexOf(), contains(), count()
*/
int QStringRef::indexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
   return qFindString(unicode(), size(), from, str.unicode(), str.size(), cs);
}

/*!
  \since 4.8

  Returns the index position of the last occurrence of the string \a
  str in this string reference, searching backward from index position
  \a from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(const QString &str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.size();
   if (sl == 1) {
      return lastIndexOf(str.at(0), from, cs);
   }

   const int l = size();;
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   return lastIndexOfHelper(reinterpret_cast<const ushort *>(unicode()), from,
                            reinterpret_cast<const ushort *>(str.unicode()), str.size(), cs);
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the character
  \a ch, searching backward from position \a from.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(QChar ch, int from, Qt::CaseSensitivity cs) const
{
   return qt_last_index_of(unicode(), size(), ch, from, cs);
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string reference, searching backward from index position
  \a from. If \a from is -1 (default), the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(QLatin1String str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.size();
   if (sl == 1) {
      return lastIndexOf(QLatin1Char(str.latin1()[0]), from, cs);
   }

   const int l = size();
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   QVarLengthArray<ushort> s(sl);
   for (int i = 0; i < sl; ++i) {
      s[i] = str.latin1()[i];
   }

   return lastIndexOfHelper(reinterpret_cast<const ushort *>(unicode()), from, s.data(), sl, cs);
}

/*!
  \since 4.8
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string
  reference \a str in this string reference, searching backward from
  index position \a from. If \a from is -1 (default), the search
  starts at the last character; if \a from is -2, at the next to last
  character and so on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa QString::lastIndexOf(), indexOf(), contains(), count()
*/
int QStringRef::lastIndexOf(const QStringRef &str, int from, Qt::CaseSensitivity cs) const
{
   const int sl = str.size();
   if (sl == 1) {
      return lastIndexOf(str.at(0), from, cs);
   }

   const int l = size();
   if (from < 0) {
      from += l;
   }
   int delta = l - sl;
   if (from == l && sl == 0) {
      return from;
   }
   if (from < 0 || from >= l || delta < 0) {
      return -1;
   }
   if (from > delta) {
      from = delta;
   }

   return lastIndexOfHelper(reinterpret_cast<const ushort *>(unicode()), from,
                            reinterpret_cast<const ushort *>(str.unicode()),
                            str.size(), cs);
}

/*!
    \since 4.8
    Returns the number of (potentially overlapping) occurrences of
    the string \a str in this string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(const QString &str, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), str.unicode(), str.size(), cs);
}

/*!
    \since 4.8
    \overload count()

    Returns the number of occurrences of the character \a ch in the
    string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(QChar ch, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), ch, cs);
}

/*!
    \since 4.8
    \overload count()

    Returns the number of (potentially overlapping) occurrences of the
    string reference \a str in this string reference.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::count(), contains(), indexOf()
*/
int QStringRef::count(const QStringRef &str, Qt::CaseSensitivity cs) const
{
   return qt_string_count(unicode(), size(), str.unicode(), str.size(), cs);
}

/*!
    \since 4.8

    Returns true if the string reference starts with \a str; otherwise
    returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(const QString &str, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(),
                         str.isNull() ? 0 : str.unicode(), str.size(), cs);
}

/*!
    \since 4.8
    \overload startsWith()
    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(QLatin1String str, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(), str, cs);
}

/*!
    \since 4.8
    \overload startsWith()
    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(const QStringRef &str, Qt::CaseSensitivity cs) const
{
   return qt_starts_with(isNull() ? 0 : unicode(), size(),
                         str.isNull() ? 0 : str.unicode(), str.size(), cs);
}

/*!
    \since 4.8
    \overload startsWith()

    Returns true if the string reference starts with \a ch; otherwise
    returns false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::startsWith(), endsWith()
*/
bool QStringRef::startsWith(QChar ch, Qt::CaseSensitivity cs) const
{
   if (!isEmpty()) {
      const ushort *data = reinterpret_cast<const ushort *>(unicode());
      return (cs == Qt::CaseSensitive
              ? data[0] == ch
              : foldCase(data[0]) == foldCase(ch.unicode()));
   } else {
      return false;
   }
}

/*!
    \since 4.8
    Returns true if the string reference ends with \a str; otherwise
    returns false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::endsWith(), startsWith()
*/
bool QStringRef::endsWith(const QString &str, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(),
                       str.isNull() ? 0 : str.unicode(), str.size(), cs);
}

/*!
    \since 4.8
    \overload endsWith()

    Returns true if the string reference ends with \a ch; otherwise
    returns false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(QChar ch, Qt::CaseSensitivity cs) const
{
   if (!isEmpty()) {
      const ushort *data = reinterpret_cast<const ushort *>(unicode());
      const int size = length();
      return (cs == Qt::CaseSensitive
              ? data[size - 1] == ch
              : foldCase(data[size - 1]) == foldCase(ch.unicode()));
   } else {
      return false;
   }
}

/*!
    \since 4.8
    \overload endsWith()
    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(QLatin1String str, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(), str, cs);
}

/*!
    \since 4.8
    \overload endsWith()
    \sa QString::endsWith(), endsWith()
*/
bool QStringRef::endsWith(const QStringRef &str, Qt::CaseSensitivity cs) const
{
   return qt_ends_with(isNull() ? 0 : unicode(), size(),
                       str.isNull() ? 0 : str.unicode(), str.size(), cs);
}


/*! \fn bool QStringRef::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \since 4.8
    Returns true if this string reference contains an occurrence of
    the string \a str; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QStringRef::contains(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \overload contains()
    \since 4.8

    Returns true if this string contains an occurrence of the
    character \a ch; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

*/

/*! \fn bool QStringRef::contains(const QStringRef &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \overload contains()
    \since 4.8

    Returns true if this string reference contains an occurrence of
    the string reference \a str; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

/*! \fn bool QStringRef::contains(QLatin1String str, Qt::CaseSensitivity cs) const
    \since 4,8
    \overload contains()

    Returns true if this string reference contains an occurrence of
    the string \a str; otherwise returns false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

static inline int qt_last_index_of(const QChar *haystack, int haystackLen, QChar needle,
                                   int from, Qt::CaseSensitivity cs)
{
   ushort c = needle.unicode();
   if (from < 0) {
      from += haystackLen;
   }
   if (from < 0 || from >= haystackLen) {
      return -1;
   }
   if (from >= 0) {
      const ushort *b = reinterpret_cast<const ushort *>(haystack);
      const ushort *n = b + from;
      if (cs == Qt::CaseSensitive) {
         for (; n >= b; --n)
            if (*n == c) {
               return n - b;
            }
      } else {
         c = foldCase(c);
         for (; n >= b; --n)
            if (foldCase(*n) == c) {
               return n - b;
            }
      }
   }
   return -1;


}

static inline int qt_string_count(const QChar *haystack, int haystackLen,
                  const QChar *needle, int needleLen, Qt::CaseSensitivity cs)
{
   int num = 0;
   int i = -1;

   while ((i = qFindString(haystack, haystackLen, i + 1, needle, needleLen, cs)) != -1) {
      ++num;
   }

   return num;
}

static inline int qt_string_count(const QChar *unicode, int size, QChar ch,
                  Qt::CaseSensitivity cs)
{
   ushort c = ch.unicode();
   int num = 0;
   const ushort *b = reinterpret_cast<const ushort *>(unicode);
   const ushort *i = b + size;
   if (cs == Qt::CaseSensitive) {
      while (i != b)
         if (*--i == c) {
            ++num;
         }
   } else {
      c = foldCase(c);
      while (i != b)
         if (foldCase(*(--i)) == c) {
            ++num;
         }
   }
   return num;
}

static inline int qt_find_latin1_string(const QChar *haystack, int size,
                                        const QLatin1String &needle,
                                        int from, Qt::CaseSensitivity cs)
{
   const char *latin1 = needle.latin1();
   int len = needle.size();
   QVarLengthArray<ushort> s(len);
   for (int i = 0; i < len; ++i) {
      s[i] = latin1[i];
   }

   return qFindString(haystack, size, from,
                      reinterpret_cast<const QChar *>(s.constData()), len, cs);
}

static inline bool qt_starts_with(const QChar *haystack, int haystackLen,
                                  const QChar *needle, int needleLen, Qt::CaseSensitivity cs)
{
   if (!haystack) {
      return !needle;
   }
   if (haystackLen == 0) {
      return needleLen == 0;
   }
   if (needleLen > haystackLen) {
      return false;
   }

   const ushort *h = reinterpret_cast<const ushort *>(haystack);
   const ushort *n = reinterpret_cast<const ushort *>(needle);

   if (cs == Qt::CaseSensitive) {
      return qMemEquals(h, n, needleLen);
   } else {
      uint last = 0;
      uint olast = 0;
      for (int i = 0; i < needleLen; ++i)
         if (foldCase(h[i], last) != foldCase(n[i], olast)) {
            return false;
         }
   }
   return true;
}

static inline bool qt_starts_with(const QChar *haystack, int haystackLen,
                                  const QLatin1String &needle, Qt::CaseSensitivity cs)
{
   if (!haystack) {
      return !needle.latin1();
   }
   if (haystackLen == 0) {
      return !needle.latin1() || *needle.latin1() == 0;
   }
   const int slen = needle.size();
   if (slen > haystackLen) {
      return false;
   }
   const ushort *data = reinterpret_cast<const ushort *>(haystack);
   const uchar *latin = reinterpret_cast<const uchar *>(needle.latin1());
   if (cs == Qt::CaseSensitive) {
      for (int i = 0; i < slen; ++i)
         if (data[i] != latin[i]) {
            return false;
         }
   } else {
      for (int i = 0; i < slen; ++i)
         if (foldCase(data[i]) != foldCase((ushort)latin[i])) {
            return false;
         }
   }
   return true;
}

static inline bool qt_ends_with(const QChar *haystack, int haystackLen,
                                const QChar *needle, int needleLen, Qt::CaseSensitivity cs)
{
   if (!haystack) {
      return !needle;
   }
   if (haystackLen == 0) {
      return needleLen == 0;
   }
   const int pos = haystackLen - needleLen;
   if (pos < 0) {
      return false;
   }

   const ushort *h = reinterpret_cast<const ushort *>(haystack);
   const ushort *n = reinterpret_cast<const ushort *>(needle);

   if (cs == Qt::CaseSensitive) {
      return qMemEquals(h + pos, n, needleLen);
   } else {
      uint last = 0;
      uint olast = 0;
      for (int i = 0; i < needleLen; i++)
         if (foldCase(h[pos + i], last) != foldCase(n[i], olast)) {
            return false;
         }
   }
   return true;
}


static inline bool qt_ends_with(const QChar *haystack, int haystackLen,
                                const QLatin1String &needle, Qt::CaseSensitivity cs)
{
   if (!haystack) {
      return !needle.latin1();
   }
   if (haystackLen == 0) {
      return !needle.latin1() || *needle.latin1() == 0;
   }
   const int slen = needle.size();
   int pos = haystackLen - slen;
   if (pos < 0) {
      return false;
   }
   const uchar *latin = reinterpret_cast<const uchar *>(needle.latin1());
   const ushort *data = reinterpret_cast<const ushort *>(haystack);
   if (cs == Qt::CaseSensitive) {
      for (int i = 0; i < slen; i++)
         if (data[pos + i] != latin[i]) {
            return false;
         }
   } else {
      for (int i = 0; i < slen; i++)
         if (foldCase(data[pos + i]) != foldCase((ushort)latin[i])) {
            return false;
         }
   }
   return true;
}

QByteArray QStringRef::toLatin1() const
{
   return toLatin1_helper(unicode(), length());
}

QByteArray QStringRef::toAscii() const
{
   return toLatin1();
}

QByteArray QStringRef::toLocal8Bit() const
{
#ifndef QT_NO_TEXTCODEC
   if (QTextCodec::codecForLocale()) {
      return QTextCodec::codecForLocale()->fromUnicode(unicode(), length());
   }
#endif // QT_NO_TEXTCODEC
   return toLatin1();
}

QByteArray QStringRef::toUtf8() const
{
   if (isNull()) {
      return QByteArray();
   }

   return QUtf8::convertFromUnicode(constData(), length(), 0);
}

QVector<uint> QStringRef::toUcs4() const
{
   QVector<uint> v(length());
   uint *a = v.data();
   int len = QString::toUcs4_helper(reinterpret_cast<const ushort *>(unicode()), length(), a);
   v.resize(len);
   return v;
}

QString QString::toHtmlEscaped() const
{
   QString rich;
   const int len = length();
   rich.reserve(int(len * 1.1));
   for (int i = 0; i < len; ++i) {
      if (at(i) == QLatin1Char('<')) {
         rich += QLatin1String("&lt;");
      } else if (at(i) == QLatin1Char('>')) {
         rich += QLatin1String("&gt;");
      } else if (at(i) == QLatin1Char('&')) {
         rich += QLatin1String("&amp;");
      } else if (at(i) == QLatin1Char('"')) {
         rich += QLatin1String("&quot;");
      } else {
         rich += at(i);
      }
   }
   rich.squeeze();
   return rich;
}
