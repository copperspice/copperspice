/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qvideosurfaceformat.h>

#include <qdebug.h>
#include <qmetatype.h>
#include <qpair.h>
#include <qvariant.h>
#include <qvector.h>

class QVideoSurfaceFormatPrivate : public QSharedData
{
 public:
   QVideoSurfaceFormatPrivate()
      : pixelFormat(QVideoFrame::Format_Invalid)
      , handleType(QAbstractVideoBuffer::NoHandle)
      , scanLineDirection(QVideoSurfaceFormat::TopToBottom)
      , pixelAspectRatio(1, 1)
      , ycbcrColorSpace(QVideoSurfaceFormat::YCbCr_Undefined)
      , frameRate(0.0) {
   }

   QVideoSurfaceFormatPrivate(
      const QSize &size,
      QVideoFrame::PixelFormat format,
      QAbstractVideoBuffer::HandleType type)
      : pixelFormat(format)
      , handleType(type)
      , scanLineDirection(QVideoSurfaceFormat::TopToBottom)
      , frameSize(size)
      , pixelAspectRatio(1, 1)
      , ycbcrColorSpace(QVideoSurfaceFormat::YCbCr_Undefined)
      , viewport(QPoint(0, 0), size)
      , frameRate(0.0) {
   }

   QVideoSurfaceFormatPrivate(const QVideoSurfaceFormatPrivate &other)
      : QSharedData(other)
      , pixelFormat(other.pixelFormat)
      , handleType(other.handleType)
      , scanLineDirection(other.scanLineDirection)
      , frameSize(other.frameSize)
      , pixelAspectRatio(other.pixelAspectRatio)
      , ycbcrColorSpace(other.ycbcrColorSpace)
      , viewport(other.viewport)
      , frameRate(other.frameRate)
      , propertyNames(other.propertyNames)
      , propertyValues(other.propertyValues) {
   }

   bool operator ==(const QVideoSurfaceFormatPrivate &other) const {
      if (pixelFormat == other.pixelFormat
            && handleType == other.handleType
            && scanLineDirection == other.scanLineDirection
            && frameSize == other.frameSize
            && pixelAspectRatio == other.pixelAspectRatio
            && viewport == other.viewport
            && frameRatesEqual(frameRate, other.frameRate)
            && ycbcrColorSpace == other.ycbcrColorSpace
            && propertyNames.count() == other.propertyNames.count()) {
         for (int i = 0; i < propertyNames.count(); ++i) {
            int j = other.propertyNames.indexOf(propertyNames.at(i));

            if (j == -1 || propertyValues.at(i) != other.propertyValues.at(j)) {
               return false;
            }
         }
         return true;
      } else {
         return false;
      }
   }

   inline static bool frameRatesEqual(qreal r1, qreal r2) {
      return qAbs(r1 - r2) <= 0.00001 * qMin(qAbs(r1), qAbs(r2));
   }

   QVideoFrame::PixelFormat pixelFormat;
   QAbstractVideoBuffer::HandleType handleType;
   QVideoSurfaceFormat::Direction scanLineDirection;
   QSize frameSize;
   QSize pixelAspectRatio;
   QVideoSurfaceFormat::YCbCrColorSpace ycbcrColorSpace;
   QRect viewport;
   qreal frameRate;

   QList<QString > propertyNames;
   QList<QVariant> propertyValues;
};


QVideoSurfaceFormat::QVideoSurfaceFormat()
   : d(new QVideoSurfaceFormatPrivate)
{
}

QVideoSurfaceFormat::QVideoSurfaceFormat(
   const QSize &size, QVideoFrame::PixelFormat format, QAbstractVideoBuffer::HandleType type)
   : d(new QVideoSurfaceFormatPrivate(size, format, type))
{
}

/*!
    Constructs a copy of \a other.
*/

QVideoSurfaceFormat::QVideoSurfaceFormat(const QVideoSurfaceFormat &other)
   : d(other.d)
{
}

/*!
    Assigns the values of \a other to a video stream description.
*/

QVideoSurfaceFormat &QVideoSurfaceFormat::operator =(const QVideoSurfaceFormat &other)
{
   d = other.d;

   return *this;
}

/*!
    Destroys a video stream description.
*/

QVideoSurfaceFormat::~QVideoSurfaceFormat()
{
}

