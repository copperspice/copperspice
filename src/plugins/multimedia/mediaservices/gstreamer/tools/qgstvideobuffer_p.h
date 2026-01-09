/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QGSTVIDEOBUFFER_P_H
#define QGSTVIDEOBUFFER_P_H

#include <qabstractvideobuffer.h>
#include <qvariant.h>

#include <gst/gst.h>
#include <gst/video/video.h>

class QGstVideoBuffer : public QAbstractPlanarVideoBuffer
{
 public:
   QGstVideoBuffer(GstBuffer *buffer, const GstVideoInfo &info);
   QGstVideoBuffer(GstBuffer *buffer, const GstVideoInfo &info, HandleType handleType, const QVariant &handle);

   ~QGstVideoBuffer();

   MapMode mapMode() const override;

   int map(MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4]) override;

   void unmap() override;

   QVariant handle() const override {
      return m_handle;
   }

 private:
   GstVideoInfo m_videoInfo;
   GstVideoFrame m_frame;

   GstBuffer *m_buffer;
   MapMode m_mode;
   QVariant m_handle;
};

#endif
