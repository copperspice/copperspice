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

#define CS_CARBON_MM
#include <cs_carbon_wrapper_p.h>
#undef  CS_CARBON_MM

#include <Carbon/Carbon.h>

#ifdef __OBJC__
#include <AppKit/AppKit.h>

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

bool documentMode;

CS_FontUIFontType CS_HIThemeGetUIFontType(ThemeFontID themeID)
{
   return CS_FontUIFontType(HIThemeGetUIFontType(themeID));
}

CTFontRef CS_CTFontCreateUIFontForLanguage(CS_FontUIFontType uiType, CGFloat size, CFStringRef language )
{
  return CTFontCreateUIFontForLanguage(CTFontUIFontType(uiType), size, language);
}

void CS_HIThemeBrushCreateCGColor(ThemeBrush brush, CGColorRef *cgClr)
{
   HIThemeBrushCreateCGColor(brush, cgClr);
}

void CS_EnableSecureEventInput()
{
   EnableSecureEventInput();
}

void CS_DisableSecureEventInput()
{
   DisableSecureEventInput();
}

OSStatus CS_GetEventParameter(EventRef event, EventParamName eventName, EventParamType eventType,
                              EventParamType *eventTypePtr,
                              ByteCount bufferSize, ByteCount *actualSize, void *data)
{
   return GetEventParameter(event, eventName, eventType, eventTypePtr, bufferSize, actualSize, data);
}

OSType CS_GetEventKind(EventRef event)
{
   return GetEventKind(event);
}

OSType CS_GetEventClass(EventRef event)
{
   return GetEventClass(event);
}

EventTime CS_GetCurrentEventTime()
{
   return GetCurrentEventTime();
}

EventTime CS_GetEventTime(EventRef event)
{
   return GetEventTime(event);
}

UInt32 CS_GetCurrentKeyModifiers()
{
   return GetCurrentKeyModifiers();
}

UInt32 CS_GetCurrentEventKeyModifiers()
{
   return GetCurrentEventKeyModifiers();
}

EventQueueRef CS_GetMainEventQueue()
{
   return GetMainEventQueue();
}

EventLoopRef CS_GetMainEventLoop()
{
   return GetMainEventLoop();
}

EventTargetRef CS_GetEventMonitorTarget()
{
   return GetEventMonitorTarget();
}

UInt32 CS_GetCurrentEventButtonState()
{
   return GetCurrentEventButtonState();
}

ItemCount CS_GetNumEventsInQueue(EventQueueRef eventQueue)
{
   return GetNumEventsInQueue(eventQueue);
}

OSStatus CS_GetThemeMetric(ThemeMetric metric, SInt32 *callBack)
{
   return GetThemeMetric(metric, callBack);
}

OSStatus CS_RemoveEventHandler(EventHandlerRef event)
{
   return RemoveEventHandler(event);
}

OSStatus CS_PostEventToQueue(EventQueueRef eventQueue, EventRef event, EventPriority priority)
{
   return PostEventToQueue(eventQueue, event, priority);
}

OSStatus CS_RemoveEventFromQueue(EventQueueRef eventQueue, EventRef event)
{
   return RemoveEventFromQueue(eventQueue, event);
}

EventRef CS_RetainEvent(EventRef keydownEvent)
{
   return RetainEvent(keydownEvent);
}

void CS_ReleaseEvent(EventRef keydownEvent)
{
   ReleaseEvent(keydownEvent);
}

SInt16 CS_LMGetKbdType()
{
   return LMGetKbdType();
}

TISInputSourceRef CS_TISCopyCurrentKeyboardInputSource()
{
   return TISCopyCurrentKeyboardInputSource();
}

void *CS_TISGetInputSourceProperty(TISInputSourceRef source, CFStringRef string)
{
   return TISGetInputSourceProperty(source, string);
}

EventRef CS_FindSpecificEventInQueue(EventQueueRef eventQ, EventComparatorUPP callBack, void *data)
{
   return FindSpecificEventInQueue(eventQ, callBack, data);
}

OSAError CS_OSACompile(ComponentInstance component, const AEDesc *sourceData, SInt32 key, OSAID *id)
{
   return OSACompile(component, sourceData, key, id);
}

