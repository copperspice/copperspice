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

#include <qvideoframe.h>

#include <qimagevideobuffer_p.h>
#include <qmemoryvideobuffer_p.h>
#include <qimage.h>
#include <qpair.h>
#include <qsize.h>
#include <qvariant.h>
#include <qvector.h>

QT_BEGIN_NAMESPACE

class QVideoFramePrivate : public QSharedData
{
 public:
   QVideoFramePrivate()
      : startTime(-1)
      , endTime(-1)
      , data(0)
      , mappedBytes(0)
      , bytesPerLine(0)
      , pixelFormat(QVideoFrame::Format_Invalid)
      , fieldType(QVideoFrame::ProgressiveFrame)
      , buffer(0) {
   }

   QVideoFramePrivate(const QSize &size, QVideoFrame::PixelFormat format)
      : size(size)
      , startTime(-1)
      , endTime(-1)
      , data(0)
      , mappedBytes(0)
      , bytesPerLine(0)
      , pixelFormat(format)
      , fieldType(QVideoFrame::ProgressiveFrame)
      , buffer(0) {
   }

   ~QVideoFramePrivate() {
      delete buffer;
   }

   QSize size;
   qint64 startTime;
   qint64 endTime;
   uchar *data;
   int mappedBytes;
   int bytesPerLine;
   QVideoFrame::PixelFormat pixelFormat;
   QVideoFrame::FieldType fieldType;
   QAbstractVideoBuffer *buffer;

 private:
   Q_DISABLE_COPY(QVideoFramePrivate)
};

/*!
    \class QVideoFrame
    \brief The QVideoFrame class provides a representation of a frame of video data.
    \since 4.6

    A QVideoFrame encapsulates the data of a video frame, and information about the frame.

    The contents of a video frame can be mapped to memory using the map() function.  While
    mapped the video data can accessed using the bits() function which returns a pointer to a
    buffer, the total size of which is given by the mappedBytes(), and the size of each line is given
    by bytesPerLine().  The return value of the handle() function may be used to access frame data
    using the internal buffer's native APIs.

    The video data in a QVideoFrame is encapsulated in a QAbstractVideoBuffer.  A QVideoFrame
    may be constructed from any buffer type by subclassing the QAbstractVideoBuffer class.

    \note QVideoFrame is explicitly shared, any change made to video frame will also apply to any
    copies.
*/

