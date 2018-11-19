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

#ifndef QMACHPARSER_P_H
#define QMACHPARSER_P_H

#include <qendian.h>
#include <qglobal.h>
#include <qstring.h>

#if defined(Q_OF_MACH_O)

class QLibraryPrivate;

class Q_AUTOTEST_EXPORT QMachOParser
{
 public:
   enum { QtMetaDataSection, NoQtSection, NotSuitable };
   static int parse(const char *m_s, ulong fdlen, const QString &library, QString *errorString, long *pos, ulong *sectionlen);
};

#endif // defined(Q_OF_ELF) && defined(Q_CC_GNU)

#endif
