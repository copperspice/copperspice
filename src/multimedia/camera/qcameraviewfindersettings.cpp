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

#include <qcameraviewfindersettings.h>

class QCameraViewfinderSettingsPrivate  : public QSharedData
{
 public:
   QCameraViewfinderSettingsPrivate()
      : isNull(true), minimumFrameRate(0.0), maximumFrameRate(0.0),
        pixelFormat(QVideoFrame::Format_Invalid) {
   }

   QCameraViewfinderSettingsPrivate(const QCameraViewfinderSettingsPrivate &other)
      : QSharedData(other), isNull(other.isNull), resolution(other.resolution),
        minimumFrameRate(other.minimumFrameRate), maximumFrameRate(other.maximumFrameRate),
        pixelFormat(other.pixelFormat), pixelAspectRatio(other.pixelAspectRatio) {
   }

   bool isNull;
   QSize resolution;
   double minimumFrameRate;
   double maximumFrameRate;
   QVideoFrame::PixelFormat pixelFormat;
   QSize pixelAspectRatio;

 private:
   QCameraViewfinderSettingsPrivate &operator=(const QCameraViewfinderSettingsPrivate &other);
};

QCameraViewfinderSettings::QCameraViewfinderSettings()
   : d(new QCameraViewfinderSettingsPrivate)
{
}

QCameraViewfinderSettings::QCameraViewfinderSettings(const QCameraViewfinderSettings &other)
   : d(other.d)
{
}

QCameraViewfinderSettings::~QCameraViewfinderSettings()
{
}

QCameraViewfinderSettings &QCameraViewfinderSettings::operator=(const QCameraViewfinderSettings &other)
{
   d = other.d;
   return *this;
}

bool QCameraViewfinderSettings::operator==(const QCameraViewfinderSettings &other) const
{
   return (d == other.d) ||
          (d->isNull           == other.d->isNull &&
           d->resolution       == other.d->resolution &&
           d->minimumFrameRate == other.d->minimumFrameRate &&
           d->maximumFrameRate == other.d->maximumFrameRate &&
           d->pixelFormat      == other.d->pixelFormat &&
           d->pixelAspectRatio == other.d->pixelAspectRatio);
}

bool QCameraViewfinderSettings::isNull() const
{
   return d->isNull;
}

QSize QCameraViewfinderSettings::resolution() const
{
   return d->resolution;
}

void QCameraViewfinderSettings::setResolution(const QSize &resolution)
{
   d->isNull     = false;
   d->resolution = resolution;
}

double QCameraViewfinderSettings::minimumFrameRate() const
{
   return d->minimumFrameRate;
}

void QCameraViewfinderSettings::setMinimumFrameRate(double rate)
{
   d->isNull = false;
   d->minimumFrameRate = rate;
}

double QCameraViewfinderSettings::maximumFrameRate() const
{
   return d->maximumFrameRate;
}

void QCameraViewfinderSettings::setMaximumFrameRate(double rate)
{
   d->isNull = false;
   d->maximumFrameRate = rate;
}

QVideoFrame::PixelFormat QCameraViewfinderSettings::pixelFormat() const
{
   return d->pixelFormat;
}

void QCameraViewfinderSettings::setPixelFormat(QVideoFrame::PixelFormat format)
{
   d->isNull = false;
   d->pixelFormat = format;
}

QSize QCameraViewfinderSettings::pixelAspectRatio() const
{
   return d->pixelAspectRatio;
}

void QCameraViewfinderSettings::setPixelAspectRatio(const QSize &ratio)
{
   d->isNull = false;
   d->pixelAspectRatio = ratio;
}
