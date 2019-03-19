/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
** Copyright (c) 2007-2008, Apple, Inc.
***********************************************************************/

#ifndef QT_COCOA_HELPERS_MAC_P_H
#define QT_COCOA_HELPERS_MAC_P_H

#include <qt_mac_p.h>

#ifdef __OBJC__
// OS X framework
#include <AppKit/AppKit.h>
#endif

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qwidget.h>
#include <qevent.h>
#include <qhash.h>
#include <qlabel.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qeffects_p.h>
#include <qwidget_p.h>
#include <qtextdocument.h>
#include <qdebug.h>
#include <qpoint.h>

typedef struct CGPoint NSPoint;

QT_BEGIN_NAMESPACE

enum {
   QtCocoaEventSubTypeWakeup       = SHRT_MAX,
   QtCocoaEventSubTypePostMessage  = SHRT_MAX - 1
};

Qt::MouseButtons qt_mac_get_buttons(int buttons);

void macWindowFade(void * /*OSWindowRef*/ window, float durationSeconds = 0.15);
bool macWindowIsTextured(void * /*OSWindowRef*/ window);
void macWindowToolbarShow(const QWidget *widget, bool show );
void macWindowToolbarSet( void * /*OSWindowRef*/ window, void *toolbarRef );
bool macWindowToolbarIsVisible( void * /*OSWindowRef*/ window );
void macWindowSetHasShadow( void * /*OSWindowRef*/ window, bool hasShadow );
void macWindowFlush(void * /*OSWindowRef*/ window);
void macSendToolbarChangeEvent(QWidget *widget);
void qt_mac_updateContentBorderMetrics(void * /*OSWindowRef */window, const ::HIContentBorderMetrics &metrics);
void qt_mac_replaceDrawRect(void * /*OSWindowRef */window, QWidgetPrivate *widget);
void qt_mac_replaceDrawRectOriginal(void * /*OSWindowRef */window, QWidgetPrivate *widget);
void qt_mac_showBaseLineSeparator(void * /*OSWindowRef */window, bool show);
void * /*NSImage */qt_mac_create_nsimage(const QPixmap &pm);
void qt_mac_update_mouseTracking(QWidget *widget);

OSStatus qt_mac_drawCGImage(CGContextRef cg, const CGRect *inbounds, CGImageRef);
bool qt_mac_checkForNativeSizeGrip(const QWidget *widget);
void qt_dispatchTabletProximityEvent(void * /*NSEvent * */ tabletEvent);
bool qt_dispatchKeyEventWithCocoa(void * /*NSEvent * */ keyEvent, QWidget *widgetToGetEvent);

// These methods exists only for supporting unified mode
void macDrawRectOnTop(void * /*OSWindowRef */ window);
void macSyncDrawingOnFirstInvocation(void * /*OSWindowRef */window);
void qt_cocoaStackChildWindowOnTopOfOtherChildren(QWidget *widget);
void qt_mac_menu_collapseSeparators(void * /*NSMenu */ menu, bool collapse);

bool qt_dispatchKeyEvent(void * /*NSEvent * */ keyEvent, QWidget *widgetToGetEvent);
void qt_dispatchModifiersChanged(void * /*NSEvent * */flagsChangedEvent, QWidget *widgetToGetEvent);
bool qt_mac_handleTabletEvent(void * /*QCocoaView * */view, void * /*NSEvent * */event);
inline QApplication *qAppInstance()
{
   return static_cast<QApplication *>(QCoreApplication::instance());
}

void qt_dispatchTabletProximityEvent(const ::TabletProximityRec &proxRec);
Qt::KeyboardModifiers qt_cocoaModifiers2QtModifiers(ulong modifierFlags);
Qt::KeyboardModifiers qt_cocoaDragOperation2QtModifiers(uint dragOperations);
QPixmap qt_mac_convert_iconref(const IconRef icon, int width, int height);
void qt_mac_constructQIconFromIconRef(const IconRef icon, const IconRef overlayIcon, QIcon *retIcon,
                                      QStyle::StandardPixmap standardIcon = QStyle::SP_CustomBase);

#ifdef __OBJC__

struct DnDParams {
   NSView *view;
   NSEvent *theEvent;
   QPoint globalPoint;
   NSDragOperation performedAction;
};

DnDParams *macCurrentDnDParameters();
NSDragOperation qt_mac_mapDropAction(Qt::DropAction action);
NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions);
Qt::DropAction qt_mac_mapNSDragOperation(NSDragOperation nsActions);
Qt::DropActions qt_mac_mapNSDragOperations(NSDragOperation nsActions);

QWidget *qt_mac_getTargetForKeyEvent(QWidget *widgetThatReceivedEvent);
QWidget *qt_mac_getTargetForMouseEvent(NSEvent *event, QEvent::Type eventType, QPoint &returnLocalPoint, 
                                       QPoint &returnGlobalPoint, QWidget *nativeWidget, QWidget **returnWidgetUnderMouse);

bool qt_mac_handleMouseEvent(NSEvent *event, QEvent::Type eventType, Qt::MouseButton button, QWidget *nativeWidget,
                             bool fakeEvent = false);
