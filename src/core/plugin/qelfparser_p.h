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

#ifndef QELFPARSER_P_H
#define QELFPARSER_P_H

#include <qendian.h>
#include <qglobal.h>

#if defined (Q_OF_ELF) && defined(Q_CC_GNU)

QT_BEGIN_NAMESPACE

class QString;
class QLibraryPrivate;

typedef quint16  qelfhalf_t;
typedef quint32  qelfword_t;
typedef quintptr qelfoff_t;
typedef quintptr qelfaddr_t;

class QElfParser
{
 public:
   enum {Ok = 0, NotElf = 1, NoQtSection = 2, Corrupt = 3};
   enum {ElfLittleEndian = 0, ElfBigEndian = 1};

   struct ElfSectionHeader {
      qelfword_t name;
      qelfword_t type;
      qelfoff_t  offset;
      qelfoff_t  size;
   };

   int m_endian;
   int m_bits;
   int m_stringTableFileOffset;

   template <typename T>
   T read(const char *s) {
      if (m_endian == ElfBigEndian) {
         return qFromBigEndian<T>(reinterpret_cast<const uchar *>(s));
      } else {
         return qFromLittleEndian<T>(reinterpret_cast<const uchar *>(s));
      }
   }

   const char *parseSectionHeader(const char *s, ElfSectionHeader *sh);
   int parse(const char *m_s, ulong fdlen, const QString &library, QLibraryPrivate *lib, long *pos, ulong *sectionlen);
};

QT_END_NAMESPACE

#endif // defined(Q_OF_ELF) && defined(Q_CC_GNU)

#endif // QELFPARSER_P_H
