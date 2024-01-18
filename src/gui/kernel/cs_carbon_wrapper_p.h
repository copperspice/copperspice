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

#ifndef CS_CARBON_WRAPPER_P_H
#define CS_CARBON_WRAPPER_P_H

#include <qglobal.h>
#include <qtabbar.h>

// OS X framework
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>

using ByteCount    = unsigned long;
using UInt8        = uint8_t;
using SInt16       = int16_t;
using UInt16       = uint16_t;
using SInt32       = int32_t;
using UInt32       = uint32_t;
using UInt64       = uint64_t;
using OSAID        = unsigned int;
using OSAError     = int;
using OSStatus     = SInt32;
using OSType       = UInt32;

//
struct __CFAllocator;
struct __CFString;
struct __EventLoopTimer;
struct __HIShape;
struct __TISInputSource;

struct CGColor;
struct CGImageSource;
struct ComponentInstanceRecord;
struct FSSpec;
struct HIContentBorderMetrics;
struct OpaqueControlRef;
struct OpaqueDragRef;
struct OpaqueEventLoopRef;
struct OpaqueEventRef;
struct OpaqueEventHandlerRef;
struct OpaqueEventHandlerCallRef;
struct OpaqueEventTargetRef;
struct OpaqueEventQueueRef;

using CFAllocatorRef               = const __CFAllocator *;
using CFStringRef                  = const __CFString *;
using CGColorRef                   = CGColor *;
using CGImageSourceRef             = CGImageSource *;
using ComponentInstance            = ComponentInstanceRecord *;
using ControlRef                   = OpaqueControlRef *;
using DragRef                      = OpaqueDragRef *;
using EventRef                     = OpaqueEventRef *;
using EventHandlerRef              = OpaqueEventHandlerRef *;
using EventHandlerCallRef          = OpaqueEventHandlerCallRef *;
using EventLoopRef                 = OpaqueEventLoopRef *;
using EventLoopTimerRef            = __EventLoopTimer *;
using EventQueueRef                = OpaqueEventQueueRef *;
using EventTargetRef               = OpaqueEventTargetRef *;
using HIShapeRef                   = const __HIShape *;
using HIViewRef                    = ControlRef;
using OSPasteboardRef              = PasteboardRef;
using TISInputSourceRef            = __TISInputSource *;

//
using ATSFontContext               = UInt32;
using ATSFontFormat                = UInt32;
using ATSOptionFlags               = UInt32;
using ATSFontContainerRef          = UInt32;
using CFAbsoluteTime               = double;
using CS_FontUIFontType             = uint32_t;
using ControlPartCode              = SInt16;
using DescType                     = UInt32;

using EventComparatorUPP           = unsigned char (*)(EventRef, void *);
using EventHandlerProcPtr          = OSStatus (*)(EventHandlerCallRef, EventRef, void *);
using EventHandlerUPP              = EventHandlerProcPtr;
using EventLoopTimerProcPtr        = void (*)(EventLoopTimerRef, void *);
using EventLoopTimerUPP            = EventLoopTimerProcPtr;
using EventPriority                = SInt16;
using EventAttributes              = UInt32;
using EventParamName               = OSType;
using EventParamType               = OSType;
using EventTime                    = double;
using EventTimerInterval           = EventTime;

using ItemCount                    = unsigned long;
using HIPoint                      = CGPoint;
using HIRect                       = CGRect;
using HIThemeFrameKind             = UInt32;
using HIThemeGrowBoxSize           = UInt32;
using HIThemeGrowBoxKind           = UInt32;
using HIThemeGroupBoxKind          = UInt32;
using HIThemeOrientation           = UInt32;
using HIThemeSplitterAdornment     = UInt32;
using HIThemeTextHorizontalFlush   = UInt32;
using HIThemeTabAdornment          = UInt32;
using HIThemeTabKind               = UInt32;
using HIThemeTabPaneAdornment      = UInt32;
using HIThemeTabPosition           = UInt32;
using HIThemeTabSize               = UInt32;
using HIThemeTextBoxOptions        = UInt32;
using HIThemeTextTruncation        = UInt32;
using HIThemeTextVerticalFlush     = UInt32;
using KeyboardLayoutKind           = SInt32;
using MenuItemIndex                = UInt16;

