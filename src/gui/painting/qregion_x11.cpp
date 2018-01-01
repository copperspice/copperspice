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

#include <qt_x11_p.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

QRegion::QRegionData QRegion::shared_empty = {1, 0, 0, 0};

void QRegion::updateX11Region() const
{
   d->rgn = XCreateRegion();
   if (!d->qt_rgn) {
      return;
   }

   int n = d->qt_rgn->numRects;
   const QRect *rect = (n == 1 ? &d->qt_rgn->extents : d->qt_rgn->rects.constData());
   while (n--) {
      XRectangle r;
      r.x = qMax(SHRT_MIN, rect->x());
      r.y = qMax(SHRT_MIN, rect->y());
      r.width = qMin((int)USHRT_MAX, rect->width());
      r.height = qMin((int)USHRT_MAX, rect->height());
      XUnionRectWithRegion(&r, d->rgn, d->rgn);
      ++rect;
   }
}

void *QRegion::clipRectangles(int &num) const
{
   if (!d->xrectangles && !(d == &shared_empty || d->qt_rgn->numRects == 0)) {
      XRectangle *r = static_cast<XRectangle *>(malloc(d->qt_rgn->numRects * sizeof(XRectangle)));
      d->xrectangles = r;
      int n = d->qt_rgn->numRects;
      const QRect *rect = (n == 1 ? &d->qt_rgn->extents : d->qt_rgn->rects.constData());
      while (n--) {
         r->x = qMax(SHRT_MIN, rect->x());
         r->y = qMax(SHRT_MIN, rect->y());
         r->width = qMin((int)USHRT_MAX, rect->width());
         r->height = qMin((int)USHRT_MAX, rect->height());
         ++r;
         ++rect;
      }
   }
   if (d == &shared_empty || d->qt_rgn->numRects == 0) {
      num = 0;
   } else {
      num = d->qt_rgn->numRects;
   }
   return d->xrectangles;
}

QT_END_NAMESPACE
