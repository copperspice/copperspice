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

#ifndef QCAMERAVIEWFINDERSETTINGS_H
#define QCAMERAVIEWFINDERSETTINGS_H

#include <qglobal.h>
#include <qvideoframe.h>
#include <qshareddata.h>
#include <qsize.h>

class QCameraViewfinderSettingsPrivate;

class Q_MULTIMEDIA_EXPORT QCameraViewfinderSettings
{
 public:
   QCameraViewfinderSettings();
   QCameraViewfinderSettings(const QCameraViewfinderSettings &other);

   ~QCameraViewfinderSettings();

   QCameraViewfinderSettings &operator=(const QCameraViewfinderSettings &other);

   QCameraViewfinderSettings &operator=(QCameraViewfinderSettings &&other) {
      swap(other);
      return *this;
   }

   void swap(QCameraViewfinderSettings &other) {
      d.swap(other.d);
   }

   bool isNull() const;

   QSize resolution() const;
   void setResolution(const QSize &resolution);

   void setResolution(int width, int height) {
      setResolution(QSize(width, height));
   }

   qreal minimumFrameRate() const;
   void setMinimumFrameRate(qreal rate);

   qreal maximumFrameRate() const;
   void setMaximumFrameRate(qreal rate);

   QVideoFrame::PixelFormat pixelFormat() const;
   void setPixelFormat(QVideoFrame::PixelFormat format);

   QSize pixelAspectRatio() const;
   void setPixelAspectRatio(const QSize &ratio);

   void setPixelAspectRatio(int horizontal, int vertical) {
      setPixelAspectRatio(QSize(horizontal, vertical));
   }

   bool operator==(const QCameraViewfinderSettings &other) const;

   bool operator!=(const QCameraViewfinderSettings &other) const {
      return ! operator==(other);
   }

 private:
   QSharedDataPointer<QCameraViewfinderSettingsPrivate> d;
};

inline void swap(QCameraViewfinderSettings &a, QCameraViewfinderSettings &b)
{
   a.swap(b);
}

#endif