/*!
    Identifies if a video surface format has a valid pixel format and frame size.

    Returns true if the format is valid, and false otherwise.
*/

bool QVideoSurfaceFormat::isValid() const
{
   return d->pixelFormat != QVideoFrame::Format_Invalid && d->frameSize.isValid();
}

/*!
    Returns true if \a other is the same as a video format, and false if they are the different.
*/

bool QVideoSurfaceFormat::operator ==(const QVideoSurfaceFormat &other) const
{
   return d == other.d || *d == *other.d;
}

/*!
    Returns true if \a other is different to a video format, and false if they are the same.
*/

bool QVideoSurfaceFormat::operator !=(const QVideoSurfaceFormat &other) const
{
   return d != other.d && !(*d == *other.d);
}

/*!
    Returns the pixel format of frames in a video stream.
*/

QVideoFrame::PixelFormat QVideoSurfaceFormat::pixelFormat() const
{
   return d->pixelFormat;
}

/*!
    Returns the type of handle the surface uses to present the frame data.

    If the handle type is QAbstractVideoBuffer::NoHandle buffers with any handle type are valid
    provided they can be \l {QAbstractVideoBuffer::map()}{mapped} with the
    QAbstractVideoBuffer::ReadOnly flag.  If the handleType() is not QAbstractVideoBuffer::NoHandle
    then the handle type of the buffer be the same as that of the surface format.
*/

QAbstractVideoBuffer::HandleType QVideoSurfaceFormat::handleType() const
{
   return d->handleType;
}

/*!
    Returns the size of frames in a video stream.

    \sa frameWidth(), frameHeight()
*/

QSize QVideoSurfaceFormat::frameSize() const
{
   return d->frameSize;
}

/*!
    Returns the width of frames in a video stream.

    \sa frameSize(), frameHeight()
*/

int QVideoSurfaceFormat::frameWidth() const
{
   return d->frameSize.width();
}

/*!
    Returns the height of frame in a video stream.
*/

int QVideoSurfaceFormat::frameHeight() const
{
   return d->frameSize.height();
}

/*!
    Sets the size of frames in a video stream to \a size.

    This will reset the viewport() to fill the entire frame.
*/

void QVideoSurfaceFormat::setFrameSize(const QSize &size)
{
   d->frameSize = size;
   d->viewport = QRect(QPoint(0, 0), size);
}

/*!
    \overload

    Sets the \a width and \a height of frames in a video stream.

    This will reset the viewport() to fill the entire frame.
*/

void QVideoSurfaceFormat::setFrameSize(int width, int height)
{
   d->frameSize = QSize(width, height);
   d->viewport = QRect(0, 0, width, height);
}

/*!
    Returns the viewport of a video stream.

    The viewport is the region of a video frame that is actually displayed.

    By default the viewport covers an entire frame.
*/

QRect QVideoSurfaceFormat::viewport() const
{
   return d->viewport;
}

/*!
    Sets the viewport of a video stream to \a viewport.
*/

void QVideoSurfaceFormat::setViewport(const QRect &viewport)
{
   d->viewport = viewport;
}

/*!
    Returns the direction of scan lines.
*/

QVideoSurfaceFormat::Direction QVideoSurfaceFormat::scanLineDirection() const
{
   return d->scanLineDirection;
}

/*!
    Sets the \a direction of scan lines.
*/

void QVideoSurfaceFormat::setScanLineDirection(Direction direction)
{
   d->scanLineDirection = direction;
}

/*!
    Returns the frame rate of a video stream in frames per second.
*/

qreal QVideoSurfaceFormat::frameRate() const
{
   return d->frameRate;
}

/*!
    Sets the frame \a rate of a video stream in frames per second.
*/

void QVideoSurfaceFormat::setFrameRate(qreal rate)
{
   d->frameRate = rate;
}

/*!
    Returns a video stream's pixel aspect ratio.
*/

QSize QVideoSurfaceFormat::pixelAspectRatio() const
{
   return d->pixelAspectRatio;
}

/*!
    Sets a video stream's pixel aspect \a ratio.
*/

void QVideoSurfaceFormat::setPixelAspectRatio(const QSize &ratio)
{
   d->pixelAspectRatio = ratio;
}

/*!
    \overload

    Sets the \a horizontal and \a vertical elements of a video stream's pixel aspect ratio.
*/

