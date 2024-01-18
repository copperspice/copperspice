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

/***********************************************************************
* Copyright (c) 2007-2008, Apple, Inc.
* All rights reserved.
*
* Refer to APPLE_LICENSE.TXT (in this directory) for license terms
***********************************************************************/

#include <qglobal.h>

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
typedef struct OpaqueEventHandlerCallRef *EventHandlerCallRef;
typedef struct OpaqueEventRef *EventRef;
typedef struct OpaqueMenuRef *MenuRef;
typedef struct OpaquePasteboardRef *PasteboardRef;
typedef struct OpaqueRgnHandle *RgnHandle;
typedef const struct __HIShape *HIShapeRef;
typedef struct __HIShape *HIMutableShapeRef;
typedef struct CGRect CGRect;
typedef struct CGImage *CGImageRef;
typedef struct CGContext *CGContextRef;
typedef struct GDevice *GDPtr;
typedef GDPtr *GDHandle;
typedef struct OpaqueIconRef *IconRef;
#   ifdef __OBJC__
typedef NSWindow *OSWindowRef;
typedef NSView *OSViewRef;
typedef NSMenu *OSMenuRef;
typedef NSEvent *OSEventRef;
#   else
typedef void *OSWindowRef;
typedef void *OSViewRef;
typedef void *OSMenuRef;
typedef void *OSEventRef;
#   endif

typedef PasteboardRef OSPasteboardRef;
typedef struct AEDesc AEDescList;
typedef AEDescList AERecord;
typedef AERecord AppleEvent;

#ifdef check
#undef check
#endif