void qt_mac_handleNonClientAreaMouseEvent(NSWindow *window, NSEvent *event);

#endif


inline int flipYCoordinate(int y)
{
   return QApplication::desktop()->screenGeometry(0).height() - y;
}

inline qreal flipYCoordinate(qreal y)
{
   return QApplication::desktop()->screenGeometry(0).height() - y;
}

QPointF flipPoint(const NSPoint &p);
NSPoint flipPoint(const QPoint &p);
NSPoint flipPoint(const QPointF &p);

QStringList qt_mac_NSArrayToQStringList(void *nsarray);
void *qt_mac_QStringListToNSMutableArrayVoid(const QStringList &list);

void qt_syncCocoaTitleBarButtons(OSWindowRef window, QWidget *widgetForWindow);

CGFloat qt_mac_get_scalefactor();
QString qt_mac_get_pasteboardString(OSPasteboardRef paste);

#ifdef __OBJC__

inline NSMutableArray *qt_mac_QStringListToNSMutableArray(const QStringList &qstrlist)
{
   return reinterpret_cast<NSMutableArray *>(qt_mac_QStringListToNSMutableArrayVoid(qstrlist));
}

inline QString qt_mac_NSStringToQString(const NSString *nsstr)
{
   return QCFString::toQString(reinterpret_cast<const CFStringRef>(nsstr));
}

inline NSString *qt_mac_QStringToNSString(const QString &qstr)
{
   return [const_cast<NSString *>(reinterpret_cast<const NSString *>(QCFString::toCFStringRef(qstr))) autorelease];
}

class QCocoaPostMessageArgs
{
 public:
   id target;
   SEL selector;
   int argCount;
   id arg1;
   id arg2;

   QCocoaPostMessageArgs(id target, SEL selector, int argCount = 0, id arg1 = 0, id arg2 = 0)
      : target(target), selector(selector), argCount(argCount), arg1(arg1), arg2(arg2) {
      [target retain];
      [arg1 retain];
      [arg2 retain];
   }

   ~QCocoaPostMessageArgs() {
      [arg2 release];
      [arg1 release];
      [target release];
   }
};
void qt_cocoaPostMessage(id target, SEL selector, int argCount = 0, id arg1 = 0, id arg2 = 0);
void qt_cocoaPostMessageAfterEventLoopExit(id target, SEL selector, int argCount = 0, id arg1 = 0, id arg2 = 0);

QThreadData *internal_get_ThreadData(QObject *object);
#endif

class QMacScrollOptimization
{
   // This class is made to optimize for the case when the user
   // scrolls both horizontally and vertically at the same
   // time. This will result in two QWheelEvents (one for each
   // direction), which will typically result in two calls to
   // QWidget::_scroll_sys. Rather than copying pixels twize on
   // screen because of this, we add this helper class to try to
   // get away with only one blit.
   static QWidgetPrivate *_target;
   static bool _inWheelEvent;
   static int _dx;
   static int _dy;
   static QRect _scrollRect;

 public:
   static void initDelayedScroll() {
      _inWheelEvent = true;
   }

   static bool delayScroll(QWidgetPrivate *target, int dx, int dy, const QRect &scrollRect) {
      if (!_inWheelEvent) {
         return false;
      }
      if (_target && _target != target) {
         return false;
      }
      if (_scrollRect.width() != -1 && _scrollRect != scrollRect) {
         return false;
      }

      _target = target;
      _dx += dx;
      _dy += dy;
      _scrollRect = scrollRect;
      return true;
   }

   static void performDelayedScroll() {
      if (!_inWheelEvent) {
         return;
      }
      _inWheelEvent = false;
      if (!_target) {
         return;
      }

      _target->scroll_sys(_dx, _dy, _scrollRect);

      _target = 0;
      _dx = 0;
      _dy = 0;
      _scrollRect = QRect(0, 0, -1, -1);
   }
};

void qt_mac_post_retranslateAppMenu();

void qt_mac_display(QWidget *widget);
void qt_mac_setNeedsDisplay(QWidget *widget);
void qt_mac_setNeedsDisplayInRect(QWidget *widget, QRegion region);


// Utility functions to ease the use of Core Graphics contexts.

inline void qt_mac_retain_graphics_context(CGContextRef context)
{
   CGContextRetain(context);
   CGContextSaveGState(context);
}

inline void qt_mac_release_graphics_context(CGContextRef context)
{
   CGContextRestoreGState(context);
   CGContextRelease(context);
}

inline void qt_mac_draw_image(CGContextRef context, CGContextRef imageContext, CGRect area, CGRect drawingArea)
{
   CGImageRef image = CGBitmapContextCreateImage(imageContext);
   CGImageRef subImage = CGImageCreateWithImageInRect(image, area);

   CGContextTranslateCTM (context, 0, drawingArea.origin.y + CGRectGetMaxY(drawingArea));
   CGContextScaleCTM(context, 1, -1);
   CGContextDrawImage(context, drawingArea, subImage);

   CGImageRelease(subImage);
   CGImageRelease(image);
}

QT_END_NAMESPACE

#endif // QT_COCOA_HELPERS_MAC_P_H