OSAError CS_OSAExecute(ComponentInstance component, OSAID id1, OSAID id2, SInt32 key, OSAID *id3)
{
   return OSAExecute(component, id1, id2, key, id3);
}

OSAError CS_OSAScriptError(ComponentInstance component, OSType type, DescType descType, AEDesc *sourceData)
{
   return OSAScriptError(component, type, descType, sourceData);
}

OSAError CS_OSADisplay(ComponentInstance component, OSAID id, DescType descType, SInt32 key, AEDesc *sourceData)
{
   return OSADisplay(component, id, descType, key, sourceData);
}

OSAError CS_OSADispose(ComponentInstance component, OSAID id)
{
   return OSADispose(component, id);
}

OSStatus CS_RemoveEventLoopTimer(EventLoopTimerRef timer)
{
   return RemoveEventLoopTimer(timer);
}

EventLoopTimerUPP CS_NewEventLoopTimerUPP(EventLoopTimerProcPtr callBack)
{
   return NewEventLoopTimerUPP(callBack);
}

EventHandlerUPP CS_NewEventHandlerUPP(EventHandlerProcPtr callBack)
{
   return NewEventHandlerUPP(callBack);
}

OSStatus CS_CreateEvent(CFAllocatorRef allocator, UInt32 id, UInt32 kind, EventTime time, EventAttributes flags,
                        EventRef *event)
{
   return CreateEvent(allocator, id, kind, time, flags, event);
}

OSStatus CS_InstallEventLoopTimer(EventLoopRef eventLopp, EventTimerInterval delay, EventTimerInterval interval,
                                  EventLoopTimerUPP callBack, void *data, EventLoopTimerRef *timer)
{
   return InstallEventLoopTimer(eventLopp, delay, interval, callBack, data, timer);
}

OSStatus CS_InstallEventHandler(EventTargetRef target, EventHandlerUPP handler, ItemCount numTypes,
                                const EventTypeSpec *list,
                                void *data, EventHandlerRef *ref)
{
   return InstallEventHandler(target, handler, numTypes, list, data, ref);
}

void CS_DisposeEventHandlerUPP(EventHandlerUPP userUPP)
{
   DisposeEventHandlerUPP(userUPP);
}

OSStatus CS_SetSystemUIMode(SystemUIMode mode, SystemUIOptions options)
{
   return SetSystemUIMode(mode, options);
}

OSStatus CS_HIThemeGetGrowBoxBounds(const HIPoint *origin, const HIThemeGrowBoxDrawInfo *drawInfo, HIRect *outBounds)
{
   return HIThemeGetGrowBoxBounds(origin, drawInfo, outBounds);
}

OSStatus CS_HIThemeGetButtonContentBounds(const HIRect *inBounds, const HIThemeButtonDrawInfo *drawInfo,
      HIRect *outBounds)
{
   return HIThemeGetButtonContentBounds(inBounds, drawInfo, outBounds);
}

OSStatus CS_HIThemeDrawButton(const HIRect *inBounds, const HIThemeButtonDrawInfo *inDrawInfo, CGContextRef inContext,
                              HIThemeOrientation inOrientation, HIRect *outLabelRect)
{
   return HIThemeDrawButton(inBounds, inDrawInfo, inContext, inOrientation, outLabelRect);
}

OSStatus CS_HIThemeSetFill(ThemeBrush brush, void *info, CGContextRef context, HIThemeOrientation orientation)
{
   return HIThemeSetFill(brush, info, context, orientation);
}

OSStatus CS_HIThemeDrawMenuBackground(const HIRect *rect, const HIThemeMenuDrawInfo *info, CGContextRef context,
                                      HIThemeOrientation orientation)
{
   return HIThemeDrawMenuBackground(rect, info, context, orientation);
}

OSStatus CS_HIThemeGetMenuBackgroundShape(const HIRect *menuRect, const HIThemeMenuDrawInfo *mdi, HIShapeRef *shape)
{
   return HIThemeGetMenuBackgroundShape(menuRect, mdi, shape);
}

