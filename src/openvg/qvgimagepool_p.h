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

#ifndef QVGIMAGEPOOL_P_H
#define QVGIMAGEPOOL_P_H

#include <qvg.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QVGPixmapData;
class QVGImagePoolPrivate;

class Q_OPENVG_EXPORT QVGImagePool
{
public:
    QVGImagePool();
    virtual ~QVGImagePool();

    static QVGImagePool *instance();

    // This function can be used from system-specific graphics system
    // plugins to alter the image allocation strategy.
    static void setImagePool(QVGImagePool *pool);

    // Create a new VGImage from the pool with the specified parameters
    // that is not associated with a pixmap.  The VGImage is returned to
    // the pool when releaseImage() is called.
    //
    // This function will call reclaimSpace() when vgCreateImage() fails.
    //
    // This function is typically called when allocating temporary
    // VGImage's for pixmap filters.  The "keepData" object will not
    // be reclaimed if reclaimSpace() needs to be called.
    virtual VGImage createTemporaryImage(VGImageFormat format,
                                         VGint width, VGint height,
                                         VGbitfield allowedQuality,
                                         QVGPixmapData *keepData = 0);

    // Create a new VGImage with the specified parameters and associate
    // it with "data".  The QVGPixmapData will be notified when the
    // VGImage needs to be reclaimed by the pool.
    //
    // This function will call reclaimSpace() when vgCreateImage() fails.
    virtual VGImage createImageForPixmap(VGImageFormat format,
                                         VGint width, VGint height,
                                         VGbitfield allowedQuality,
                                         QVGPixmapData *data);

    // Create a permanent VGImage with the specified parameters.
    // If there is insufficient space for the vgCreateImage call,
    // then this function will call reclaimSpace() and try again.
    //
    // The caller is responsible for calling vgDestroyImage()
    // when it no longer needs the VGImage, as the image is not
    // recorded in the image pool.
    //
    // This function is typically used for pattern brushes where
    // the OpenVG engine is responsible for managing the lifetime
    // of the VGImage, destroying it automatically when the brush
    // is no longer in use.
    virtual VGImage createPermanentImage(VGImageFormat format,
                                         VGint width, VGint height,
                                         VGbitfield allowedQuality);

    // Release a VGImage that is no longer required.
    virtual void releaseImage(QVGPixmapData *data, VGImage image);

    // Notify the pool that a QVGPixmapData object is using
    // an image again.  This allows the pool to move the image
    // within a least-recently-used list of QVGPixmapData objects.
    virtual void useImage(QVGPixmapData *data);

    // Notify the pool that the VGImage's associated with a
    // QVGPixmapData are being detached from the pool.  The caller
    // will become responsible for calling vgDestroyImage().
    virtual void detachImage(QVGPixmapData *data);

    // Reclaim space for an image allocation with the specified parameters.
    // Returns true if space was reclaimed, or false if there is no
    // further space that can be reclaimed.  The "data" parameter
    // indicates the pixmap that is trying to obtain space which should
    // not itself be reclaimed.
    virtual bool reclaimSpace(VGImageFormat format,
                              VGint width, VGint height,
                              QVGPixmapData *data);

    // Hibernate the image pool because the context is about to be
    // destroyed.  All VGImage's left in the pool should be released.
    virtual void hibernate();

protected:
    // Helper functions for managing the LRU list of QVGPixmapData objects.
    void moveToHeadOfLRU(QVGPixmapData *data);
    void removeFromLRU(QVGPixmapData *data);
    QVGPixmapData *pixmapLRU();

private:
    QScopedPointer<QVGImagePoolPrivate> d_ptr;

    Q_DECLARE_PRIVATE(QVGImagePool)
    Q_DISABLE_COPY(QVGImagePool)
};

QT_END_NAMESPACE

#endif
