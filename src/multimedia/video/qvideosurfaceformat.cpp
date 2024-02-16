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

#include <qvideosurfaceformat.h>

#include <qdebug.h>
#include <qpair.h>
#include <qvariant.h>
#include <qvector.h>

class QVideoSurfaceFormatPrivate : public QSharedData
{
 public:
   QVideoSurfaceFormatPrivate()
      : pixelFormat(QVideoFrame::Format_Invalid), handleType(QAbstractVideoBuffer::NoHandle),
        scanLineDirection(QVideoSurfaceFormat::TopToBottom), pixelAspectRatio(1, 1),
        ycbcrColorSpace(QVideoSurfaceFormat::YCbCr_Undefined), frameRate(0.0), mirrored(false)
   {
   }

   QVideoSurfaceFormatPrivate(const QSize &size, QVideoFrame::PixelFormat format,
         QAbstractVideoBuffer::HandleType type)
      : pixelFormat(format), handleType(type), scanLineDirection(QVideoSurfaceFormat::TopToBottom),
        frameSize(size), pixelAspectRatio(1, 1), ycbcrColorSpace(QVideoSurfaceFormat::YCbCr_Undefined),
        viewport(QPoint(0, 0), size), frameRate(0.0), mirrored(false)
 {
   }

   QVideoSurfaceFormatPrivate(const QVideoSurfaceFormatPrivate &other)
      : QSharedData(other), pixelFormat(other.pixelFormat), handleType(other.handleType),
        scanLineDirection(other.scanLineDirection), frameSize(other.frameSize),
        pixelAspectRatio(other.pixelAspectRatio), ycbcrColorSpace(other.ycbcrColorSpace),
        viewport(other.viewport), frameRate(other.frameRate), mirrored(other.mirrored),
        propertyNames(other.propertyNames), propertyValues(other.propertyValues)
   {
   }