OSStatus CS_HIThemeDrawGroupBox(const HIRect *hirect, const HIThemeGroupBoxDrawInfo *drawInfo, CGContextRef context,
                                HIThemeOrientation orientation)
{
   return HIThemeDrawGroupBox(hirect, drawInfo, context, orientation);
}

OSStatus CS_CopyThemeIdentifier(CFStringRef *theme)
{
   return CopyThemeIdentifier(theme);
}

OSStatus CS_HIThemeDrawFrame(const HIRect *hirect, const HIThemeFrameDrawInfo *drawInfo, CGContextRef context,
                  HIThemeOrientation orientation)
{
   return HIThemeDrawFrame(hirect, drawInfo, context, orientation);
}

OSStatus CS_HIThemeDrawPopupArrow(const HIRect *hirect, const HIThemePopupArrowDrawInfo *drawInfo, CGContextRef context,
                  HIThemeOrientation orientation)
{
   return HIThemeDrawPopupArrow(hirect, drawInfo, context, orientation);
}


OSStatus CS_HIThemeDrawSeparator(const HIRect *hirect, const HIThemeSeparatorDrawInfo *drawInfo, CGContextRef context,
                  HIThemeOrientation orientation)
{
   return HIThemeDrawSeparator(hirect, drawInfo, context, orientation);
}

OSStatus CS_HIThemeDrawTabPane(const HIRect *hirect, const HIThemeTabPaneDrawInfo *info, CGContextRef context,
                  HIThemeOrientation orientation)
{
   return HIThemeDrawTabPane(hirect, info, context, orientation);
}

OSStatus CS_HIThemeDrawTextBox(CFTypeRef text, const HIRect *hirect, HIThemeTextInfo *info, CGContextRef context,
                  HIThemeOrientation orientation)
{
   return HIThemeDrawTextBox(text, hirect, info, context, orientation);
}

OSStatus CS_HIThemeDrawTab(const HIRect *hirect, const HIThemeTabDrawInfo *info, CGContextRef context,
                           HIThemeOrientation orientation, HIRect *outRect)
{
   return HIThemeDrawTab(hirect, info, context, orientation, outRect);
}

OSStatus CS_HIThemeDrawWindowFrame(const HIRect *hirect, const HIThemeWindowDrawInfo *info, CGContextRef context,
                                   HIThemeOrientation orientation, HIRect *outRect)
{
   return HIThemeDrawWindowFrame(hirect, info, context, orientation, outRect);
}

OSStatus CS_HIThemeDrawFocusRect(const HIRect *hirect, unsigned char data, CGContextRef context,
                                 HIThemeOrientation orientation)
{
   return HIThemeDrawFocusRect(hirect, data, context, orientation);
}

OSStatus CS_HIThemeGetWindowShape(const HIRect *hirect, const HIThemeWindowDrawInfo *info, WindowRegionCode code,
                                  HIShapeRef *outShape)
{
   return HIThemeGetWindowShape(hirect, info, code, outShape);
}

OSStatus CS_HIThemeDrawMenuSeparator(const HIRect *hirect1, const HIRect *hirect2, const HIThemeMenuItemDrawInfo *info,
                                     CGContextRef context, HIThemeOrientation orientation)
{
   return HIThemeDrawMenuSeparator(hirect1, hirect2, info, context, orientation);
}

OSStatus CS_HIThemeDrawMenuItem(const HIRect *hirect1, const HIRect *hirect2, const HIThemeMenuItemDrawInfo *info,
                                CGContextRef context, HIThemeOrientation orientation, HIRect *outRect)
{
   return HIThemeDrawMenuItem(hirect1, hirect2, info, context, orientation, outRect);
}

OSStatus CS_HIThemeGetTextDimensions(CFTypeRef string, CGFloat width, HIThemeTextInfo *tti, CGFloat *outWidth,
                                     CGFloat *outHeight, CGFloat *outBaseline)
{
   return HIThemeGetTextDimensions(string, width, tti, outWidth, outHeight, outBaseline);
}

OSStatus CS_HIThemeDrawMenuBarBackground(const HIRect *hirect, const HIThemeMenuBarDrawInfo *bdi, CGContextRef context,
      HIThemeOrientation orientation)
{
   return HIThemeDrawMenuBarBackground(hirect, bdi, context, orientation);
}

