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

#include <qchar32.h>
#include <qdatastream.h>

//   #include <qtextcodec.h>
//   #include <qunicodetables_p.h>
//   #include "qunicodetables.cpp"         // do not change to < >

#define FLAG(x) (1 << (x))

// methods


// operators

#if ! defined(QT_NO_DATASTREAM)
   QDataStream &operator>>(QDataStream &out, QChar32 &str)
   {
      // broom - not implemented
      return out;
   }

   QDataStream &operator<<(QDataStream &out, const QChar32 &str)
   {
      // broom - not implemented
      return out;
   }
#endif