void QVideoSurfaceFormat::setPixelAspectRatio(int horizontal, int vertical)
{
   d->pixelAspectRatio = QSize(horizontal, vertical);
}

/*!
    Returns the Y'CbCr color space of a video stream.
*/

QVideoSurfaceFormat::YCbCrColorSpace QVideoSurfaceFormat::yCbCrColorSpace() const
{
   return d->ycbcrColorSpace;
}

/*!
    Sets the Y'CbCr color \a space of a video stream.
    It is only used with raw YUV frame types.
*/

void QVideoSurfaceFormat::setYCbCrColorSpace(QVideoSurfaceFormat::YCbCrColorSpace space)
{
   d->ycbcrColorSpace = space;
}

/*!
    Returns a suggested size in pixels for the video stream.

    This is the size of the viewport scaled according to the pixel aspect ratio.
*/

QSize QVideoSurfaceFormat::sizeHint() const
{
   QSize size = d->viewport.size();

   if (d->pixelAspectRatio.height() != 0) {
      size.setWidth(size.width() * d->pixelAspectRatio.width() / d->pixelAspectRatio.height());
   }

   return size;
}

/*!
    Returns a list of video format dynamic property names.
*/

QList<QString > QVideoSurfaceFormat::propertyNames() const
{
   return (QList<QString>()
           << "handleType"
           << "pixelFormat"
           << "frameSize"
           << "frameWidth"
           << "viewport"
           << "scanLineDirection"
           << "frameRate"
           << "pixelAspectRatio"
           << "sizeHint"
           << "yCbCrColorSpace")
          + d->propertyNames;
}

/*!
    Returns the value of the video format's \a name property.
*/

QVariant QVideoSurfaceFormat::property(QStringView name) const
{
   if (name == "handleType") {
      return QVariant::fromValue(d->handleType);

   } else if (name == "pixelFormat") {
      return QVariant::fromValue(d->pixelFormat);

   } else if (name == "handleType") {
      return QVariant::fromValue(d->handleType);

   } else if (name == "frameSize") {
      return d->frameSize;

   } else if (name == "frameWidth") {
      return d->frameSize.width();

   } else if (name == "frameHeight") {
      return d->frameSize.height();

   } else if (name == "viewport") {
      return d->viewport;

   } else if (name == "scanLineDirection") {
      return QVariant::fromValue(d->scanLineDirection);

   } else if (name == "frameRate") {
      return QVariant::fromValue(d->frameRate);

   } else if (name == "pixelAspectRatio") {
      return QVariant::fromValue(d->pixelAspectRatio);

   } else if (name == "sizeHint") {
      return sizeHint();

   } else if (name == "yCbCrColorSpace") {
      return QVariant::fromValue(d->ycbcrColorSpace);

   } else {
      int id = 0;

      for (; id < d->propertyNames.count() && d->propertyNames.at(id) != name; ++id) {
         // do nothing
      }

      return id < d->propertyValues.count() ? d->propertyValues.at(id) : QVariant();
   }
}

/*!
    Sets the video format's \a name property to \a value.
*/

void QVideoSurfaceFormat::setProperty(QStringView name, const QVariant &value)
{
   if (name == "handleType") {
      // read only

   } else if (name == "pixelFormat") {
      // read only

   } else if (name == "frameSize") {
      if (value.canConvert<QSize>()) {
         d->frameSize = qvariant_cast<QSize>(value);
         d->viewport = QRect(QPoint(0, 0), d->frameSize);
      }

   } else if (name == "frameWidth") {
      // read only.

   } else if (name == "frameHeight") {
      // read only.

   } else if (name == "viewport") {
      if (value.canConvert<QRect>()) {
         d->viewport = qvariant_cast<QRect>(value);
      }

   } else if (name == "scanLineDirection") {
      if (value.canConvert<Direction>()) {
         d->scanLineDirection = qvariant_cast<Direction>(value);
      }

   } else if (name == "frameRate") {
      if (value.canConvert<qreal>()) {
         d->frameRate = qvariant_cast<qreal>(value);
      }

   } else if (name == "pixelAspectRatio") {
      if (value.canConvert<QSize>()) {
         d->pixelAspectRatio = qvariant_cast<QSize>(value);
      }

   } else if (name == "sizeHint") {
      // read only.

   } else if (name == "yCbCrColorSpace") {
      if (value.canConvert<YCbCrColorSpace>()) {
         d->ycbcrColorSpace = qvariant_cast<YCbCrColorSpace>(value);
      }
   } else {
      int id = 0;

      for (; id < d->propertyNames.count() && d->propertyNames.at(id) != name; ++id) {
         //
      }

      if (id < d->propertyValues.count()) {
         if (value.isNull()) {
            d->propertyNames.removeAt(id);
            d->propertyValues.removeAt(id);

         } else {
            d->propertyValues[id] = value;
         }

      } else if (!value.isNull()) {
         d->propertyNames.append(name);
         d->propertyValues.append(value);
      }
   }
}