OSStatus CS_HIThemeDrawTrack(const HIThemeTrackDrawInfo *tdi, const HIRect *hirect, CGContextRef context,
                             HIThemeOrientation orientation)
{
   return HIThemeDrawTrack(tdi, hirect, context, orientation);
}

OSStatus CS_HIThemeDrawGrowBox(const HIPoint *pt, const HIThemeGrowBoxDrawInfo *gdi, CGContextRef context,
                               HIThemeOrientation orientation)
{
   return HIThemeDrawGrowBox(pt, gdi, context, orientation);
}

OSStatus CS_HIThemeDrawPaneSplitter(const HIRect *hirect, const HIThemeSplitterDrawInfo *info, CGContextRef context,
                                    HIThemeOrientation orientation)
{
   return HIThemeDrawPaneSplitter(hirect, info, context, orientation);
}

OSStatus CS_HIThemeGetButtonShape(const HIRect *hirect, const HIThemeButtonDrawInfo *bdi, HIShapeRef *shape)
{
   return HIThemeGetButtonShape(hirect, bdi, shape);
}

OSStatus CS_HIThemeGetTrackThumbShape(const HIThemeTrackDrawInfo *tdi, HIShapeRef *shape)
{
   return HIThemeGetTrackThumbShape(tdi, shape);
}

OSStatus CS_HIThemeDrawTrackTickMarks(const HIThemeTrackDrawInfo *tdi, ItemCount numMarks, CGContextRef context,
                                      HIThemeOrientation orientation)
{
   return HIThemeDrawTrackTickMarks(tdi, numMarks, context, orientation);
}

OSStatus CS_HIThemeGetButtonBackgroundBounds(const HIRect *newRect, const HIThemeButtonDrawInfo *info, HIRect *outRect)
{
   return HIThemeGetButtonBackgroundBounds(newRect, info, outRect);
}

OSStatus CS_HIThemeDrawTitleBarWidget(const HIRect *titleBarRect, const HIThemeWindowWidgetDrawInfo *info,
                                      CGContextRef context,
                                      HIThemeOrientation orientation)
{
   return HIThemeDrawTitleBarWidget(titleBarRect, info, context, orientation);
}

OSStatus CS_HIThemeGetTrackBounds(const HIThemeTrackDrawInfo *info, HIRect *macRect)
{
   return HIThemeGetTrackBounds(info, macRect);
}

OSStatus CS_HIThemeGetTrackDragRect(const HIThemeTrackDrawInfo *info, HIRect *macRect)
{
   return HIThemeGetTrackDragRect(info, macRect);
}

unsigned char CS_HIThemeHitTestTrack(const HIThemeTrackDrawInfo *info, const HIPoint *point,
                                     ControlPartCode *outPartHit)
{
   return HIThemeHitTestTrack(info, point, outPartHit);
}

unsigned char CS_HIThemeHitTestScrollBarArrows(const HIRect *bounds, const HIScrollBarTrackInfo *info,
      unsigned char isHoriz,
      const HIPoint *point, HIRect *outBounds,  ControlPartCode *outPartCode)
{
   return HIThemeHitTestScrollBarArrows(bounds, info, isHoriz, point, outBounds, outPartCode);
}

OSStatus CS_HIThemeGetTrackPartBounds(const HIThemeTrackDrawInfo *info, ControlPartCode inPartCode,
                                      HIRect *outPartBounds)
{
   return HIThemeGetTrackPartBounds(info, inPartCode, outPartBounds);
}

OSStatus CS_GetThemeMenuSeparatorHeight(SInt16 *ash)
{
   return GetThemeMenuSeparatorHeight(ash);
}


// wrappers
void qt_mac_updateContentBorderMetrics(void * /*OSWindowRef */window, const ::HIContentBorderMetrics &metrics)
{
   OSWindowRef theWindow = static_cast<OSWindowRef>(window);

   if ([theWindow styleMask] & NSWindowStyleMaskTexturedBackground) {
      [theWindow setContentBorderThickness: metrics.top forEdge: NSMaxYEdge];
   }

   [theWindow setContentBorderThickness: metrics.bottom forEdge: NSMinYEdge];
}
