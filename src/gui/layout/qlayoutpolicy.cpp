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

#include <qlayoutpolicy_p.h>
#include <qdebug.h>
#include <qdatastream.h>

void QLayoutPolicy::setControlType(ControlType type)
{
   /*
       The control type is a flag type, with values 0x1, 0x2, 0x4, 0x8, 0x10,
       etc. In memory, we pack it onto the available bits (CTSize) in
       setControlType(), and unpack it here.

       Example:

           0x00000001 maps to 0
           0x00000002 maps to 1
           0x00000004 maps to 2
           0x00000008 maps to 3
           etc.
   */

   int i = 0;
   while (true) {
      if (type & (0x1 << i)) {
         bits.ctype = i;
         return;
      }
      ++i;
   }
}

QLayoutPolicy::ControlType QLayoutPolicy::controlType() const
{
   return QLayoutPolicy::ControlType(1 << bits.ctype);
}

QDataStream &operator<<(QDataStream &stream, const QLayoutPolicy &policy)
{
   // order here is for historical reasons

   quint32 data = (policy.bits.horPolicy |         // [0, 3]
         policy.bits.verPolicy << 4 |    // [4, 7]
         policy.bits.hfw << 8 |          // [8]
         policy.bits.ctype << 9 |        // [9, 13]
         policy.bits.wfh << 14 |         // [14]
         //policy.bits.padding << 15 |     // [15]
         policy.bits.verStretch << 16 |  // [16, 23]
         policy.bits.horStretch << 24);  // [24, 31]
   return stream << data;
}

#define VALUE_OF_BITS(data, bitstart, bitcount) ((data >> bitstart) & ((1 << bitcount) -1))

QDataStream &operator>>(QDataStream &stream, QLayoutPolicy &policy)
{
   quint32 data;
   stream >> data;
   policy.bits.horPolicy =  VALUE_OF_BITS(data, 0, 4);
   policy.bits.verPolicy =  VALUE_OF_BITS(data, 4, 4);
   policy.bits.hfw =        VALUE_OF_BITS(data, 8, 1);
   policy.bits.ctype =      VALUE_OF_BITS(data, 9, 5);
   policy.bits.wfh =        VALUE_OF_BITS(data, 14, 1);
   policy.bits.padding =   0;
   policy.bits.verStretch = VALUE_OF_BITS(data, 16, 8);
   policy.bits.horStretch = VALUE_OF_BITS(data, 24, 8);
   return stream;
}

QDebug operator<<(QDebug dbg, const QLayoutPolicy &p)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QLayoutPolicy(horizontalPolicy = " << p.horizontalPolicy()
      << ", verticalPolicy = " << p.verticalPolicy() << ')';
   return dbg;
}