   bool operator ==(const QVideoSurfaceFormatPrivate &other) const {
      if (pixelFormat == other.pixelFormat && handleType == other.handleType
            && scanLineDirection == other.scanLineDirection && frameSize == other.frameSize
            && pixelAspectRatio == other.pixelAspectRatio && viewport == other.viewport
            && frameRatesEqual(frameRate, other.frameRate)
            && ycbcrColorSpace == other.ycbcrColorSpace && mirrored == other.mirrored
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

   static bool frameRatesEqual(qreal r1, qreal r2) {
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
   bool mirrored;

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

QVideoSurfaceFormat::QVideoSurfaceFormat(const QVideoSurfaceFormat &other)
   : d(other.d)
{
}

QVideoSurfaceFormat &QVideoSurfaceFormat::operator =(const QVideoSurfaceFormat &other)
{
   d = other.d;

   return *this;
}

QVideoSurfaceFormat::~QVideoSurfaceFormat()
{
}

bool QVideoSurfaceFormat::isValid() const
{
   return d->pixelFormat != QVideoFrame::Format_Invalid && d->frameSize.isValid();
}

bool QVideoSurfaceFormat::operator ==(const QVideoSurfaceFormat &other) const
{
   return d == other.d || *d == *other.d;
}

bool QVideoSurfaceFormat::operator !=(const QVideoSurfaceFormat &other) const
{
   return d != other.d && !(*d == *other.d);
}

QVideoFrame::PixelFormat QVideoSurfaceFormat::pixelFormat() const
{
   return d->pixelFormat;
}

QAbstractVideoBuffer::HandleType QVideoSurfaceFormat::handleType() const
{
   return d->handleType;
}

QSize QVideoSurfaceFormat::frameSize() const
{
   return d->frameSize;
}

int QVideoSurfaceFormat::frameWidth() const
{
   return d->frameSize.width();
}

int QVideoSurfaceFormat::frameHeight() const
{
   return d->frameSize.height();
}

void QVideoSurfaceFormat::setFrameSize(const QSize &size)
{
   d->frameSize = size;
   d->viewport = QRect(QPoint(0, 0), size);
}

void QVideoSurfaceFormat::setFrameSize(int width, int height)
{
   d->frameSize = QSize(width, height);
   d->viewport = QRect(0, 0, width, height);
}

QRect QVideoSurfaceFormat::viewport() const
{
   return d->viewport;
}

void QVideoSurfaceFormat::setViewport(const QRect &viewport)
{
   d->viewport = viewport;
}

QVideoSurfaceFormat::Direction QVideoSurfaceFormat::scanLineDirection() const
{
   return d->scanLineDirection;
}

void QVideoSurfaceFormat::setScanLineDirection(Direction direction)
{
   d->scanLineDirection = direction;
}

qreal QVideoSurfaceFormat::frameRate() const
{
   return d->frameRate;
}

void QVideoSurfaceFormat::setFrameRate(qreal rate)
{
   d->frameRate = rate;
}

QSize QVideoSurfaceFormat::pixelAspectRatio() const
{
   return d->pixelAspectRatio;
}

void QVideoSurfaceFormat::setPixelAspectRatio(const QSize &ratio)
{
   d->pixelAspectRatio = ratio;
}

void QVideoSurfaceFormat::setPixelAspectRatio(int horizontal, int vertical)
{
   d->pixelAspectRatio = QSize(horizontal, vertical);
}

QVideoSurfaceFormat::YCbCrColorSpace QVideoSurfaceFormat::yCbCrColorSpace() const
{
   return d->ycbcrColorSpace;
}

void QVideoSurfaceFormat::setYCbCrColorSpace(QVideoSurfaceFormat::YCbCrColorSpace space)
{
   d->ycbcrColorSpace = space;
}

QSize QVideoSurfaceFormat::sizeHint() const
{
   QSize size = d->viewport.size();

   if (d->pixelAspectRatio.height() != 0) {
      size.setWidth(size.width() * d->pixelAspectRatio.width() / d->pixelAspectRatio.height());
   }

   return size;
}

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
         << "yCbCrColorSpace"
         << "mirrored")
      + d->propertyNames;
}

QVariant QVideoSurfaceFormat::property(const QString &name) const
{
   if (name == "handleType") {
      return QVariant::fromValue(d->handleType);

   } else if (name == "pixelFormat") {
      return QVariant::fromValue(d->pixelFormat);

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
   } else if (name == "mirrored") {
      return d->mirrored;

   } else {
      int id = 0;

      for (; id < d->propertyNames.count() && d->propertyNames.at(id) != name; ++id) {
         // do nothing
      }

      return id < d->propertyValues.count() ? d->propertyValues.at(id) : QVariant();
   }
}

void QVideoSurfaceFormat::setProperty(QStringView name, const QVariant &value)
{
   if (name == "handleType") {
      // read only

   } else if (name == "pixelFormat") {
      // read only

   } else if (name == "frameSize") {
      if (value.canConvert<QSize>()) {
         d->frameSize = value.value<QSize>();
         d->viewport = QRect(QPoint(0, 0), d->frameSize);
      }

   } else if (name == "frameWidth") {
      // read only.

   } else if (name == "frameHeight") {
      // read only.

   } else if (name == "viewport") {
      if (value.canConvert<QRect>()) {
         d->viewport = value.value<QRect>();
      }

   } else if (name == "scanLineDirection") {
      if (value.canConvert<Direction>()) {
         d->scanLineDirection = value.value<Direction>();
      }

   } else if (name == "frameRate") {
      if (value.canConvert<qreal>()) {
         d->frameRate = value.value<qreal>();
      }

   } else if (name == "pixelAspectRatio") {
      if (value.canConvert<QSize>()) {
         d->pixelAspectRatio = value.value<QSize>();
      }

   } else if (name == "sizeHint") {
      // read only

   } else if (name == "yCbCrColorSpace") {
      if (value.canConvert<YCbCrColorSpace>()) {
         d->ycbcrColorSpace = value.value<YCbCrColorSpace>();
      }

   } else if (name == "mirrored") {
      if (value.canConvert<bool>()) {
         d->mirrored = value.value<bool>();
      }

   } else {
      int id = 0;

      for (; id < d->propertyNames.count() && d->propertyNames.at(id) != name; ++id) {
         //
      }

      if (id < d->propertyValues.count()) {
         if (! value.isValid()) {
            d->propertyNames.removeAt(id);
            d->propertyValues.removeAt(id);

         } else {
            d->propertyValues[id] = value;
         }

      } else if (value.isValid()) {
         d->propertyNames.append(name);
         d->propertyValues.append(value);
      }
   }
}

QDebug operator<<(QDebug dbg, QVideoSurfaceFormat::YCbCrColorSpace cs)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (cs) {
      case QVideoSurfaceFormat::YCbCr_BT601:
         dbg << "YCbCr_BT601";
         break;
      case QVideoSurfaceFormat::YCbCr_BT709:
         dbg << "YCbCr_BT709";
         break;
      case QVideoSurfaceFormat::YCbCr_JPEG:
         dbg << "YCbCr_JPEG";
         break;
      case QVideoSurfaceFormat::YCbCr_xvYCC601:
         dbg << "YCbCr_xvYCC601";
         break;
      case QVideoSurfaceFormat::YCbCr_xvYCC709:
         dbg << "YCbCr_xvYCC709";
         break;
      case QVideoSurfaceFormat::YCbCr_CustomMatrix:
         dbg << "YCbCr_CustomMatrix";
         break;
      default:
         dbg << "YCbCr_Undefined";
         break;
   }

   return dbg;
}

QDebug operator<<(QDebug dbg, QVideoSurfaceFormat::Direction dir)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (dir) {
      case QVideoSurfaceFormat::BottomToTop:
         dbg << "BottomToTop";
         break;

      case QVideoSurfaceFormat::TopToBottom:
         dbg << "TopToBottom";
         break;
   }

   return dbg;
}

QDebug operator<<(QDebug dbg, const QVideoSurfaceFormat &f)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   dbg << "QVideoSurfaceFormat(" << f.pixelFormat() << ", " << f.frameSize()
      << ", viewport=" << f.viewport() << ", pixelAspectRatio=" << f.pixelAspectRatio()
      << ", handleType=" << f.handleType() <<  ", yCbCrColorSpace=" << f.yCbCrColorSpace()
      << ')';

   for (const QString &propertyName : f.propertyNames()) {
      dbg << "\n    " << propertyName << " = " << f.property(propertyName).toString();
   }

   return dbg;
}