using ThemeArrowOrientation        = UInt16;
using ThemeButtonAdornnent         = UInt16;
using ThemeButtonKind              = UInt16;
using ThemeButtonValue             = UInt16;
using ThemeBrush                   = SInt16;
using ThemeDrawState               = UInt32;
using ThemeFontID                  = UInt16;
using ThemeGrowDirection           = UInt16;
using ThemePopupArrowSize          = UInt16;
using ThemeTabDirection            = UInt16;
using ThemeTabStyle                = UInt16;
using ThemeTextColor               = SInt16;
using ThemeThumbDirection          = UInt8;
using ThemeTitleBarWidget          = UInt16;
using ThemeTrackAttributes         = UInt16;
using ThemeTrackEnableState        = UInt8;
using ThemeTrackKind               = UInt16;
using ThemeTrackPressState         = UInt8;

using ThemeMetric                  = UInt32;
using ThemeMenuBarState            = UInt16;
using ThemeMenuType                = UInt16;
using ThemeMenuItemType            = UInt16;
using ThemeMenuState               = UInt16;
using ThemeWindowType              = UInt16;
using ThemeWindowAttributes        = UInt32;

using SystemUIMode                 = UInt32;
using SystemUIOptions              = UInt32;
using WindowAttributes             = UInt32;
using WindowClass                  = UInt32;
using WindowRegionCode             = UInt16;

//
#ifdef CS_CARBON_MM

struct EventTypeSpec;
struct ScrollBarTrackInfo;
struct SliderTrackInfo;
struct ProgressTrackInfo;
struct TabletProximityRec;
struct HIThemeAnimationTimeInfo;
struct HIThemeAnimationFrameInfo;
struct HIThemeButtonDrawInfo;
struct HIThemeGrowBoxDrawInfo;
struct HIThemeGroupBoxDrawInfo;
struct HIThemeFrameDrawInfo;
struct HIThemeSeparatorDrawInfo;
struct HIThemeMenuBarDrawInfo;
struct HIThemeMenuDrawInfo;
struct HIThemeMenuItemDrawInfo;
struct HIThemePopupArrowDrawInfo;
struct HIScrollBarTrackInfo;
struct HIThemeSplitterDrawInfo;
struct HIThemeTrackDrawInfo;
struct HIThemeWindowWidgetDrawInfo;
struct HIThemeTabPaneDrawInfo;
struct HIThemeTabDrawInfo;
struct HIThemeTextInfo;
struct HIThemeWindowDrawInfo;

#else

#pragma pack(push, 2)

struct EventTypeSpec {
   UInt32 eventClass;
   UInt32 eventKind;
};

struct ProgressTrackInfo {
   UInt8 phase;
};

struct ScrollBarTrackInfo {
   SInt32 viewsize;
   ThemeTrackPressState pressState;
};

struct SliderTrackInfo {
   ThemeThumbDirection thumbDir;
   ThemeTrackPressState pressState;
};

struct TabletProximityRec {
   UInt16 vendorID;
   UInt16 tabletID;
   UInt16 pointerID;
   UInt16 deviceID;
   UInt16 systemTabletID;
   UInt16 vendorPointerType;
   UInt32 pointerSerialNumber;
   UInt64 uniqueID;
   UInt32 capabilityMask;
   UInt8  pointerType;
   UInt8  enterProximity;
};

struct HIThemeAnimationTimeInfo {
   CFAbsoluteTime start;
   CFAbsoluteTime current;
};

struct HIThemeAnimationFrameInfo {
   UInt32 index;
};

struct HIThemeButtonDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   ThemeButtonKind  kind;
   ThemeButtonValue value;
   ThemeButtonAdornnent adornment;

   union {
      HIThemeAnimationTimeInfo time;
      HIThemeAnimationFrameInfo frame;
   } animation;
};

struct HIThemeGrowBoxDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   HIThemeGrowBoxKind kind;
   ThemeGrowDirection direction;
   HIThemeGrowBoxSize size;
};

struct HIThemeGroupBoxDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   HIThemeGroupBoxKind kind;
};

