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

/***********************************************************************
** Copyright (c) 2007-2008, Apple, Inc.
***********************************************************************/

#ifndef QMacDefines_MAC_H
#define QMacDefines_MAC_H

#include <qglobal.h>

#ifdef qDebug
#  define old_qDebug qDebug
#  undef qDebug
#endif

#ifdef __OBJC__
#    ifdef slots
#      define old_slots slots
#      undef slots
#    endif

#include <Cocoa/Cocoa.h>

#    ifdef old_slots
#      undef slots
#      define slots
#      undef old_slots
#    endif
#endif

/*
  Mac Note: if /usr/include/AssertMacros.h is included prior to QItemDelegate, and the application
  is building in debug mode, the  check(assertion) will conflict with QItemDelegate::check

  To avoid this problem, add the folling #undef check

  after including AssertMacros.h ( mabye Cocoa.h also )
*/

#ifdef check
#undef check
#endif

#ifdef old_qDebug
#  undef qDebug
#  define qDebug QT_NO_QDEBUG_MACRO
#  undef old_qDebug
#endif

#ifdef __OBJC__
typedef NSWindow *OSWindowRef;
typedef NSView   *OSViewRef;
typedef NSMenu   *OSMenuRef;
typedef NSEvent  *OSEventRef;
#else
typedef void *OSWindowRef;
typedef void *OSViewRef;
typedef void *OSMenuRef;
typedef void *OSEventRef;
#endif

#endif