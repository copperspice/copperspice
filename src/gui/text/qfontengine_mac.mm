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

#include <qfontengine_mac_p.h>

#include <qapplication_p.h>
#include <qfontengine_p.h>
#include <qpainter_p.h>
#include <qtextengine_p.h>
#include <qbitmap.h>
#include <qpaintengine_mac_p.h>
#include <qprintengine_mac_p.h>
#include <qglobal.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qvarlengtharray.h>
#include <qdebug.h>
#include <qendian.h>
#include <qmath.h>
#include <qimage_p.h>

// OS X framework
#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/AppKit.h>

QT_BEGIN_NAMESPACE

/*****************************************************************************
  QFontEngine debug facilities
 *****************************************************************************/
//#define DEBUG_ADVANCES

extern int qt_antialiasing_threshold; // QApplication.cpp

#ifndef FixedToQFixed
#define FixedToQFixed(a) QFixed::fromFixed((a) >> 10)
#define QFixedToFixed(x) ((x).value() << 10)
#endif

class QMacFontPath
{
   float x, y;
   QPainterPath *path;
 public:
   inline QMacFontPath(float _x, float _y, QPainterPath *_path) : x(_x), y(_y), path(_path) { }
   inline void setPosition(float _x, float _y) {
      x = _x;
      y = _y;
   }
   inline void advance(float _x) {
      x += _x;
   }
   static OSStatus lineTo(const Float32Point *, void *);
   static OSStatus cubicTo(const Float32Point *, const Float32Point *,
                           const Float32Point *, void *);
   static OSStatus moveTo(const Float32Point *, void *);
   static OSStatus closePath(void *);
};

OSStatus QMacFontPath::lineTo(const Float32Point *pt, void *data)

{
   QMacFontPath *p = static_cast<QMacFontPath *>(data);
   p->path->lineTo(p->x + pt->x, p->y + pt->y);
   return noErr;
}

OSStatus QMacFontPath::cubicTo(const Float32Point *cp1, const Float32Point *cp2,
                               const Float32Point *ep, void *data)

{
   QMacFontPath *p = static_cast<QMacFontPath *>(data);
   p->path->cubicTo(p->x + cp1->x, p->y + cp1->y,
                    p->x + cp2->x, p->y + cp2->y,
                    p->x + ep->x, p->y + ep->y);
   return noErr;
}

OSStatus QMacFontPath::moveTo(const Float32Point *pt, void *data)
{
   QMacFontPath *p = static_cast<QMacFontPath *>(data);
   p->path->moveTo(p->x + pt->x, p->y + pt->y);
   return noErr;
}

OSStatus QMacFontPath::closePath(void *data)
{
   static_cast<QMacFontPath *>(data)->path->closeSubpath();
   return noErr;
}

QT_END_NAMESPACE
