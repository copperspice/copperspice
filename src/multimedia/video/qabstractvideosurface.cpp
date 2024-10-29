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

QVideoSurfaceFormat QAbstractVideoSurface::nearestFormat(const QVideoSurfaceFormat &format) const
{
   return isFormatSupported(format)
      ? format
      : QVideoSurfaceFormat();
}

QVideoSurfaceFormat QAbstractVideoSurface::surfaceFormat() const
{
   Q_D(const QAbstractVideoSurface);
   return d->surfaceFormat;
}

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

bool QAbstractVideoSurface::isActive() const
{
   Q_D(const QAbstractVideoSurface);
   return d->active;
}


QAbstractVideoSurface::Error QAbstractVideoSurface::error() const
{
   Q_D(const QAbstractVideoSurface);
   return d->error;
}

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
