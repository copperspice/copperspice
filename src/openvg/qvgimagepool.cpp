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

#include "qvgimagepool_p.h"
#include "qpixmapdata_vg_p.h"

QT_BEGIN_NAMESPACE

static QVGImagePool *qt_vg_image_pool = 0;

class QVGImagePoolPrivate
{
public:
    QVGImagePoolPrivate() : lruFirst(0), lruLast(0) {}

    QVGPixmapData *lruFirst;
    QVGPixmapData *lruLast;
};

QVGImagePool::QVGImagePool()
    : d_ptr(new QVGImagePoolPrivate())
{
}

QVGImagePool::~QVGImagePool()
{
}

QVGImagePool *QVGImagePool::instance()
{
    if (!qt_vg_image_pool)
        qt_vg_image_pool = new QVGImagePool();
    return qt_vg_image_pool;
}

void QVGImagePool::setImagePool(QVGImagePool *pool)
{
    if (qt_vg_image_pool != pool)
        delete qt_vg_image_pool;
    qt_vg_image_pool = pool;
}

VGImage QVGImagePool::createTemporaryImage(VGImageFormat format,
                                           VGint width, VGint height,
                                           VGbitfield allowedQuality,
                                           QVGPixmapData *keepData)
{
    VGImage image;
    do {
        image = vgCreateImage(format, width, height, allowedQuality);
        if (image != VG_INVALID_HANDLE)
            return image;
    } while (reclaimSpace(format, width, height, keepData));
    qWarning("QVGImagePool: cannot reclaim sufficient space for a %dx%d temporary image",
             width, height);
    return VG_INVALID_HANDLE;
}

VGImage QVGImagePool::createImageForPixmap(VGImageFormat format,
                                           VGint width, VGint height,
                                           VGbitfield allowedQuality,
                                           QVGPixmapData *data)
{
    VGImage image;
    do {
        image = vgCreateImage(format, width, height, allowedQuality);
        if (image != VG_INVALID_HANDLE) {
            if (data)
                moveToHeadOfLRU(data);
            return image;
        }
    } while (reclaimSpace(format, width, height, data));
    qWarning("QVGImagePool: cannot reclaim sufficient space for a %dx%d pixmap",
             width, height);
    return VG_INVALID_HANDLE;
}

VGImage QVGImagePool::createPermanentImage(VGImageFormat format,
                                           VGint width, VGint height,
                                           VGbitfield allowedQuality)
{
    VGImage image;
    do {
        image = vgCreateImage(format, width, height, allowedQuality);
        if (image != VG_INVALID_HANDLE)
            return image;
    } while (reclaimSpace(format, width, height, 0));
    qWarning("QVGImagePool: cannot reclaim sufficient space for a %dx%d image",
             width, height);
    return VG_INVALID_HANDLE;
}

void QVGImagePool::releaseImage(QVGPixmapData *data, VGImage image)
{
    // Very simple strategy at the moment: just destroy the image.
    if (data)
        removeFromLRU(data);
    vgDestroyImage(image);
}

void QVGImagePool::useImage(QVGPixmapData *data)
{
    moveToHeadOfLRU(data);
}

void QVGImagePool::detachImage(QVGPixmapData *data)
{
    removeFromLRU(data);
}

bool QVGImagePool::reclaimSpace(VGImageFormat format,
                                VGint width, VGint height,
                                QVGPixmapData *data)
{
    Q_UNUSED(format);   // For future use in picking the best image to eject.
    Q_UNUSED(width);
    Q_UNUSED(height);

    bool succeeded = false;
    bool wasInLRU = false;
    if (data) {
        wasInLRU = data->inLRU;
        moveToHeadOfLRU(data);
    }

    QVGPixmapData *lrudata = pixmapLRU();
    if (lrudata && lrudata != data) {
        lrudata->reclaimImages();
        succeeded = true;
    }

    if (data && !wasInLRU)
        removeFromLRU(data);

    return succeeded;
}

void QVGImagePool::hibernate()
{
    Q_D(QVGImagePool);
    QVGPixmapData *pd = d->lruLast;
    while (pd) {
        QVGPixmapData *prevLRU = pd->prevLRU;
        pd->inImagePool = false;
        pd->inLRU = false;
        pd->nextLRU = 0;
        pd->prevLRU = 0;
        pd->hibernate();
        pd = prevLRU;
    }
    d->lruFirst = 0;
    d->lruLast = 0;
}

void QVGImagePool::moveToHeadOfLRU(QVGPixmapData *data)
{
    Q_D(QVGImagePool);
    if (data->inLRU) {
        if (!data->prevLRU)
            return;     // Already at the head of the list.
        removeFromLRU(data);
    }
    data->inLRU = true;
    data->nextLRU = d->lruFirst;
    data->prevLRU = 0;
    if (d->lruFirst)
        d->lruFirst->prevLRU = data;
    else
        d->lruLast = data;
    d->lruFirst = data;
}

void QVGImagePool::removeFromLRU(QVGPixmapData *data)
{
    Q_D(QVGImagePool);
    if (!data->inLRU)
        return;
    if (data->nextLRU)
        data->nextLRU->prevLRU = data->prevLRU;
    else
        d->lruLast = data->prevLRU;
    if (data->prevLRU)
        data->prevLRU->nextLRU = data->nextLRU;
    else
        d->lruFirst = data->nextLRU;
    data->inLRU = false;
}

QVGPixmapData *QVGImagePool::pixmapLRU()
{
    Q_D(QVGImagePool);
    return d->lruLast;
}

QT_END_NAMESPACE
