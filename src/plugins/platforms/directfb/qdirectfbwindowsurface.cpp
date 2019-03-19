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

#include "qdirectfbwindowsurface.h"
#include "qdirectfbintegration.h"
#include "qdirectfbblitter.h"
#include "qdirectfbconvenience.h"
#include "qdirectfbwindow.h"
#include <private/qpixmap_blitter_p.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QDirectFbWindowSurface::QDirectFbWindowSurface(QWidget *window)
    : QWindowSurface(window), m_pixmap(0), m_pmdata(0)
{
    IDirectFBWindow *dfbWindow = static_cast<QDirectFbWindow *>(window->platformWindow())->dfbWindow();
    dfbWindow->GetSurface(dfbWindow, m_dfbSurface.outPtr());

//WRONGSIZE
    QDirectFbBlitter *blitter = new QDirectFbBlitter(window->size(), m_dfbSurface.data());
    m_pmdata = new QDirectFbBlitterPlatformPixmap;
    m_pmdata->setBlittable(blitter);
    m_pixmap.reset(new QPixmap(m_pmdata));
}

QPaintDevice *QDirectFbWindowSurface::paintDevice()
{
    return m_pixmap.data();
}

void QDirectFbWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    m_pmdata->blittable()->unlock();

    QVector<QRect> rects = region.rects();
    for (int i = 0 ; i < rects.size(); i++) {
        const QRect rect = rects.at(i);
        DFBRegion dfbReg = { rect.x() + offset.x(),rect.y() + offset.y(),rect.right() + offset.x(),rect.bottom() + offset.y()};
        m_dfbSurface->Flip(m_dfbSurface.data(), &dfbReg, DFBSurfaceFlipFlags(DSFLIP_BLIT|DSFLIP_ONSYNC));
    }
}

void QDirectFbWindowSurface::resize(const QSize &size)
{
    if (size == QWindowSurface::size())
        return;

    QWindowSurface::resize(size);
    QDirectFbBlitter *blitter = new QDirectFbBlitter(size, m_dfbSurface.data());
    m_pmdata->setBlittable(blitter);
}

static inline void scrollSurface(IDirectFBSurface *surface, const QRect &r, int dx, int dy)
{
    const DFBRectangle rect = { r.x(), r.y(), r.width(), r.height() };
    surface->Blit(surface, surface, &rect, r.x() + dx, r.y() + dy);
    const DFBRegion region = { rect.x + dx, rect.y + dy, r.right() + dx, r.bottom() + dy };
    surface->Flip(surface, &region, DFBSurfaceFlipFlags(DSFLIP_BLIT));
}

bool QDirectFbWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    m_pmdata->blittable()->unlock();

    if (!m_dfbSurface || area.isEmpty())
        return false;
    m_dfbSurface->SetBlittingFlags(m_dfbSurface.data(), DSBLIT_NOFX);
    if (area.rectCount() == 1) {
        scrollSurface(m_dfbSurface.data(), area.boundingRect(), dx, dy);
    } else {
        const QVector<QRect> rects = area.rects();
        const int n = rects.size();
        for (int i=0; i<n; ++i) {
            scrollSurface(m_dfbSurface.data(), rects.at(i), dx, dy);
        }
    }
    return true;
}

void QDirectFbWindowSurface::beginPaint(const QRegion &region)
{
    Q_UNUSED(region);
}

void QDirectFbWindowSurface::endPaint(const QRegion &region)
{
    Q_UNUSED(region);
}

QT_END_NAMESPACE