struct HIThemeFrameDrawInfo {
   UInt32 version;
   HIThemeFrameKind kind;
   ThemeDrawState state;
   unsigned char isFocused;
};

struct HIThemeMenuBarDrawInfo {
   UInt32 version;
   ThemeMenuBarState state;
   UInt32 attributes;
};

struct HIThemeMenuDrawInfo {
   UInt32 version;
   ThemeMenuType menuType;
   unsigned long reserved1;
   CGFloat reserved2;
   UInt32 menuDirection;
   CGFloat reserved3;
   CGFloat reserved4;
};

struct HIThemeMenuItemDrawInfo {
   UInt32 version;
   ThemeMenuItemType itemType;
   ThemeMenuState state;
};

struct HIThemePopupArrowDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   ThemeArrowOrientation orientation;
   ThemePopupArrowSize size;
};

struct HIScrollBarTrackInfo {
   UInt32 version;
   ThemeTrackEnableState enableState;
   ThemeTrackPressState pressState;
   CGFloat viewsize;
};

struct HIThemeSeparatorDrawInfo {
   UInt32 version;
   ThemeDrawState state;
};

struct HIThemeSplitterDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   HIThemeSplitterAdornment adornment;
};

struct HIThemeTrackDrawInfo {
   UInt32 version;
   ThemeTrackKind kind;
   HIRect bounds;
   SInt32 min;
   SInt32 max;
   SInt32 value;
   UInt32 reserved;
   ThemeTrackAttributes attributes;
   ThemeTrackEnableState enableState;
   UInt8 filler1;

   union {
      ScrollBarTrackInfo scrollbar;
      SliderTrackInfo slider;
      ProgressTrackInfo progress;
   } trackInfo;
};

struct HIThemeWindowWidgetDrawInfo {
   UInt32 version;
   ThemeDrawState widgetState;
   ThemeTitleBarWidget widgetType;
   ThemeDrawState windowState;
   ThemeWindowType windowType;
   ThemeWindowAttributes attributes;
   CGFloat titleHeight;
   CGFloat titleWidth;
};

struct HIThemeTabPaneDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   ThemeTabDirection direction;
   HIThemeTabSize size;
   HIThemeTabKind kind;
   HIThemeTabPaneAdornment adornment;
};

struct HIThemeTabDrawInfo {
   UInt32 version;
   ThemeTabStyle style;
   ThemeTabDirection direction;
   HIThemeTabSize size;
   HIThemeTabAdornment adornment;
   HIThemeTabKind kind;
   HIThemeTabPosition position;
};

struct HIThemeTextInfo {
   UInt32 version;
   ThemeDrawState state;
   ThemeFontID fontID;
   HIThemeTextHorizontalFlush horizontalFlushness;
   HIThemeTextVerticalFlush verticalFlushness;
   HIThemeTextBoxOptions options;
   HIThemeTextTruncation truncationPosition;
   UInt32 truncationMaxLines;
   unsigned char truncationHappened;
   UInt8 filler1;
   CTFontRef font;
};

struct HIThemeWindowDrawInfo {
   UInt32 version;
   ThemeDrawState state;
   ThemeWindowType windowType;
   ThemeWindowAttributes attributes;
   CGFloat titleHeight;
   CGFloat titleWidth;
};

#pragma pack(pop)

#endif

// methods
CS_FontUIFontType CS_HIThemeGetUIFontType(ThemeFontID themeID);
CTFontRef CS_CTFontCreateUIFontForLanguage(CS_FontUIFontType uiType, CGFloat size, CFStringRef language );
void CS_HIThemeBrushCreateCGColor(ThemeBrush brush, CGColorRef *cgClr);

void CS_EnableSecureEventInput();
void CS_DisableSecureEventInput();

OSStatus CS_GetEventParameter(EventRef event, EventParamName eventName, EventParamType eventType,
   EventParamType *eventTypePtr,
   ByteCount bufferSize, ByteCount *actualSize, void *data);

