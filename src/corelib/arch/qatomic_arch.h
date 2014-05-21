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

#include "QtCore/qglobal.h"

#if defined(QT_ARCH_ALPHA)
#  include "QtCore/qatomic_alpha.h"
#elif defined(QT_ARCH_ARM)
#  include "QtCore/qatomic_arm.h"
#elif defined(QT_ARCH_ARMV6)
#  include "QtCore/qatomic_armv6.h"
#elif defined(QT_ARCH_AVR32)
#  include "QtCore/qatomic_avr32.h"
#elif defined(QT_ARCH_BFIN)
#  include "QtCore/qatomic_bfin.h"
#elif defined(QT_ARCH_GENERIC)
#  include "QtCore/qatomic_generic.h"
#elif defined(QT_ARCH_I386)
#  include "QtCore/qatomic_i386.h"
#elif defined(QT_ARCH_IA64)
#  include "QtCore/qatomic_ia64.h"
#elif defined(QT_ARCH_MACOSX)
#  include "QtCore/qatomic_macosx.h"
#elif defined(QT_ARCH_MIPS)
#  include "QtCore/qatomic_mips.h"
#elif defined(QT_ARCH_PARISC)
#  include "QtCore/qatomic_parisc.h"
#elif defined(QT_ARCH_POWERPC)
#  include "QtCore/qatomic_powerpc.h"
#elif defined(QT_ARCH_S390)
#  include "QtCore/qatomic_s390.h"
#elif defined(QT_ARCH_SPARC)
#  include "QtCore/qatomic_sparc.h"
#elif defined(QT_ARCH_WINDOWS)
#  include "QtCore/qatomic_windows.h"
#elif defined(QT_ARCH_X86_64)
#  include "QtCore/qatomic_x86_64.h"
#elif defined(QT_ARCH_SH)
#  include "QtCore/qatomic_sh.h"
#elif defined(QT_ARCH_SH4A)
#  include "QtCore/qatomic_sh4a.h"
#elif defined(QT_ARCH_NACL)
#  include "QtCore/qatomic_generic.h"
#else
#  error "CopperSpice has not been ported to this architecture"
#endif

#endif // QATOMIC_ARCH_H
