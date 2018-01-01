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

#include <qt_mac_p.h>
#include <qcoreapplication.h>
#include <qlibrary.h>

QT_BEGIN_NAMESPACE

QRegion::QRegionData QRegion::shared_empty = { 1, 0 };

OSStatus QRegion::shape2QRegionHelper(int inMessage, HIShapeRef, const CGRect *inRect, void *inRefcon)
{
   QRegion *region = static_cast<QRegion *>(inRefcon);
   if (!region) {
      return paramErr;
   }

   switch (inMessage) {
      case kHIShapeEnumerateRect:
         *region += QRect(inRect->origin.x, inRect->origin.y, inRect->size.width, inRect->size.height);
         break;

      case kHIShapeEnumerateInit:
      // Assume the region is already setup correctly

      case kHIShapeEnumerateTerminate:

      default:
         break;
   }

   return noErr;
}

HIMutableShapeRef QRegion::toHIMutableShape() const
{
   HIMutableShapeRef shape = HIShapeCreateMutable();

   if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5) {

      if (d->qt_rgn && d->qt_rgn->numRects) {
         int n = d->qt_rgn->numRects;
         const QRect *qt_r = (n == 1) ? &d->qt_rgn->extents : d->qt_rgn->rects.constData();

         while (n--) {
            CGRect cgRect = CGRectMake(qt_r->x(), qt_r->y(), qt_r->width(), qt_r->height());
            HIShapeUnionWithRect(shape, &cgRect);
            ++qt_r;
         }
      }

   }

   return shape;
}

QRegion QRegion::fromHIShapeRef(HIShapeRef shape)
{
   QRegion returnRegion;
   returnRegion.detach();

   HIShapeEnumerate(shape, kHIShapeParseFromTopLeft, shape2QRegionHelper, &returnRegion);

   return returnRegion;
}

QT_END_NAMESPACE