UInt32 CS_GetCurrentKeyModifiers();
UInt32 CS_GetCurrentEventKeyModifiers();
EventTime CS_GetCurrentEventTime();
OSType CS_GetEventKind(EventRef event);
OSType CS_GetEventClass(EventRef event);
EventTime CS_GetEventTime(EventRef event);
EventQueueRef CS_GetMainEventQueue();
EventLoopRef CS_GetMainEventLoop();
EventTargetRef CS_GetEventMonitorTarget();
UInt32 CS_GetCurrentEventButtonState();
ItemCount CS_GetNumEventsInQueue(EventQueueRef eventQueue);

OSStatus CS_GetThemeMetric(ThemeMetric metric, SInt32 *callBack);

OSStatus CS_RemoveEventHandler(EventHandlerRef event);
OSStatus CS_PostEventToQueue(EventQueueRef eventQueue, EventRef event, EventPriority priority);
OSStatus CS_RemoveEventFromQueue(EventQueueRef eventQueue, EventRef event);

EventRef CS_RetainEvent(EventRef keydownEvent);
void CS_ReleaseEvent(EventRef keydownEvent);

SInt16 CS_LMGetKbdType();

TISInputSourceRef CS_TISCopyCurrentKeyboardInputSource();
void *CS_TISGetInputSourceProperty(TISInputSourceRef source, CFStringRef string);

EventRef CS_FindSpecificEventInQueue(EventQueueRef eventQ, EventComparatorUPP callBack, void *data);

OSAError CS_OSACompile(ComponentInstance component, const AEDesc *sourceData, SInt32 key, OSAID *id);
OSAError CS_OSAExecute(ComponentInstance component, OSAID id1, OSAID id2, SInt32 key, OSAID *id3);
OSAError CS_OSAScriptError(ComponentInstance component, OSType type, DescType descType, AEDesc *sourceData);
OSAError CS_OSADisplay(ComponentInstance component, OSAID id, DescType descType, SInt32 key, AEDesc *sourceData);
OSAError CS_OSADispose(ComponentInstance component, OSAID id);

OSStatus CS_RemoveEventLoopTimer(EventLoopTimerRef timer);
EventLoopTimerUPP CS_NewEventLoopTimerUPP(EventLoopTimerProcPtr callBack);
EventHandlerUPP CS_NewEventHandlerUPP(EventHandlerProcPtr callBack);
OSStatus CS_CreateEvent(CFAllocatorRef allocator, UInt32 id, UInt32 kind, EventTime time, EventAttributes flags,
   EventRef *event);

OSStatus CS_InstallEventLoopTimer(EventLoopRef eventLopp, EventTimerInterval delay, EventTimerInterval interval,
   EventLoopTimerUPP callBack, void *data, EventLoopTimerRef *timer);

OSStatus CS_InstallEventHandler(EventTargetRef target, EventHandlerUPP handler, ItemCount numTypes,
   const EventTypeSpec *list,
   void *data, EventHandlerRef *ref);

void CS_DisposeEventHandlerUPP(EventHandlerUPP userUPP);
OSStatus CS_SetSystemUIMode(SystemUIMode mode, SystemUIOptions options);

//OSStatus CS_ATSFontActivateFromFileSpecification(const FSSpec *spec, ATSFontContext context, ATSFontFormat format, void *reserved,
//         ATSOptionFlags flags, ATSFontContainerRef *handle);

OSStatus CS_HIThemeGetGrowBoxBounds(const HIPoint *origin, const HIThemeGrowBoxDrawInfo *drawInfo, HIRect *outBounds);
OSStatus CS_HIThemeGetButtonContentBounds(const HIRect *inBounds, const HIThemeButtonDrawInfo *drawInfo,
   HIRect *outBounds);

OSStatus CS_HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext,
   HIThemeOrientation inOrientation, HIRect *outLabelRect);

