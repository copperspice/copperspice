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

#ifndef QVIDEOSURFACEFORMAT_H
#define QVIDEOSURFACEFORMAT_H

#include <qlist.h>
#include <qpair.h>
#include <qshareddata.h>
#include <qsize.h>
#include <qimage.h>
#include <qvideoframe.h>

class QDebug;
class QVideoSurfaceFormatPrivate;

class Q_MULTIMEDIA_EXPORT QVideoSurfaceFormat
{
 public:
   enum Direction {
      TopToBottom,
      BottomToTop
   };

   enum YCbCrColorSpace {
      YCbCr_Undefined,
      YCbCr_BT601,
      YCbCr_BT709,
      YCbCr_xvYCC601,
      YCbCr_xvYCC709,
      YCbCr_JPEG,
      YCbCr_CustomMatrix
   };

   QVideoSurfaceFormat();
   QVideoSurfaceFormat(const QSize &size, QVideoFrame::PixelFormat pixelFormat,
      QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle);

   QVideoSurfaceFormat(const QVideoSurfaceFormat &other);
   ~QVideoSurfaceFormat();

   QVideoSurfaceFormat &operator =(const QVideoSurfaceFormat &other);

   bool operator ==(const QVideoSurfaceFormat &other) const;
   bool operator !=(const QVideoSurfaceFormat &other) const;

   bool isValid() const;

   QVideoFrame::PixelFormat pixelFormat() const;
   QAbstractVideoBuffer::HandleType handleType() const;

   QSize frameSize() const;
   void setFrameSize(const QSize &size);
   void setFrameSize(int width, int height);

   int frameWidth() const;
   int frameHeight() const;

   QRect viewport() const;
   void setViewport(const QRect &viewport);

   Direction scanLineDirection() const;
   void setScanLineDirection(Direction direction);

   qreal frameRate() const;
   void setFrameRate(qreal rate);

   QSize pixelAspectRatio() const;
   void setPixelAspectRatio(const QSize &ratio);
   void setPixelAspectRatio(int horizontal, int vertical);

   YCbCrColorSpace yCbCrColorSpace() const;
   void setYCbCrColorSpace(YCbCrColorSpace colorSpace);

   QSize sizeHint() const;

   QList<QString > propertyNames() const;
   QVariant property(const QString &name) const;
   void setProperty(QStringView name, const QVariant &value);

 private:
   QSharedDataPointer<QVideoSurfaceFormatPrivate> d;
};

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QVideoSurfaceFormat &);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QVideoSurfaceFormat::Direction);
Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, QVideoSurfaceFormat::YCbCrColorSpace);

#endif

