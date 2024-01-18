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

#ifndef QHEXSTRING_P_H
#define QHEXSTRING_P_H

#include <qstring.h>

// converts an integer or double to an unique string token
template <typename T>
struct HexString {
   HexString(const T t)
      : m_data(t)
   { }

   QString toString() const {
      return QString("%1").formatArg(m_data, sizeof(T) * 2, 16, '0');
   }

   const T m_data;
};

template <>
inline QString HexString<double>::toString() const
{

   uchar buffer[sizeof(double)];
   memcpy(buffer, &m_data, sizeof(double));

   QString retval;

   for (std::size_t i = 0; i < sizeof(double); ++i) {
      retval += QString("%1").formatArg(buffer[i], 2, 16, '0');
   }

   return retval;
}

template <>
inline QString HexString<float>::toString() const
{

   uchar buffer[sizeof(float)];
   memcpy(buffer, &m_data, sizeof(float));

   QString retval;

   for (std::size_t i = 0; i < sizeof(float); ++i) {
      retval += QString("%1").formatArg(buffer[i], 2, 16, '0');
   }

   return retval;
}

template <typename T>
QString operator+(QString str, HexString<T> hex)
{
   return std::move(str) + hex.toString();
}

template <typename T>
QString operator+(HexString<T> hex, const QString &str)
{
   return hex.toString() + str;
}


#endif
