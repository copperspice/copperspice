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

#ifndef QABSTRACTVIDEOSURFACE_H
#define QABSTRACTVIDEOSURFACE_H

#include <QtCore/qobject.h>
#include <QtMultimedia/qvideoframe.h>
#include <QScopedPointer>
#include <QVideoSurfaceFormat>

QT_BEGIN_NAMESPACE

class QRectF;
class QAbstractVideoSurfacePrivate;

class Q_MULTIMEDIA_EXPORT QAbstractVideoSurface : public QObject
{
   MULTI_CS_OBJECT(QAbstractVideoSurface)

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

 protected:
   QAbstractVideoSurface(QAbstractVideoSurfacePrivate &dd, QObject *parent);
   QScopedPointer<QAbstractVideoSurfacePrivate> d_ptr;

   void setError(Error error);

 private:
   Q_DECLARE_PRIVATE(QAbstractVideoSurface)

};

QT_END_NAMESPACE


#endif
