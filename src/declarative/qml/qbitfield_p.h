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

#ifndef QBITFIELD_P_H
#define QBITFIELD_P_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QBitField
{
 public:
   inline QBitField();
   inline QBitField(const quint32 *, int bits);
   inline QBitField(const QBitField &);
   inline ~QBitField();

   inline QBitField &operator=(const QBitField &);

   inline quint32 size() const;
   inline QBitField united(const QBitField &);
   inline bool testBit(int) const;

 private:
   quint32 bits: 31;
   quint32 *ownData;
   const quint32 *data;
};

QBitField::QBitField()
   : bits(0), ownData(0), data(0)
{
}

QBitField::QBitField(const quint32 *bitData, int bitCount)
   : bits((quint32)bitCount), ownData(0), data(bitData)
{
}

QBitField::QBitField(const QBitField &other)
   : bits(other.bits), ownData(other.ownData), data(other.data)
{
   if (ownData) {
      ++(*ownData);
   }
}

QBitField::~QBitField()
{
   if (ownData)
      if (0 == --(*ownData)) {
         delete [] ownData;
      }
}

QBitField &QBitField::operator=(const QBitField &other)
{
   if (other.data == data) {
      return *this;
   }

   if (ownData)
      if (0 == --(*ownData)) {
         delete [] ownData;
      }

   bits = other.bits;
   ownData = other.ownData;
   data = other.data;

   if (ownData) {
      ++(*ownData);
   }

   return *this;
}

inline quint32 QBitField::size() const
{
   return bits;
}

QBitField QBitField::united(const QBitField &o)
{
   if (o.bits == 0) {
      return *this;
   } else if (bits == 0) {
      return o;
   } else {
      int max = (bits > o.bits) ? bits : o.bits;
      int length = (max + 31) / 32;
      QBitField rv;
      rv.bits = max;
      rv.ownData = new quint32[length + 1];
      *(rv.ownData) = 1;
      rv.data = rv.ownData + 1;
      if (bits > o.bits) {
         ::memcpy((quint32 *)rv.data, data, length * sizeof(quint32));
         for (quint32 ii = 0; ii < (o.bits + quint32(31)) / 32; ++ii) {
            ((quint32 *)rv.data)[ii] |= o.data[ii];
         }
      } else {
         ::memcpy((quint32 *)rv.data, o.data, length * sizeof(quint32));
         for (quint32 ii = 0; ii < (bits + quint32(31)) / 32; ++ii) {
            ((quint32 *)rv.data)[ii] |= data[ii];
         }
      }
      return rv;
   }
}

bool QBitField::testBit(int b) const
{
   Q_ASSERT(b >= 0);
   if ((quint32)b < bits) {
      return data[b / 32] & (1 << (b % 32));
   } else {
      return false;
   }
}

QT_END_NAMESPACE

#endif // QBITFIELD_P_H
