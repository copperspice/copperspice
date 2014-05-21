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

/**************************************************
** Copyright (c) 2007-2008, Apple, Inc.
**************************************************/

/*
 *  qmacdefines_mac_p.h
 *  All the defines you'll ever need for Qt/Mac :-)
 */

#ifndef QMacDefines_MAC_H
#define QMacDefines_MAC_H

#include <QtCore/qglobal.h>

#ifdef qDebug
#  define old_qDebug qDebug
#  undef qDebug
#endif

#ifdef __LP64__
typedef signed int OSStatus;
#else
typedef signed long OSStatus;
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
#ifdef QT_MAC_USE_COCOA
    typedef struct OpaqueEventHandlerCallRef * EventHandlerCallRef;
    typedef struct OpaqueEventRef * EventRef;
    typedef struct OpaqueMenuRef * MenuRef;
    typedef struct OpaquePasteboardRef* PasteboardRef;
    typedef struct OpaqueRgnHandle * RgnHandle;
    typedef const struct __HIShape *HIShapeRef;
    typedef struct __HIShape *HIMutableShapeRef;
    typedef struct CGRect CGRect;
    typedef struct CGImage *CGImageRef;
    typedef struct CGContext *CGContextRef;
    typedef struct GDevice * GDPtr;
    typedef GDPtr * GDHandle;
    typedef struct OpaqueIconRef * IconRef;
#   ifdef __OBJC__
        typedef NSWindow* OSWindowRef;
        typedef NSView *OSViewRef;
        typedef NSMenu *OSMenuRef;
        typedef NSEvent *OSEventRef;
#   else
        typedef void *OSWindowRef;
        typedef void *OSViewRef;
        typedef void *OSMenuRef;
        typedef void *OSEventRef;
#   endif
#else  // Carbon
    typedef struct OpaqueEventHandlerCallRef * EventHandlerCallRef;
    typedef struct OpaqueEventRef * EventRef;
    typedef struct OpaqueMenuRef * MenuRef;
    typedef struct OpaquePasteboardRef* PasteboardRef;
    typedef struct OpaqueRgnHandle * RgnHandle;
    typedef const struct __HIShape *HIShapeRef;
    typedef struct __HIShape *HIMutableShapeRef;
    typedef struct CGRect CGRect;
    typedef struct CGImage *CGImageRef;
    typedef struct CGContext *CGContextRef;
    typedef struct GDevice * GDPtr;
    typedef GDPtr * GDHandle;
    typedef struct OpaqueIconRef * IconRef;
    typedef struct OpaqueWindowPtr * WindowRef;
    typedef struct OpaqueControlRef * HIViewRef;
    typedef WindowRef OSWindowRef;
    typedef HIViewRef OSViewRef;
    typedef MenuRef OSMenuRef;
    typedef EventRef OSEventRef;
#endif  // QT_MAC_USE_COCOA

typedef PasteboardRef OSPasteboardRef;
typedef struct AEDesc AEDescList;
typedef AEDescList AERecord;
typedef AERecord AppleEvent;

#ifdef check
#undef check
#endif

#ifdef old_qDebug
#  undef qDebug
#  define qDebug QT_NO_QDEBUG_MACRO
#  undef old_qDebug
#endif

#endif