/*!
    \enum QVideoFrame::PixelFormat

    Enumerates video data types.

    \value Format_Invalid
    The frame is invalid.

    \value Format_ARGB32
    The frame is stored using a 32-bit ARGB format (0xAARRGGBB).  This is equivalent to
    QImage::Format_ARGB32.

    \value Format_ARGB32_Premultiplied
    The frame stored using a premultiplied 32-bit ARGB format (0xAARRGGBB).  This is equivalent
    to QImage::Format_ARGB32_Premultiplied.

    \value Format_RGB32
    The frame stored using a 32-bit RGB format (0xffRRGGBB).  This is equivalent to
    QImage::Format_RGB32

    \value Format_RGB24
    The frame is stored using a 24-bit RGB format (8-8-8).  This is equivalent to
    QImage::Format_RGB888

    \value Format_RGB565
    The frame is stored using a 16-bit RGB format (5-6-5).  This is equivalent to
    QImage::Format_RGB16.

    \value Format_RGB555
    The frame is stored using a 16-bit RGB format (5-5-5).  This is equivalent to
    QImage::Format_RGB555.

    \value Format_ARGB8565_Premultiplied
    The frame is stored using a 24-bit premultiplied ARGB format (8-6-6-5).

    \value Format_BGRA32
    The frame is stored using a 32-bit ARGB format (0xBBGGRRAA).

    \value Format_BGRA32_Premultiplied
    The frame is stored using a premultiplied 32bit BGRA format.

    \value Format_BGR32
    The frame is stored using a 32-bit BGR format (0xBBGGRRff).

    \value Format_BGR24
    The frame is stored using a 24-bit BGR format (0xBBGGRR).

    \value Format_BGR565
    The frame is stored using a 16-bit BGR format (5-6-5).

    \value Format_BGR555
    The frame is stored using a 16-bit BGR format (5-5-5).

    \value Format_BGRA5658_Premultiplied
    The frame is stored using a 24-bit premultiplied BGRA format (5-6-5-8).

    \value Format_AYUV444
    The frame is stored using a packed 32-bit AYUV format (0xAAYYUUVV).

    \value Format_AYUV444_Premultiplied
    The frame is stored using a packed premultiplied 32-bit AYUV format (0xAAYYUUVV).

    \value Format_YUV444
    The frame is stored using a 24-bit packed YUV format (8-8-8).

    \value Format_YUV420P
    The frame is stored using an 8-bit per component planar YUV format with the U and V planes
    horizontally and vertically sub-sampled, i.e. the height and width of the U and V planes are
    half that of the Y plane.

    \value Format_YV12
    The frame is stored using an 8-bit per component planar YVU format with the V and U planes
    horizontally and vertically sub-sampled, i.e. the height and width of the V and U planes are
    half that of the Y plane.

    \value Format_UYVY
    The frame is stored using an 8-bit per component packed YUV format with the U and V planes
    horizontally sub-sampled (U-Y-V-Y), i.e. two horizontally adjacent pixels are stored as a 32-bit
    macropixel which has a Y value for each pixel and common U and V values.

    \value Format_YUYV
    The frame is stored using an 8-bit per component packed YUV format with the U and V planes
    horizontally sub-sampled (Y-U-Y-V), i.e. two horizontally adjacent pixels are stored as a 32-bit
    macropixel which has a Y value for each pixel and common U and V values.

    \value Format_NV12
    The frame is stored using an 8-bit per component semi-planar YUV format with a Y plane (Y)
    followed by a horizontally and vertically sub-sampled, packed UV plane (U-V).

    \value Format_NV21
    The frame is stored using an 8-bit per component semi-planar YUV format with a Y plane (Y)
    followed by a horizontally and vertically sub-sampled, packed VU plane (V-U).

    \value Format_IMC1
    The frame is stored using an 8-bit per component planar YUV format with the U and V planes
    horizontally and vertically sub-sampled.  This is similar to the Format_YUV420P type, except
    that the bytes per line of the U and V planes are padded out to the same stride as the Y plane.

    \value Format_IMC2
    The frame is stored using an 8-bit per component planar YUV format with the U and V planes
    horizontally and vertically sub-sampled.  This is similar to the Format_YUV420P type, except
    that the lines of the U and V planes are interleaved, i.e. each line of U data is followed by a
    line of V data creating a single line of the same stride as the Y data.

    \value Format_IMC3
    The frame is stored using an 8-bit per component planar YVU format with the V and U planes
    horizontally and vertically sub-sampled.  This is similar to the Format_YV12 type, except that
    the bytes per line of the V and U planes are padded out to the same stride as the Y plane.

    \value Format_IMC4
    The frame is stored using an 8-bit per component planar YVU format with the V and U planes
    horizontally and vertically sub-sampled.  This is similar to the Format_YV12 type, except that
    the lines of the V and U planes are interleaved, i.e. each line of V data is followed by a line
    of U data creating a single line of the same stride as the Y data.

    \value Format_Y8
    The frame is stored using an 8-bit greyscale format.

    \value Format_Y16
    The frame is stored using a 16-bit linear greyscale format.  Little endian.

    \value Format_User
    Start value for user defined pixel formats.
*/

/*!
    \enum QVideoFrame::FieldType

    Specifies the field an interlaced video frame belongs to.

    \value ProgressiveFrame The frame is not interlaced.
    \value TopField The frame contains a top field.
    \value BottomField The frame contains a bottom field.
    \value InterlacedFrame The frame contains a merged top and bottom field.
*/

/*!
    Constructs a null video frame.
*/

QVideoFrame::QVideoFrame()
   : d(new QVideoFramePrivate)
{
}

/*!
    Constructs a video frame from a \a buffer of the given pixel \a format and \a size in pixels.

    \note This doesn't increment the reference count of the video buffer.
*/

