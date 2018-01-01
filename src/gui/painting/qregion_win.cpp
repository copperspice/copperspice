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

#include <qatomic.h>
#include <qbitmap.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qpolygon.h>
#include <qregion.h>
#include <qt_windows.h>
#include <qpainterpath.h>

QT_BEGIN_NAMESPACE

QRegion::QRegionData QRegion::shared_empty = { 1, 0, 0 };

HRGN qt_tryCreateRegion(QRegion::RegionType type, int left, int top, int right, int bottom)
{
   const int tries = 10;
   for (int i = 0; i < tries; ++i) {
      HRGN region = 0;
      switch (type) {
         case QRegion::Rectangle:
            region = CreateRectRgn(left, top, right, bottom);
            break;

         case QRegion::Ellipse:
            region = CreateEllipticRgn(left, top, right, bottom);
            break;
      }
      if (region) {
         if (GetRegionData(region, 0, 0)) {
            return region;
         } else {
            DeleteObject(region);
         }
      }
   }
   return 0;
}

QRegion qt_region_from_HRGN(HRGN rgn)
{
   int numBytes = GetRegionData(rgn, 0, 0);
   if (numBytes == 0) {
      return QRegion();
   }

   char *buf = new char[numBytes];
   if (buf == 0) {
      return QRegion();
   }

   RGNDATA *rd = reinterpret_cast<RGNDATA *>(buf);
   if (GetRegionData(rgn, numBytes, rd) == 0) {
      delete [] buf;
      return QRegion();
   }

   QRegion region;
   RECT *r = reinterpret_cast<RECT *>(rd->Buffer);
   for (uint i = 0; i < rd->rdh.nCount; ++i) {
      QRect rect;
      rect.setCoords(r->left, r->top, r->right - 1, r->bottom - 1);
      ++r;
      region |= rect;
   }

   delete [] buf;

   return region;
}

void qt_win_dispose_rgn(HRGN r)
{
   if (r) {
      DeleteObject(r);
   }
}

static void qt_add_rect(HRGN &winRegion, QRect r)
{
   HRGN rgn = CreateRectRgn(r.left(), r.top(), r.x() + r.width(), r.y() + r.height());
   if (rgn) {
      HRGN dest = CreateRectRgn(0, 0, 0, 0);
      int result = CombineRgn(dest, winRegion, rgn, RGN_OR);
      if (result) {
         DeleteObject(winRegion);
         winRegion = dest;
      }
      DeleteObject(rgn);
   }
}

void QRegion::ensureHandle() const
{
   if (d->rgn) {
      DeleteObject(d->rgn);
   }
   d->rgn = CreateRectRgn(0, 0, 0, 0);
   if (d->qt_rgn) {
      if (d->qt_rgn->numRects == 1) {
         QRect r = d->qt_rgn->extents;
         qt_add_rect(d->rgn, r);
         return;
      }
      for (int i = 0; i < d->qt_rgn->numRects; i++) {
         QRect r = d->qt_rgn->rects.at(i);
         qt_add_rect(d->rgn, r);
      }
   }
}


QT_END_NAMESPACE
