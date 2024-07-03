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

#ifndef QCOCOAHELPERS_H
#define QCOCOAHELPERS_H

#include <qt_mac_p.h>
#include <qapplication_p.h>
#include <qscreen.h>
#include <qstring.h>

class QPixmap;

// Conversion functions
QStringList qt_mac_NSArrayToQStringList(void *nsarray);
void *qt_mac_QStringListToNSMutableArrayVoid(const QStringList &list);

inline NSMutableArray *qt_mac_QStringListToNSMutableArray(const QStringList &qstrlist)
{
   return reinterpret_cast<NSMutableArray *>(qt_mac_QStringListToNSMutableArrayVoid(qstrlist));
}

NSImage *qt_mac_cgimage_to_nsimage(CGImageRef iamge);
NSImage *qt_mac_create_nsimage(const QPixmap &pm);
NSImage *qt_mac_create_nsimage(const QIcon &icon, int defaultSize = 0);
CGImageRef qt_mac_toCGImage(const QImage &qImage);
CGImageRef qt_mac_toCGImageMask(const QImage &qImage);
QImage qt_mac_toQImage(CGImageRef image);

NSSize qt_mac_toNSSize(const QSize &qtSize);
NSRect qt_mac_toNSRect(const QRect &rect);
QRect qt_mac_toQRect(const NSRect &rect);

QColor qt_mac_toQColor(const NSColor *color);
QColor qt_mac_toQColor(CGColorRef color);


// Creates a mutable shape, it's the caller's responsibility to release.
HIMutableShapeRef qt_mac_QRegionToHIMutableShape(const QRegion &region);

void qt_mac_drawCGImage(CGContextRef inContext, const CGRect *inBounds, CGImageRef inImage);

NSDragOperation qt_mac_mapDropAction(Qt::DropAction action);
NSDragOperation qt_mac_mapDropActions(Qt::DropActions actions);
Qt::DropAction qt_mac_mapNSDragOperation(NSDragOperation nsActions);
Qt::DropActions qt_mac_mapNSDragOperations(NSDragOperation nsActions);

// Misc
void qt_mac_transformProccessToForegroundApplication();
QString qt_mac_removeMnemonics(const QString &original);
CGColorSpaceRef qt_mac_genericColorSpace();
CGColorSpaceRef qt_mac_displayColorSpace(const QWidget *widget);
QString qt_mac_applicationName();

int qt_mac_flipYCoordinate(int y);
qreal qt_mac_flipYCoordinate(qreal y);
QPointF qt_mac_flipPoint(const NSPoint &p);
NSPoint qt_mac_flipPoint(const QPoint &p);
NSPoint qt_mac_flipPoint(const QPointF &p);

NSRect qt_mac_flipRect(const QRect &rect);

Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum);

bool qt_mac_execute_apple_script(const char *script, long script_len, AEDesc *ret);
bool qt_mac_execute_apple_script(const char *script, AEDesc *ret);
bool qt_mac_execute_apple_script(const QString &script, AEDesc *ret);

// strip out '&' characters, and convert "&&" to a single '&', in menu
// text - since menu text is sometimes decorated with these for Windows accelerators.
QString qt_mac_removeAmpersandEscapes(QString s);

static constexpr const auto QtCocoaEventSubTypeWakeup      = SHRT_MAX;
static constexpr const auto QtCocoaEventSubTypePostMessage = SHRT_MAX - 1;

class QCocoaPostMessageArgs
{
 public:
   id target;
   SEL selector;
   int argCount;
   id arg1;
   id arg2;

   QCocoaPostMessageArgs(id target, SEL selector, int argCount = 0, id arg1 = nullptr, id arg2 = nullptr)
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

CGContextRef qt_mac_cg_context(QPaintDevice *pdev);

template<typename T>
T qt_mac_resolveOption(const T &fallback, const QByteArray &environment)
{
   // check for environment variable
   if (! environment.isEmpty()) {
      QByteArray env = qgetenv(environment.constData());

      if (!env.isEmpty()) {
         return T(env.toInt());   // works when T is bool, int
      }
   }

   return fallback;
}

template<typename T>
T qt_mac_resolveOption(const T &fallback, QWindow *window, const QByteArray &property, const QByteArray &environment)
{
   // check for environment variable
   if (! environment.isEmpty()) {
      QByteArray env = qgetenv(environment.constData());

      if (!env.isEmpty()) {
         return T(env.toInt());   // works when T is bool, int
      }
   }

   // check for window property
   if (window && !property.isNull()) {
      QVariant windowProperty = window->property(property);
      if (windowProperty.isValid()) {
         return windowProperty.value<T>();
      }
   }

   // return default value
   return fallback;
}

#endif

