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

#ifndef QENDIAN_H
#define QENDIAN_H

#include <qglobal.h>

// include stdlib.h and hope that it defines __GLIBC__ for glibc-based systems
#include <stdlib.h>

#ifdef __GLIBC__
#include <byteswap.h>
#endif

inline void qbswap_helper(const uchar *source, uchar *dest, int size)
{
   for (int i = 0; i < size ; ++i) {
      dest[i] = source[size - 1 - i];
   }
}

template <typename T>
inline void qbswap(const T src, uchar *dest)
{
   qbswap_helper(reinterpret_cast<const uchar *>(&src), dest, sizeof(T));
}

// Used to implement a type-safe and alignment-safe copy operation
// If you want to avoid the memcopy you must write specializations for this function
template <typename T>
inline void qToUnaligned(const T src, uchar *dest)
{
   memcpy(dest, &src, sizeof(T));
}

template <typename T>
inline T qFromUnaligned(const uchar *src)
{
    T dest;
    memcpy(&dest, src, sizeof(T));

    return dest;
}

/* T qFromLittleEndian(const uchar *src)
 * This function will read a little-endian encoded value from \a src and return the value in host-endian encoding.
 * There is no requirement that \a src must be aligned.
*/

template <typename T>
inline T qFromLittleEndian(const uchar *source)
{
   static_assert(std::is_integral<T>::value, "Data type for T must be an integer");
   static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "T must be a 16-bit, 32-bit, or 64-bit integer");

   if constexpr (sizeof(T) == 8) {
      // 64
      quint64 retval = 0
             | source[0]
             | source[1] * Q_UINT64_C(0x0000000000000100)
             | source[2] * Q_UINT64_C(0x0000000000010000)
             | source[3] * Q_UINT64_C(0x0000000001000000)
             | source[4] * Q_UINT64_C(0x0000000100000000)
             | source[5] * Q_UINT64_C(0x0000010000000000)
             | source[6] * Q_UINT64_C(0x0001000000000000)
             | source[7] * Q_UINT64_C(0x0100000000000000);

      return static_cast<T>(retval);

   } else if constexpr (sizeof(T) == 4) {
      // 32
      quint32 retval = 0
             | source[0]
             | source[1] * quint32(0x00000100)
             | source[2] * quint32(0x00010000)
             | source[3] * quint32(0x01000000);

      return static_cast<T>(retval);

   } else  {
      // 16
      quint16 retval = 0
             | source[0]
             | source[1] * 0x0100;

      return static_cast<T>(retval);
   }
}


/* This function will read a big-endian (also known as network order) encoded value from \a src
 * and return the value in host-endian encoding.There is no requirement that \a src must be aligned.
*/

template <typename T>
inline T qFromBigEndian(const uchar *source)
{
   static_assert(std::is_integral<T>::value, "Data type for T must be an integer");
   static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "T must be a 16-bit, 32-bit, or 64-bit integer");

   if constexpr (sizeof(T) == 8) {
      // 64
      quint64 retval = 0
             | source[7]
             | source[6] * Q_UINT64_C(0x0000000000000100)
             | source[5] * Q_UINT64_C(0x0000000000010000)
             | source[4] * Q_UINT64_C(0x0000000001000000)
             | source[3] * Q_UINT64_C(0x0000000100000000)
             | source[2] * Q_UINT64_C(0x0000010000000000)
             | source[1] * Q_UINT64_C(0x0001000000000000)
             | source[0] * Q_UINT64_C(0x0100000000000000);

      return static_cast<T>(retval);

   } else if constexpr (sizeof(T) == 4) {
      // 32
      quint32 retval = 0
             | source[3]
             | source[2] * quint32(0x00000100)
             | source[1] * quint32(0x00010000)
             | source[0] * quint32(0x01000000);

      return static_cast<T>(retval);

   } else  {
      // 16
      quint16 retval = 0
             | source[1]
             | source[0] * 0x0100;

      return static_cast<T>(retval);
   }
}

template <typename T>
T qbswap(T source);


#ifdef __GLIBC__
template <>
inline quint64 qbswap<quint64>(quint64 source)
{
   return bswap_64(source);
}

template <>
inline quint32 qbswap<quint32>(quint32 source)
{
   return bswap_32(source);
}

template <>
inline quint16 qbswap<quint16>(quint16 source)
{
   return bswap_16(source);
}

#else

template <>
inline quint64 qbswap<quint64>(quint64 source)
{
   return 0
          | ((source & Q_UINT64_C(0x00000000000000ff)) << 56)
          | ((source & Q_UINT64_C(0x000000000000ff00)) << 40)
          | ((source & Q_UINT64_C(0x0000000000ff0000)) << 24)
          | ((source & Q_UINT64_C(0x00000000ff000000)) << 8)
          | ((source & Q_UINT64_C(0x000000ff00000000)) >> 8)
          | ((source & Q_UINT64_C(0x0000ff0000000000)) >> 24)
          | ((source & Q_UINT64_C(0x00ff000000000000)) >> 40)
          | ((source & Q_UINT64_C(0xff00000000000000)) >> 56);
}

template <>
inline quint32 qbswap<quint32>(quint32 source)
{
   return 0
          | ((source & 0x000000ff) << 24)
          | ((source & 0x0000ff00) << 8)
          | ((source & 0x00ff0000) >> 8)
          | ((source & 0xff000000) >> 24);
}

template <>
inline quint16 qbswap<quint16>(quint16 source)
{
   return quint16( 0
                   | ((source & 0x00ff) << 8)
                   | ((source & 0xff00) >> 8) );
}
#endif // __GLIBC__


// signed specializations
template <>
inline qint64 qbswap<qint64>(qint64 source)
{
   return qbswap<quint64>(quint64(source));
}

template <>
inline qint32 qbswap<qint32>(qint32 source)
{
   return qbswap<quint32>(quint32(source));
}

template <>
inline qint16 qbswap<qint16>(qint16 source)
{
   return qbswap<quint16>(quint16(source));
}

#if Q_BYTE_ORDER == Q_BIG_ENDIAN

template <typename T>
inline T qToBigEndian(T source)
{
   return source;
}

template <typename T>
inline T qFromBigEndian(T source)
{
   return source;
}

template <typename T>
inline T qToLittleEndian(T source)
{
   return qbswap<T>(source);
}

template <typename T>
inline T qFromLittleEndian(T source)
{
   return qbswap<T>(source);
}

template <typename T>
inline void qToBigEndian(T source, uchar *dest)
{
   qToUnaligned<T>(source, dest);
}

template <typename T>
inline void qToLittleEndian(T source, uchar *dest)
{
   qbswap<T>(source, dest);
}

#else // Q_LITTLE_ENDIAN

template <typename T>
inline T qToBigEndian(T source)
{
   return qbswap<T>(source);
}

template <typename T>
inline T qFromBigEndian(T source)
{
   return qbswap<T>(source);
}

template <typename T>
inline T qToLittleEndian(T source)
{
   return source;
}

template <typename T>
inline T qFromLittleEndian(T source)
{
   return source;
}

template <typename T>
inline void qToBigEndian(T source, uchar *dest)
{
   qbswap<T>(source, dest);
}

template <typename T>
inline void qToLittleEndian(T source, uchar *dest)
{
   qToUnaligned<T>(source, dest);
}

#endif // Q_BYTE_ORDER == Q_BIG_ENDIAN

template <>
inline quint8 qbswap<quint8>(quint8 source)
{
   return source;
}

#endif