QVideoFrame::QVideoFrame(
   QAbstractVideoBuffer *buffer, const QSize &size, PixelFormat format)
   : d(new QVideoFramePrivate(size, format))
{
   d->buffer = buffer;
}

/*!
    Constructs a video frame of the given pixel \a format and \a size in pixels.

    The \a bytesPerLine (stride) is the length of each scan line in bytes, and \a bytes is the total
    number of bytes that must be allocated for the frame.
*/

QVideoFrame::QVideoFrame(int bytes, const QSize &size, int bytesPerLine, PixelFormat format)
   : d(new QVideoFramePrivate(size, format))
{
   if (bytes > 0) {
      QByteArray data;
      data.resize(bytes);

      // Check the memory was successfully allocated.
      if (!data.isEmpty()) {
         d->buffer = new QMemoryVideoBuffer(data, bytesPerLine);
      }
   }
}

/*!
    Constructs a video frame from an \a image.

    \note This will construct an invalid video frame if there is no frame type equivalent to the
    image format.

    \sa pixelFormatFromImageFormat()
*/

QVideoFrame::QVideoFrame(const QImage &image)
   : d(new QVideoFramePrivate(
          image.size(), pixelFormatFromImageFormat(image.format())))
{
   if (d->pixelFormat != Format_Invalid) {
      d->buffer = new QImageVideoBuffer(image);
   }
}

/*!
    Constructs a copy of \a other.
*/

QVideoFrame::QVideoFrame(const QVideoFrame &other)
   : d(other.d)
{
}

/*!
    Assigns the contents of \a other to a video frame.
*/

QVideoFrame &QVideoFrame::operator =(const QVideoFrame &other)
{
   d = other.d;

   return *this;
}

/*!
    Destroys a video frame.
*/

QVideoFrame::~QVideoFrame()
{
}

/*!
    Identifies whether a video frame is valid.

    An invalid frame has no video buffer associated with it.

    Returns true if the frame is valid, and false if it is not.
*/

bool QVideoFrame::isValid() const
{
   return d->buffer != 0;
}

/*!
    Returns the color format of a video frame.
*/

QVideoFrame::PixelFormat QVideoFrame::pixelFormat() const
{
   return d->pixelFormat;
}

/*!
    Returns the type of a video frame's handle.
*/

QAbstractVideoBuffer::HandleType QVideoFrame::handleType() const
{
   return d->buffer ? d->buffer->handleType() : QAbstractVideoBuffer::NoHandle;
}

/*!
    Returns the size of a video frame.
*/

QSize QVideoFrame::size() const
{
   return d->size;
}

/*!
    Returns the width of a video frame.
*/

int QVideoFrame::width() const
{
   return d->size.width();
}

/*!
    Returns the height of a video frame.
*/

int QVideoFrame::height() const
{
   return d->size.height();
}

/*!
    Returns the field an interlaced video frame belongs to.

    If the video is not interlaced this will return WholeFrame.
*/

QVideoFrame::FieldType QVideoFrame::fieldType() const
{
   return d->fieldType;
}

/*!
    Sets the \a field an interlaced video frame belongs to.
*/

void QVideoFrame::setFieldType(QVideoFrame::FieldType field)
{
   d->fieldType = field;
}

/*!
    Identifies if a video frame's contents are currently mapped to system memory.

    This is a convenience function which checks that the \l {QAbstractVideoBuffer::MapMode}{MapMode}
    of the frame is not equal to QAbstractVideoBuffer::NotMapped.

    Returns true if the contents of the video frame are mapped to system memory, and false
    otherwise.

    \sa mapMode() QAbstractVideoBuffer::MapMode
*/

bool QVideoFrame::isMapped() const
{
   return d->buffer != 0 && d->buffer->mapMode() != QAbstractVideoBuffer::NotMapped;
}

/*!
    Identifies if the mapped contents of a video frame will be persisted when the frame is unmapped.

    This is a convenience function which checks if the \l {QAbstractVideoBuffer::MapMode}{MapMode}
    contains the QAbstractVideoBuffer::WriteOnly flag.

    Returns true if the video frame will be updated when unmapped, and false otherwise.

    \note The result of altering the data of a frame that is mapped in read-only mode is undefined.
    Depending on the buffer implementation the changes may be persisted, or worse alter a shared
    buffer.

    \sa mapMode(), QAbstractVideoBuffer::MapMode
*/