OSStatus CS_HIThemeSetFill(ThemeBrush brush, void *info, CGContextRef context, HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawMenuBackground(const HIRect *rect, const HIThemeMenuDrawInfo *info, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeGetMenuBackgroundShape(const HIRect *menuRect, const HIThemeMenuDrawInfo *mdi, HIShapeRef *shape);

OSStatus CS_HIThemeDrawGroupBox(const HIRect *hirect, const HIThemeGroupBoxDrawInfo *drawInfo, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_CopyThemeIdentifier(CFStringRef *theme);

OSStatus CS_HIThemeDrawFrame(const HIRect *hirect, const HIThemeFrameDrawInfo *drawInfo, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawPopupArrow(const HIRect *hirect, const HIThemePopupArrowDrawInfo *drawInfo, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawSeparator(const HIRect *hirect, const HIThemeSeparatorDrawInfo *drawInfo, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawTabPane(const HIRect *hirect, const HIThemeTabPaneDrawInfo *info, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawTextBox(CFTypeRef text, const HIRect *hirect, HIThemeTextInfo *info, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawTab(const HIRect *hirect, const HIThemeTabDrawInfo *info, CGContextRef context,
   HIThemeOrientation orientation, HIRect *outRect);

OSStatus CS_HIThemeDrawWindowFrame(const HIRect *hirect, const HIThemeWindowDrawInfo *info, CGContextRef context,
   HIThemeOrientation orientation, HIRect *outRect);

OSStatus CS_HIThemeDrawFocusRect(const HIRect *hirect, unsigned char, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeGetWindowShape(const HIRect *hirect, const HIThemeWindowDrawInfo *info, WindowRegionCode code,
   HIShapeRef *outShape);

OSStatus CS_HIThemeDrawMenuSeparator(const HIRect *hirect1, const HIRect *hirect2, const HIThemeMenuItemDrawInfo *info,
   CGContextRef context, HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawMenuItem(const HIRect *hirect1, const HIRect *hirect2, const HIThemeMenuItemDrawInfo *info,
   CGContextRef context, HIThemeOrientation orientation, HIRect *outRect);

OSStatus CS_HIThemeGetTextDimensions(CFTypeRef string, CGFloat width, HIThemeTextInfo *tti, CGFloat *outWidth,
   CGFloat *outHeight, CGFloat *outBaseline);

OSStatus CS_HIThemeDrawMenuBarBackground(const HIRect *hirect, const HIThemeMenuBarDrawInfo *bdi, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawTrack(const HIThemeTrackDrawInfo *tdi, const HIRect *hirect, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawGrowBox(const HIPoint *pt, const HIThemeGrowBoxDrawInfo *gdi, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeDrawPaneSplitter(const HIRect *hirect, const HIThemeSplitterDrawInfo *info, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeGetButtonShape(const HIRect *hirect, const HIThemeButtonDrawInfo *bdi, HIShapeRef *shape);

OSStatus CS_HIThemeGetTrackThumbShape(const HIThemeTrackDrawInfo *tdi, HIShapeRef *shape);

OSStatus CS_HIThemeDrawTrackTickMarks(const HIThemeTrackDrawInfo *tdi, ItemCount numMarks, CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeGetButtonBackgroundBounds(const HIRect *newRect, const HIThemeButtonDrawInfo *info, HIRect *outRect);
OSStatus CS_HIThemeDrawTitleBarWidget(const HIRect *titleBarRect, const HIThemeWindowWidgetDrawInfo *info,
   CGContextRef context,
   HIThemeOrientation orientation);

OSStatus CS_HIThemeGetTrackBounds(const HIThemeTrackDrawInfo *info, HIRect *macRect);
OSStatus CS_HIThemeGetTrackDragRect(const HIThemeTrackDrawInfo *info, HIRect *macRect);

unsigned char CS_HIThemeHitTestTrack(const HIThemeTrackDrawInfo *info, const HIPoint *point,
   ControlPartCode *outPartHit);

unsigned char CS_HIThemeHitTestScrollBarArrows(const HIRect *bounds, const HIScrollBarTrackInfo *info,
   unsigned char isHoriz,
   const HIPoint *point, HIRect *outBounds,  ControlPartCode *outPartCode);

OSStatus CS_HIThemeGetTrackPartBounds(const HIThemeTrackDrawInfo *info, ControlPartCode inPartCode,
   HIRect *outPartBounds);

OSStatus CS_GetThemeMenuSeparatorHeight(SInt16 *ash);


// wrappers
void cs_updateMacBorderMetrics(QTabBar *q);

#endif
