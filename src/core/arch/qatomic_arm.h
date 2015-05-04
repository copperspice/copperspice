/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QATOMIC_ARM_H
#define QATOMIC_ARM_H

#if defined(__ARM_ARCH_7__) \
    || defined(__ARM_ARCH_7A__) \
    || defined(__ARM_ARCH_7R__) \
    || defined(__ARM_ARCH_7M__)
# define QT_ARCH_ARMV7

QT_BEGIN_INCLUDE_HEADER
# include "QtCore/qatomic_armv7.h"
QT_END_INCLUDE_HEADER

#elif defined(__ARM_ARCH_6__) \
    || defined(__ARM_ARCH_6J__) \
    || defined(__ARM_ARCH_6T2__) \
    || defined(__ARM_ARCH_6Z__) \
    || defined(__ARM_ARCH_6K__) \
    || defined(__ARM_ARCH_6ZK__) \
    || defined(__ARM_ARCH_6M__) \
    || (defined(__TARGET_ARCH_ARM) && (__TARGET_ARCH_ARM-0 >= 6))
# define QT_ARCH_ARMV6

QT_BEGIN_INCLUDE_HEADER
# include "QtCore/qatomic_armv6.h"
QT_END_INCLUDE_HEADER

#else
# define QT_ARCH_ARMV5

QT_BEGIN_INCLUDE_HEADER
# include "QtCore/qatomic_armv5.h"
QT_END_INCLUDE_HEADER

#endif


#endif // QATOMIC_ARM_H