QDebug operator<<(QDebug dbg, const QVideoSurfaceFormat &f)
{
   QString typeName;

   switch (f.pixelFormat()) {
      case QVideoFrame::Format_Invalid:
         typeName = "Format_Invalid";
         break;

      case QVideoFrame::Format_ARGB32:
         typeName = "Format_ARGB32";
         break;

      case QVideoFrame::Format_ARGB32_Premultiplied:
         typeName = "Format_ARGB32_Premultiplied";
         break;

      case QVideoFrame::Format_RGB32:
         typeName = "Format_RGB32";
         break;

      case QVideoFrame::Format_RGB24:
         typeName = "Format_RGB24";
         break;

      case QVideoFrame::Format_RGB565:
         typeName = "Format_RGB565";
         break;

      case QVideoFrame::Format_RGB555:
         typeName = "Format_RGB555";
         break;

      case QVideoFrame::Format_ARGB8565_Premultiplied:
         typeName = "Format_ARGB8565_Premultiplied";
         break;

      case QVideoFrame::Format_BGRA32:
         typeName = "Format_BGRA32";
         break;

      case QVideoFrame::Format_BGRA32_Premultiplied:
         typeName = "Format_BGRA32_Premultiplied";
         break;

      case QVideoFrame::Format_BGR32:
         typeName = "Format_BGR32";
         break;

      case QVideoFrame::Format_BGR24:
         typeName = "Format_BGR24";
         break;

      case QVideoFrame::Format_BGR565:
         typeName = "Format_BGR565";
         break;

      case QVideoFrame::Format_BGR555:
         typeName = "Format_BGR555";
         break;

      case QVideoFrame::Format_BGRA5658_Premultiplied:
         typeName = "Format_BGRA5658_Premultiplied";
         break;

      case QVideoFrame::Format_AYUV444:
         typeName = "Format_AYUV444";
         break;

      case QVideoFrame::Format_AYUV444_Premultiplied:
         typeName = "Format_AYUV444_Premultiplied";
         break;

      case QVideoFrame::Format_YUV444:
         typeName = "Format_YUV444";
         break;

      case QVideoFrame::Format_YUV420P:
         typeName = "Format_YUV420P";
         break;

      case QVideoFrame::Format_YV12:
         typeName = "Format_YV12";
         break;

      case QVideoFrame::Format_UYVY:
         typeName = "Format_UYVY";
         break;

      case QVideoFrame::Format_YUYV:
         typeName = "Format_YUYV";
         break;
      case QVideoFrame::Format_NV12:
         typeName = "Format_NV12";
         break;

      case QVideoFrame::Format_NV21:
         typeName = "Format_NV21";
         break;

      case QVideoFrame::Format_IMC1:
         typeName = "Format_IMC1";
         break;

      case QVideoFrame::Format_IMC2:
         typeName = "Format_IMC2";
         break;

      case QVideoFrame::Format_IMC3:
         typeName = "Format_IMC3";
         break;

      case QVideoFrame::Format_IMC4:
         typeName = "Format_IMC4";
         break;

      case QVideoFrame::Format_Y8:
         typeName = "Format_Y8";
         break;

      case QVideoFrame::Format_Y16:
         typeName = "Format_Y16";
      default:

         typeName = QString("UserType(%1)").formatArg(int(f.pixelFormat()));
   }

   dbg.nospace() << "QVideoSurfaceFormat(" << typeName;
   dbg.nospace() << ", " << f.frameSize();
   dbg.nospace() << ", viewport=" << f.viewport();
   dbg.nospace() << ", pixelAspectRatio=" << f.pixelAspectRatio();
   dbg.nospace() << ")";

   for (const QString & propertyName : f.propertyNames()) {
      dbg << "\n    " << propertyName << " = " << f.property(propertyName);
   }

   return dbg.space();
}

