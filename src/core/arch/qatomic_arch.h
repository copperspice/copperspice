/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QATOMIC_ARCH_H
#define QATOMIC_ARCH_H

#include <qglobal.h>

#if defined(QT_ARCH_ALPHA)
#  include <qatomic_alpha.h>

#elif defined(QT_ARCH_ARM)
#  include <qatomic_arm.h>

#elif defined(QT_ARCH_ARMV6)
#  include <qatomic_armv6.h>

#elif defined(QT_ARCH_AVR32)
#  include <qatomic_avr32.h>

#elif defined(QT_ARCH_BFIN)
#  include <qatomic_bfin.h>

#elif defined(QT_ARCH_GENERIC)
#  include <qatomic_generic.h>

#elif defined(QT_ARCH_I386)
#  include <qatomic_i386.h>

#elif defined(QT_ARCH_IA64)
#  include <qatomic_ia64.h>

#elif defined(QT_ARCH_MACOSX)
#  include <qatomic_macosx.h>

#elif defined(QT_ARCH_MIPS)
#  include <qatomic_mips.h>

#elif defined(QT_ARCH_PARISC)
#  include <qatomic_parisc.h>

#elif defined(QT_ARCH_POWERPC)
#  include <qatomic_powerpc.h>

#elif defined(QT_ARCH_S390)
#  include <qatomic_s390.h>

#elif defined(QT_ARCH_SPARC)
#  include <qatomic_sparc.h>

#elif defined(QT_ARCH_WINDOWS)
#  include <qatomic_windows.h>

#elif defined(QT_ARCH_X86_64)
#  include <qatomic_x86_64.h>

#elif defined(QT_ARCH_SH)
#  include <qatomic_sh.h>

#elif defined(QT_ARCH_SH4A)
#  include <qatomic_sh4a.h>

#elif defined(QT_ARCH_NACL)
#  include <qatomic_generic.h>

#else
#  error "CopperSpice has not been ported to this architecture"
#endif

#endif // QATOMIC_ARCH_H
