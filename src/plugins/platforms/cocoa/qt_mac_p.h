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

#ifndef QT_MAC_P_H
#define QT_MAC_P_H

#include <qmacdefines_mac.h>
#include <qglobal.h>
#include <qvariant.h>
#include <qmimedata.h>
#include <qpointer.h>
#include <qpainter.h>

#include <qcore_mac_p.h>

class QWidget;
class QDragMoveEvent;

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#include <objc/runtime.h>
#endif

#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>               // consider removing

enum {
   // AE types
   typeAEClipboardChanged = 1,

   // types
   typeQWidget = 1,               /* QWidget *  */

   // params
   kEventParamQWidget = 'qwid',   /* typeQWidget */

   // events
   kEventQtRequestContext       = 13,
   kEventQtRequestMenubarUpdate = 14,
   kEventQtRequestShowSheet     = 17,
   kEventQtRequestActivate      = 18,
   kEventQtRequestWindowChange  = 20
};

// Simple class to manage short-lived regions
class QMacSmartQuickDrawRegion
{
   RgnHandle qdRgn;

 public:
   explicit QMacSmartQuickDrawRegion(RgnHandle rgn)
      : qdRgn(rgn)
   {
   }

   QMacSmartQuickDrawRegion(const QMacSmartQuickDrawRegion &) = delete;
   QMacSmartQuickDrawRegion &operator=(const QMacSmartQuickDrawRegion &) = delete;

   ~QMacSmartQuickDrawRegion() {
      extern void qt_mac_dispose_rgn(RgnHandle);           // qregion_mac.cpp
      qt_mac_dispose_rgn(qdRgn);
   }

   operator RgnHandle() {
      return qdRgn;
   }
};

QString qt_mac_removeMnemonics(const QString &original);   // implemented in qmacstyle_mac.cpp

class QMacCGContext
{
   CGContextRef context;

 public:
   QMacCGContext(QPainter *p);                             // implemented in qmacstyle_mac.cpp

   QMacCGContext() {
      context = nullptr;
   }

   QMacCGContext(const QPaintDevice *pdev) {
      extern CGContextRef qt_mac_cg_context(const QPaintDevice *);
      context = qt_mac_cg_context(pdev);
   }

   QMacCGContext(CGContextRef cg, bool takeOwnership = false) {
      context = cg;
      if (! takeOwnership) {
         CGContextRetain(context);
      }
   }

   QMacCGContext(const QMacCGContext &copy)
      : context(nullptr) {
      *this = copy;
   }

   ~QMacCGContext() {
      if (context) {
         CGContextRelease(context);
      }
   }

   bool isNull() const {
      return context;
   }

   operator CGContextRef() {
      return context;
   }

   QMacCGContext &operator=(const QMacCGContext &copy) {
      if (context) {
         CGContextRelease(context);
      }

      context = copy.context;
      CGContextRetain(context);
      return *this;
   }

   QMacCGContext &operator=(CGContextRef cg) {
      if (context) {
         CGContextRelease(context);
      }

      context = cg;
      CGContextRetain(context);    // do not take ownership
      return *this;
   }
};

class QMacInternalPasteboardMime;
class QMimeData;

extern QPaintDevice *qt_mac_safe_pdev;                          // implemented in qmacstyle_mac.cpp

extern OSWindowRef qt_mac_window_for(const QWidget *);          // implemented in qwidget_mac.mm
extern OSViewRef qt_mac_nativeview_for(const QWidget *);
extern QPoint qt_mac_nativeMapFromParent(const QWidget *child, const QPoint &pt);

#ifdef check
# undef check
#endif

QFont qfontForThemeFont(ThemeFontID themeID);
QColor qcolorForThemeTextColor(ThemeTextColor themeColor);

struct QMacDndAnswerRecord {
   QRect rect;
   Qt::KeyboardModifiers modifiers;
   Qt::MouseButtons buttons;
   Qt::DropAction lastAction;
   unsigned int lastOperation;

   void clear() {
      rect = QRect();
      modifiers = Qt::NoModifier;
      buttons = Qt::NoButton;
      lastAction = Qt::IgnoreAction;
      lastOperation = 0;
   }
};

extern QMacDndAnswerRecord qt_mac_dnd_answer_rec;
void qt_mac_copy_answer_rect(const QDragMoveEvent &event);
bool qt_mac_mouse_inside_answer_rect(QPoint mouse);

#endif