bool QVideoFrame::isWritable() const
{
   return d->buffer != 0 && (d->buffer->mapMode() & QAbstractVideoBuffer::WriteOnly);
}

/*!
    Identifies if the mapped contents of a video frame were read from the frame when it was mapped.

    This is a convenience function which checks if the \l {QAbstractVideoBuffer::MapMode}{MapMode}
    contains the QAbstractVideoBuffer::WriteOnly flag.

    Returns true if the contents of the mapped memory were read from the video frame, and false
    otherwise.

    \sa mapMode(), QAbstractVideoBuffer::MapMode
*/

bool QVideoFrame::isReadable() const
{
   return d->buffer != 0 && (d->buffer->mapMode() & QAbstractVideoBuffer::ReadOnly);
}

/*!
    Returns the mode a video frame was mapped to system memory in.

    \sa map(), QAbstractVideoBuffer::MapMode
*/

QAbstractVideoBuffer::MapMode QVideoFrame::mapMode() const
{
   return d->buffer != 0 ? d->buffer->mapMode() : QAbstractVideoBuffer::NotMapped;
}

/*!
    Maps the contents of a video frame to memory.

    The map \a mode indicates whether the contents of the mapped memory should be read from and/or
    written to the frame.  If the map mode includes the QAbstractVideoBuffer::ReadOnly flag the
    mapped memory will be populated with the content of the video frame when mapped.  If the map
    mode inclues the QAbstractVideoBuffer::WriteOnly flag the content of the mapped memory will be
    persisted in the frame when unmapped.

    While mapped the contents of a video frame can be accessed directly through the pointer returned
    by the bits() function.

    When access to the data is no longer needed be sure to call the unmap() function to release the
    mapped memory.

    Returns true if the buffer was mapped to memory in the given \a mode and false otherwise.

    \sa unmap(), mapMode(), bits()
*/

bool QVideoFrame::map(QAbstractVideoBuffer::MapMode mode)
{
   if (d->buffer != 0 && d->data == 0) {
      Q_ASSERT(d->bytesPerLine == 0);
      Q_ASSERT(d->mappedBytes == 0);

      d->data = d->buffer->map(mode, &d->mappedBytes, &d->bytesPerLine);

      return d->data != 0;
   }

   return false;
}

/*!
    Releases the memory mapped by the map() function.

    If the \l {QAbstractVideoBuffer::MapMode}{MapMode} included the QAbstractVideoBuffer::WriteOnly
    flag this will persist the current content of the mapped memory to the video frame.

    \sa map()
*/

void QVideoFrame::unmap()
{
   if (d->data != 0) {
      d->mappedBytes = 0;
      d->bytesPerLine = 0;
      d->data = 0;

      d->buffer->unmap();
   }
}

/*!
    Returns the number of bytes in a scan line.

    \note This is the bytes per line of the first plane only.  The bytes per line of subsequent
    planes should be calculated as per the frame type.

    This value is only valid while the frame data is \l {map()}{mapped}.

    \sa bits(), map(), mappedBytes()
*/

int QVideoFrame::bytesPerLine() const
{
   return d->bytesPerLine;
}

/*!
    Returns a pointer to the start of the frame data buffer.

    This value is only valid while the frame data is \l {map()}{mapped}.

    \sa map(), mappedBytes(), bytesPerLine()
*/

uchar *QVideoFrame::bits()
{
   return d->data;
}

/*!
    Returns a pointer to the start of the frame data buffer.

    This value is only valid while the frame data is \l {map()}{mapped}.

    \sa map(), mappedBytes(), bytesPerLine()
*/

const uchar *QVideoFrame::bits() const
{
   return d->data;
}

/*!
    Returns the number of bytes occupied by the mapped frame data.

    This value is only valid while the frame data is \l {map()}{mapped}.

    \sa map()
*/

int QVideoFrame::mappedBytes() const
{
   return d->mappedBytes;
}

