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

#include <qabstractvideosurface_p.h>

#include <qvideosurfaceformat.h>
#include <qvariant.h>
#include <QDebug>


QAbstractVideoSurface::QAbstractVideoSurface(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractVideoSurfacePrivate)
{
}

QAbstractVideoSurface::~QAbstractVideoSurface()
{
}


bool QAbstractVideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
   return supportedPixelFormats(format.handleType()).contains(format.pixelFormat());
}

/*!
    Returns a supported video surface format that is similar to \a format.

    A similar surface format is one that has the same \l {QVideoSurfaceFormat::pixelFormat()}{pixel
    format} and \l {QVideoSurfaceFormat::handleType()}{handle type} but differs in some of the other
    properties.  For example if there are restrictions on the \l {QVideoSurfaceFormat::frameSize()}
    {frame sizes} a video surface can accept it may suggest a format with a larger frame size and
    a \l {QVideoSurfaceFormat::viewport()}{viewport} the size of the original frame size.

    If the format is already supported it will be returned unchanged, or if there is no similar
    supported format an invalid format will be returned.
*/

QVideoSurfaceFormat QAbstractVideoSurface::nearestFormat(const QVideoSurfaceFormat &format) const
{
   return isFormatSupported(format)
      ? format
      : QVideoSurfaceFormat();
}

/*!
    \fn QAbstractVideoSurface::supportedFormatsChanged()

    Signals that the set of formats supported by a video surface has changed.

    \sa supportedPixelFormats(), isFormatSupported()
*/

/*!
    Returns the format of a video surface.
*/

QVideoSurfaceFormat QAbstractVideoSurface::surfaceFormat() const
{
   Q_D(const QAbstractVideoSurface);
   return d->surfaceFormat;
}

/*!
    \fn QAbstractVideoSurface::surfaceFormatChanged(const QVideoSurfaceFormat &format)

    Signals that the configured \a format of a video surface has changed.

    \sa surfaceFormat(), start()
*/

/*!
    Starts a video surface presenting \a format frames.

    Returns true if the surface was started, and false if an error occurred.

    \sa isActive(), stop()
*/

bool QAbstractVideoSurface::start(const QVideoSurfaceFormat &format)
{
   Q_D(QAbstractVideoSurface);

   bool wasActive  = d->active;

   d->active = true;
   d->surfaceFormat = format;
   d->error = NoError;

   emit surfaceFormatChanged(format);

   if (! wasActive) {
      emit activeChanged(true);
   }

   return true;
}

void QAbstractVideoSurface::stop()
{
   Q_D(QAbstractVideoSurface);

   if (d->active) {
      d->surfaceFormat = QVideoSurfaceFormat();
      d->active = false;

      emit activeChanged(false);
      emit surfaceFormatChanged(surfaceFormat());
   }
}

/*!
    Indicates whether a video surface has been started.

    Returns true if the surface has been started, and false otherwise.
*/

bool QAbstractVideoSurface::isActive() const
{
   Q_D(const QAbstractVideoSurface);
   return d->active;
}

/*!
    \fn QAbstractVideoSurface::activeChanged(bool active)

    Signals that the \a active state of a video surface has changed.

    \sa isActive(), start(), stop()
*/

/*!
    \fn QAbstractVideoSurface::present(const QVideoFrame &frame)

    Presents a video \a frame.

    Returns true if the frame was presented, and false if an error occurred.

    Not all surfaces will block until the presentation of a frame has completed.  Calling present()
    on a non-blocking surface may fail if called before the presentation of a previous frame has
    completed.  In such cases the surface may not return to a ready state until it's had an
    opportunity to process events.

    If present() fails for any other reason the surface will immediately enter the stopped state
    and an error() value will be set.

    A video surface must be in the started state for present() to succeed, and the format of the
    video frame must be compatible with the current video surface format.

    \sa error()
*/

/*!
    Returns the last error that occurred.

    If a surface fails to start(), or stops unexpectedly this function can be called to discover
    what error occurred.
*/

QAbstractVideoSurface::Error QAbstractVideoSurface::error() const
{
   Q_D(const QAbstractVideoSurface);
   return d->error;
}

/*!
    Sets the value of error() to \a error.
*/

void QAbstractVideoSurface::setError(Error error)
{
   Q_D(QAbstractVideoSurface);

   d->error = error;
}

QSize QAbstractVideoSurface::nativeResolution() const
{
   Q_D(const QAbstractVideoSurface);
   return d->nativeResolution;
}
void QAbstractVideoSurface::setNativeResolution(const QSize &resolution)
{
   Q_D(QAbstractVideoSurface);
   if (d->nativeResolution != resolution) {
      d->nativeResolution = resolution;
      emit nativeResolutionChanged(resolution);
   }
}
QDebug operator<<(QDebug dbg, const QAbstractVideoSurface::Error &error)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (error) {
      case QAbstractVideoSurface::UnsupportedFormatError:
         dbg << "UnsupportedFormatError";
         break;
      case QAbstractVideoSurface::IncorrectFormatError:
         dbg << "IncorrectFormatError";
         break;
      case QAbstractVideoSurface::StoppedError:
         dbg << "StoppedError";
         break;
      case QAbstractVideoSurface::ResourceError:
         dbg << "ResourceError";
         break;
      default:
         dbg << "NoError";
         break;
   }
   return dbg;
}

