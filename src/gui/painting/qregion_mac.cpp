/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <private/qt_mac_p.h>
#include "qcoreapplication.h"
#include <qlibrary.h>

QT_BEGIN_NAMESPACE

QRegion::QRegionData QRegion::shared_empty = { Q_BASIC_ATOMIC_INITIALIZER(1), 0 };

Q_GUI_EXPORT RgnHandle qt_mac_get_rgn()
{
#ifdef RGN_CACHE_SIZE
    if(!rgncache_init) {
        rgncache_used = 0;
        rgncache_init = true;
        for(int i = 0; i < RGN_CACHE_SIZE; ++i)
            rgncache[i] = 0;
        qAddPostRoutine(qt_mac_cleanup_rgncache);
    } else if(rgncache_used) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(rgncache[i]) {
                RgnHandle ret = rgncache[i];
                SetEmptyRgn(ret);
                rgncache[i] = 0;
                --rgncache_used;
                return ret;
            }
        }
    }
#endif
    return NewRgn();
}

Q_GUI_EXPORT void qt_mac_dispose_rgn(RgnHandle r)
{
#ifdef RGN_CACHE_SIZE
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(!rgncache[i]) {
                ++rgncache_used;
                rgncache[i] = r;
                return;
            }
        }
    }
#endif
    DisposeRgn(r);
}

static OSStatus qt_mac_get_rgn_rect(UInt16 msg, RgnHandle, const Rect *rect, void *reg)
{
    if(msg == kQDRegionToRectsMsgParse) {
        QRect rct(rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top));
        if(!rct.isEmpty())
            *((QRegion *)reg) += rct;
    }
    return noErr;
}

Q_GUI_EXPORT QRegion qt_mac_convert_mac_region(RgnHandle rgn)
{
    return QRegion::fromQDRgn(rgn);
}

QRegion QRegion::fromQDRgn(RgnHandle rgn)
{
    QRegion ret;
    ret.detach();
    OSStatus oss = QDRegionToRects(rgn, kQDParseRegionFromTopLeft, qt_mac_get_rgn_rect, (void *)&ret);
    if(oss != noErr)
        return QRegion();
    return ret;
}

/*!
    \internal
     Create's a RegionHandle, it's the caller's responsibility to release.
*/
RgnHandle QRegion::toQDRgn() const
{
    RgnHandle rgnHandle = qt_mac_get_rgn();
    if(d->qt_rgn && d->qt_rgn->numRects) {
        RgnHandle tmp_rgn = qt_mac_get_rgn();
        int n = d->qt_rgn->numRects;
        const QRect *qt_r = (n == 1) ? &d->qt_rgn->extents : d->qt_rgn->rects.constData();
        while (n--) {
            SetRectRgn(tmp_rgn,
                       qMax(SHRT_MIN, qt_r->x()),
                       qMax(SHRT_MIN, qt_r->y()),
                       qMin(SHRT_MAX, qt_r->right() + 1),
                       qMin(SHRT_MAX, qt_r->bottom() + 1));
            UnionRgn(rgnHandle, tmp_rgn, rgnHandle);
            ++qt_r;
        }
        qt_mac_dispose_rgn(tmp_rgn);
    }
    return rgnHandle;
}

/*!
    \internal
     Create's a RegionHandle, it's the caller's responsibility to release.
     Returns 0 if the QRegion overflows.
*/
RgnHandle QRegion::toQDRgnForUpdate_sys() const
{
    RgnHandle rgnHandle = qt_mac_get_rgn();
    if(d->qt_rgn && d->qt_rgn->numRects) {
        RgnHandle tmp_rgn = qt_mac_get_rgn();
        int n = d->qt_rgn->numRects;
        const QRect *qt_r = (n == 1) ? &d->qt_rgn->extents : d->qt_rgn->rects.constData();
        while (n--) {

            // detect overflow. Tested for use with HIViewSetNeedsDisplayInRegion
            // in QWidgetPrivate::update_sys().
            enum { HIViewSetNeedsDisplayInRegionOverflow = 10000 }; // empirically determined conservative value
            if (qt_r->right() > HIViewSetNeedsDisplayInRegionOverflow || qt_r->bottom() > HIViewSetNeedsDisplayInRegionOverflow) {
                qt_mac_dispose_rgn(tmp_rgn);
                qt_mac_dispose_rgn(rgnHandle);
                return 0;
            }

            SetRectRgn(tmp_rgn,
                       qMax(SHRT_MIN, qt_r->x()),
                       qMax(SHRT_MIN, qt_r->y()),
                       qMin(SHRT_MAX, qt_r->right() + 1),
                       qMin(SHRT_MAX, qt_r->bottom() + 1));
            UnionRgn(rgnHandle, tmp_rgn, rgnHandle);
            ++qt_r;
        }
        qt_mac_dispose_rgn(tmp_rgn);
    }
    return rgnHandle;
}

#endif

OSStatus QRegion::shape2QRegionHelper(int inMessage, HIShapeRef, const CGRect *inRect, void *inRefcon)
{
    QRegion *region = static_cast<QRegion *>(inRefcon);
    if (!region)
        return paramErr;

    switch (inMessage) {
    case kHIShapeEnumerateRect:
        *region += QRect(inRect->origin.x, inRect->origin.y,
                         inRect->size.width, inRect->size.height);
        break;
    case kHIShapeEnumerateInit:
        // Assume the region is already setup correctly
    case kHIShapeEnumerateTerminate:
    default:
        break;
    }
    return noErr;
}


/*!
    \internal
     Create's a mutable shape, it's the caller's responsibility to release.
     WARNING: this function clamps the coordinates to SHRT_MIN/MAX on 10.4 and below.
*/
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
