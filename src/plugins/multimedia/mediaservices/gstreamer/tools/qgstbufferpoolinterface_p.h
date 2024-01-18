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

#ifndef QGSTBUFFERPOOLINTERFACE_P_H
#define QGSTBUFFERPOOLINTERFACE_P_H

#include <qabstractvideobuffer.h>
#include <qvideosurfaceformat.h>
#include <qobject.h>
#include <qplugin.h>

#include <gst/gst.h>

const QLatin1String QGstBufferPoolPluginKey("bufferpool");

class QGstBufferPoolInterface
{
public:
    virtual ~QGstBufferPoolInterface() {}

    virtual bool isFormatSupported(const QVideoSurfaceFormat &format) const = 0;
    virtual GstBuffer *takeBuffer(const QVideoSurfaceFormat &format, GstCaps *caps) = 0;
    virtual void clear() = 0;

    virtual QAbstractVideoBuffer::HandleType handleType() const = 0;

    /*!
      Build an QAbstractVideoBuffer instance from GstBuffer.
      Returns NULL if GstBuffer is not compatible with this buffer pool.

      This method is called from gstreamer video sink thread.
     */
    virtual QAbstractVideoBuffer *prepareVideoBuffer(GstBuffer *buffer, int bytesPerLine) = 0;
};

#define QGstBufferPoolInterface_iid "com.copperspice.CS.gstbufferpool/1.0"
CS_DECLARE_INTERFACE(QGstBufferPoolInterface, QGstBufferPoolInterface_iid)

class QGstBufferPoolPlugin : public QObject, public QGstBufferPoolInterface
{
    CS_OBJECT(QGstBufferPoolPlugin)
    CS_INTERFACES(QGstBufferPoolInterface)

public:
    explicit QGstBufferPoolPlugin(QObject *parent = nullptr);
    virtual ~QGstBufferPoolPlugin() {}

    virtual bool isFormatSupported(const QVideoSurfaceFormat &format) const = 0;
    virtual GstBuffer *takeBuffer(const QVideoSurfaceFormat &format, GstCaps *caps) = 0;
    virtual void clear() = 0;

    virtual QAbstractVideoBuffer::HandleType handleType() const = 0;

    /*!
      Build an QAbstractVideoBuffer instance from compatible GstBuffer.
      Returns NULL if GstBuffer is not compatible with this buffer pool.

      This method is called from gstreamer video sink thread.
     */
    virtual QAbstractVideoBuffer *prepareVideoBuffer(GstBuffer *buffer, int bytesPerLine) = 0;
};

#endif
