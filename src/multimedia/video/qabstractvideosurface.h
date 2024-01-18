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

#ifndef QABSTRACTVIDEOSURFACE_H
#define QABSTRACTVIDEOSURFACE_H

#include <qobject.h>
#include <qscopedpointer.h>
#include <qvideoframe.h>
#include <qvideosurfaceformat.h>

class QRectF;
class QAbstractVideoSurfacePrivate;

class Q_MULTIMEDIA_EXPORT QAbstractVideoSurface : public QObject
{
   MULTI_CS_OBJECT(QAbstractVideoSurface)

   MULTI_CS_PROPERTY_READ(nativeResolution,   nativeResolution)
   MULTI_CS_PROPERTY_NOTIFY(nativeResolution, nativeResolutionChanged)

 public:
   enum Error {
      NoError,
      UnsupportedFormatError,
      IncorrectFormatError,
      StoppedError,
      ResourceError
   };

   explicit QAbstractVideoSurface(QObject *parent = nullptr);
   ~QAbstractVideoSurface();

   virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const = 0;

   virtual bool isFormatSupported(const QVideoSurfaceFormat &format) const;
   virtual QVideoSurfaceFormat nearestFormat(const QVideoSurfaceFormat &format) const;

   QVideoSurfaceFormat surfaceFormat() const;

   QSize nativeResolution() const;
   virtual bool start(const QVideoSurfaceFormat &format);
   virtual void stop();

   bool isActive() const;

   virtual bool present(const QVideoFrame &frame) = 0;

   Error error() const;

   MULTI_CS_SIGNAL_1(Public, void activeChanged(bool active))
   MULTI_CS_SIGNAL_2(activeChanged, active)

   MULTI_CS_SIGNAL_1(Public, void surfaceFormatChanged(const QVideoSurfaceFormat &format))
   MULTI_CS_SIGNAL_2(surfaceFormatChanged, format)

   MULTI_CS_SIGNAL_1(Public, void supportedFormatsChanged())
   MULTI_CS_SIGNAL_2(supportedFormatsChanged)

   MULTI_CS_SIGNAL_1(Public, void nativeResolutionChanged(const QSize &resolution))
   MULTI_CS_SIGNAL_2(nativeResolutionChanged, resolution)

 protected:
   QScopedPointer<QAbstractVideoSurfacePrivate> d_ptr;

   void setError(Error error);
   void setNativeResolution(const QSize &resolution);

 private:
   Q_DECLARE_PRIVATE(QAbstractVideoSurface)

};

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QAbstractVideoSurface::Error &);

#endif