/*!
    Returns a type specific handle to a video frame's buffer.

    For an OpenGL texture this would be the texture ID.

    \sa QAbstractVideoBuffer::handle()
*/

QVariant QVideoFrame::handle() const
{
   return d->buffer != 0 ? d->buffer->handle() : QVariant();
}

/*!
    Returns the presentation time when the frame should be displayed.
*/

qint64 QVideoFrame::startTime() const
{
   return d->startTime;
}

/*!
    Sets the presentation \a time when the frame should be displayed.
*/

void QVideoFrame::setStartTime(qint64 time)
{
   d->startTime = time;
}

/*!
    Returns the presentation time when a frame should stop being displayed.
*/

qint64 QVideoFrame::endTime() const
{
   return d->endTime;
}

/*!
    Sets the presentation \a time when a frame should stop being displayed.
*/

void QVideoFrame::setEndTime(qint64 time)
{
   d->endTime = time;
}

/*!
    Returns an video pixel format equivalent to an image \a format.  If there is no equivalent
    format QVideoFrame::InvalidType is returned instead.
*/

QVideoFrame::PixelFormat QVideoFrame::pixelFormatFromImageFormat(QImage::Format format)
{
   switch (format) {
      case QImage::Format_Invalid:
      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
      case QImage::Format_Indexed8:
         return Format_Invalid;
      case QImage::Format_RGB32:
         return Format_RGB32;
      case QImage::Format_ARGB32:
         return Format_ARGB32;
      case QImage::Format_ARGB32_Premultiplied:
         return Format_ARGB32_Premultiplied;
      case QImage::Format_RGB16:
         return Format_RGB565;
      case QImage::Format_ARGB8565_Premultiplied:
         return Format_ARGB8565_Premultiplied;
      case QImage::Format_RGB666:
      case QImage::Format_ARGB6666_Premultiplied:
         return Format_Invalid;
      case QImage::Format_RGB555:
         return Format_RGB555;
      case QImage::Format_ARGB8555_Premultiplied:
         return Format_Invalid;
      case QImage::Format_RGB888:
         return Format_RGB24;
      case QImage::Format_RGB444:
      case QImage::Format_ARGB4444_Premultiplied:
         return Format_Invalid;
      case QImage::NImageFormats:
         return Format_Invalid;
   }
   return Format_Invalid;
}

/*!
    Returns an image format equivalent to a video frame pixel \a format.  If there is no equivalent
    format QImage::Format_Invalid is returned instead.
*/

QImage::Format QVideoFrame::imageFormatFromPixelFormat(PixelFormat format)
{
   switch (format) {
      case Format_Invalid:
         return QImage::Format_Invalid;
      case Format_ARGB32:
         return QImage::Format_ARGB32;
      case Format_ARGB32_Premultiplied:
         return QImage::Format_ARGB32_Premultiplied;
      case Format_RGB32:
         return QImage::Format_RGB32;
      case Format_RGB24:
         return QImage::Format_RGB888;
      case Format_RGB565:
         return QImage::Format_RGB16;
      case Format_RGB555:
         return QImage::Format_RGB555;
      case Format_ARGB8565_Premultiplied:
         return QImage::Format_ARGB8565_Premultiplied;
      case Format_BGRA32:
      case Format_BGRA32_Premultiplied:
      case Format_BGR32:
      case Format_BGR24:
         return QImage::Format_Invalid;
      case Format_BGR565:
      case Format_BGR555:
      case Format_BGRA5658_Premultiplied:
      case Format_AYUV444:
      case Format_AYUV444_Premultiplied:
      case Format_YUV444:
      case Format_YUV420P:
      case Format_YV12:
      case Format_UYVY:
      case Format_YUYV:
      case Format_NV12:
      case Format_NV21:
      case Format_IMC1:
      case Format_IMC2:
      case Format_IMC3:
      case Format_IMC4:
      case Format_Y8:
      case Format_Y16:
         return QImage::Format_Invalid;
      case Format_User:
         return QImage::Format_Invalid;
   }
   return QImage::Format_Invalid;
}

QT_END_NAMESPACE

