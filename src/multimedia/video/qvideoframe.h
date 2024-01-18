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

#ifndef QVIDEOFRAME_H
#define QVIDEOFRAME_H

#include <qshareddata.h>
#include <qimage.h>
#include <qabstractvideobuffer.h>
#include <qvariant.h>

class QSize;
class QVideoFramePrivate;

class Q_MULTIMEDIA_EXPORT QVideoFrame
{
 public:
   enum FieldType {
      ProgressiveFrame,
      TopField,
      BottomField,
      InterlacedFrame
   };

   enum PixelFormat {
      Format_Invalid,
      Format_ARGB32,
      Format_ARGB32_Premultiplied,
      Format_RGB32,
      Format_RGB24,
      Format_RGB565,
      Format_RGB555,
      Format_ARGB8565_Premultiplied,
      Format_BGRA32,
      Format_BGRA32_Premultiplied,
      Format_BGR32,
      Format_BGR24,
      Format_BGR565,
      Format_BGR555,
      Format_BGRA5658_Premultiplied,

      Format_AYUV444,
      Format_AYUV444_Premultiplied,
      Format_YUV444,
      Format_YUV420P,
      Format_YV12,
      Format_UYVY,
      Format_YUYV,
      Format_NV12,
      Format_NV21,
      Format_IMC1,
      Format_IMC2,
      Format_IMC3,
      Format_IMC4,
      Format_Y8,
      Format_Y16,
      Format_Jpeg,
      Format_CameraRaw,
      Format_AdobeDng,
      NPixelFormats,
      Format_User = 1000
   };

   QVideoFrame();
   QVideoFrame(QAbstractVideoBuffer *buffer, const QSize &size, PixelFormat format);
   QVideoFrame(int bytes, const QSize &size, int bytesPerLine, PixelFormat format);
   QVideoFrame(const QImage &image);
   QVideoFrame(const QVideoFrame &other);
   ~QVideoFrame();

   QVideoFrame &operator =(const QVideoFrame &other);

   bool operator==(const QVideoFrame &other) const;
   bool operator!=(const QVideoFrame &other) const;

   bool isValid() const;

   PixelFormat pixelFormat() const;

   QAbstractVideoBuffer::HandleType handleType() const;

   QSize size() const;
   int width() const;
   int height() const;

   FieldType fieldType() const;
   void setFieldType(FieldType field);

   bool isMapped() const;
   bool isReadable() const;
   bool isWritable() const;

   QAbstractVideoBuffer::MapMode mapMode() const;

   bool map(QAbstractVideoBuffer::MapMode mode);
   void unmap();

   int bytesPerLine() const;
   int bytesPerLine(int plane) const;

   uchar *bits();
   uchar *bits(int plane);
   const uchar *bits() const;
   const uchar *bits(int plane) const;
   int mappedBytes() const;
   int planeCount() const;

   QVariant handle() const;

   qint64 startTime() const;
   void setStartTime(qint64 time);

   qint64 endTime() const;
   void setEndTime(qint64 time);

   QVariantMap availableMetaData() const;
   QVariant metaData(const QString &key) const;
   void setMetaData(const QString &key, const QVariant &value);
   static PixelFormat pixelFormatFromImageFormat(QImage::Format format);
   static QImage::Format imageFormatFromPixelFormat(PixelFormat format);

 private:
   QExplicitlySharedDataPointer<QVideoFramePrivate> d;
};

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QVideoFrame &);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QVideoFrame::FieldType);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QVideoFrame::PixelFormat);

#endif

