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

#include <qvideoframe.h>

#include <qimage.h>
#include <qpair.h>
#include <qsize.h>
#include <qvariant.h>
#include <qvector.h>
#include <qmutex.h>
#include <qdebug.h>

#include <qvideoframe_p.h>
#include <qimagevideobuffer_p.h>
#include <qmemoryvideobuffer_p.h>
#include <qvideoframeconversionhelper_p.h>

class QVideoFramePrivate : public QSharedData
{
 public:
   QVideoFramePrivate()
      : startTime(-1), endTime(-1), mappedBytes(0), planeCount(0),
        pixelFormat(QVideoFrame::Format_Invalid), fieldType(QVideoFrame::ProgressiveFrame),
        buffer(nullptr), mappedCount(0)
   {
      memset(data, 0, sizeof(data));
      memset(bytesPerLine, 0, sizeof(bytesPerLine));
   }

   QVideoFramePrivate(const QSize &size, QVideoFrame::PixelFormat format)
      : size(size), startTime(-1), endTime(-1), mappedBytes(0), planeCount(0),
        pixelFormat(format), fieldType(QVideoFrame::ProgressiveFrame),
        buffer(nullptr), mappedCount(0)
   {
      memset(data, 0, sizeof(data));
      memset(bytesPerLine, 0, sizeof(bytesPerLine));
   }

   QVideoFramePrivate(const QVideoFramePrivate &) = delete;
   QVideoFramePrivate &operator=(const QVideoFramePrivate &) = delete;

   ~QVideoFramePrivate() {
      if (buffer) {
         buffer->release();
      }
   }

   QSize size;
   qint64 startTime;
   qint64 endTime;
   uchar *data[4];
   int bytesPerLine[4];
   int mappedBytes;
   int planeCount;
   QVideoFrame::PixelFormat pixelFormat;
   QVideoFrame::FieldType fieldType;
   QAbstractVideoBuffer *buffer;
   int mappedCount;
   QMutex mapMutex;
   QVariantMap metadata;
};

QVideoFrame::QVideoFrame()
   : d(new QVideoFramePrivate)
{
}

QVideoFrame::QVideoFrame(
   QAbstractVideoBuffer *buffer, const QSize &size, PixelFormat format)
   : d(new QVideoFramePrivate(size, format))
{
   d->buffer = buffer;
}

QVideoFrame::QVideoFrame(int bytes, const QSize &size, int bytesPerLine, PixelFormat format)
   : d(new QVideoFramePrivate(size, format))
{
   if (bytes > 0) {
      QByteArray data;
      data.resize(bytes);

      // Check the memory was successfully allocated.
      if (! data.isEmpty()) {
         d->buffer = new QMemoryVideoBuffer(data, bytesPerLine);
      }
   }
}

QVideoFrame::QVideoFrame(const QImage &image)
   : d(new QVideoFramePrivate(
           image.size(), pixelFormatFromImageFormat(image.format())))
{
   if (d->pixelFormat != Format_Invalid) {
      d->buffer = new QImageVideoBuffer(image);
   }
}

QVideoFrame::QVideoFrame(const QVideoFrame &other)
   : d(other.d)
{
}

QVideoFrame &QVideoFrame::operator =(const QVideoFrame &other)
{
   d = other.d;

   return *this;
}

bool QVideoFrame::operator==(const QVideoFrame &other) const
{
   // Due to explicit sharing we just compare the QSharedData which in turn compares the pointers.
   return d == other.d;
}

bool QVideoFrame::operator!=(const QVideoFrame &other) const
{
   return d != other.d;
}

QVideoFrame::~QVideoFrame()
{
}

bool QVideoFrame::isValid() const
{
   return d->buffer != nullptr;
}

QVideoFrame::PixelFormat QVideoFrame::pixelFormat() const
{
   return d->pixelFormat;
}

QAbstractVideoBuffer::HandleType QVideoFrame::handleType() const
{
   return d->buffer ? d->buffer->handleType() : QAbstractVideoBuffer::NoHandle;
}

QSize QVideoFrame::size() const
{
   return d->size;
}

int QVideoFrame::width() const
{
   return d->size.width();
}

int QVideoFrame::height() const
{
   return d->size.height();
}

QVideoFrame::FieldType QVideoFrame::fieldType() const
{
   return d->fieldType;
}

void QVideoFrame::setFieldType(QVideoFrame::FieldType field)
{
   d->fieldType = field;
}

bool QVideoFrame::isMapped() const
{
   return d->buffer != nullptr && d->buffer->mapMode() != QAbstractVideoBuffer::NotMapped;
}

bool QVideoFrame::isWritable() const
{
   return d->buffer != nullptr && (d->buffer->mapMode() & QAbstractVideoBuffer::WriteOnly);
}

bool QVideoFrame::isReadable() const
{
   return d->buffer != nullptr && (d->buffer->mapMode() & QAbstractVideoBuffer::ReadOnly);
}

QAbstractVideoBuffer::MapMode QVideoFrame::mapMode() const
{
   return d->buffer != nullptr ? d->buffer->mapMode() : QAbstractVideoBuffer::NotMapped;
}

bool QVideoFrame::map(QAbstractVideoBuffer::MapMode mode)
{
   QMutexLocker lock(&d->mapMutex);

   if (! d->buffer) {
      return false;
   }

   if (mode == QAbstractVideoBuffer::NotMapped) {
      return false;
   }

   if (d->mappedCount > 0) {
      //it's allowed to map the video frame multiple times in read only mode
      if (d->buffer->mapMode() == QAbstractVideoBuffer::ReadOnly
         && mode == QAbstractVideoBuffer::ReadOnly) {
         d->mappedCount++;
         return true;
      } else {
         return false;
      }
   }

   Q_ASSERT(d->data[0] == nullptr);
   Q_ASSERT(d->bytesPerLine[0] == 0);
   Q_ASSERT(d->planeCount == 0);
   Q_ASSERT(d->mappedBytes == 0);

   d->planeCount = d->buffer->mapPlanes(mode, &d->mappedBytes, d->bytesPerLine, d->data);
   if (d->planeCount == 0) {
      return false;
   }

   if (d->planeCount > 1) {
      // If the plane count is derive the additional planes for planar formats

   } else
      switch (d->pixelFormat) {
         case Format_Invalid:
         case Format_ARGB32:
         case Format_ARGB32_Premultiplied:
         case Format_RGB32:
         case Format_RGB24:
         case Format_RGB565:
         case Format_RGB555:
         case Format_ARGB8565_Premultiplied:
         case Format_BGRA32:
         case Format_BGRA32_Premultiplied:
         case Format_BGR32:
         case Format_BGR24:
         case Format_BGR565:
         case Format_BGR555:
         case Format_BGRA5658_Premultiplied:
         case Format_AYUV444:
         case Format_AYUV444_Premultiplied:
         case Format_YUV444:
         case Format_UYVY:
         case Format_YUYV:
         case Format_Y8:
         case Format_Y16:
         case Format_Jpeg:
         case Format_CameraRaw:
         case Format_AdobeDng:
         case Format_User:
            // Single plane or opaque format.
            break;

         case Format_YUV420P:
         case Format_YV12: {
            // The UV stride is usually half the Y stride and is 32-bit aligned.
            // However it's not always the case, at least on Windows where the
            // UV planes are sometimes not aligned.
            // We calculate the stride using the UV byte count to always
            // have a correct stride.
            const int height = d->size.height();
            const int yStride = d->bytesPerLine[0];
            const int uvStride = (d->mappedBytes - (yStride * height)) / height;

            // Three planes, the second and third vertically and horizontally subsampled.
            d->planeCount = 3;
            d->bytesPerLine[2] = d->bytesPerLine[1] = uvStride;
            d->data[1] = d->data[0] + (yStride * height);
            d->data[2] = d->data[1] + (uvStride * height / 2);
            break;
         }

         case Format_NV12:
         case Format_NV21:
         case Format_IMC2:
         case Format_IMC4: {
            // Semi planar, Full resolution Y plane with interleaved subsampled U and V planes.
            d->planeCount = 2;
            d->bytesPerLine[1] = d->bytesPerLine[0];
            d->data[1] = d->data[0] + (d->bytesPerLine[0] * d->size.height());
            break;
         }

         case Format_IMC1:
         case Format_IMC3: {
            // Three planes, the second and third vertically and horizontally subsumpled,
            // but with lines padded to the width of the first plane.
            d->planeCount = 3;
            d->bytesPerLine[2] = d->bytesPerLine[1] = d->bytesPerLine[0];
            d->data[1] = d->data[0] + (d->bytesPerLine[0] * d->size.height());
            d->data[2] = d->data[1] + (d->bytesPerLine[1] * d->size.height() / 2);
            break;
         }

         default:
            break;
      }

   d->mappedCount++;
   return true;
}

void QVideoFrame::unmap()
{
   QMutexLocker lock(&d->mapMutex);

   if (!d->buffer) {
      return;
   }

   if (d->mappedCount == 0) {
      qWarning() << "QVideoFrame::unmap() was called more times then QVideoFrame::map()";
      return;
   }

   --d->mappedCount;

   if (d->mappedCount == 0) {
      d->mappedBytes = 0;
      d->planeCount  = 0;

      memset(d->bytesPerLine, 0, sizeof(d->bytesPerLine));
      memset(d->data, 0, sizeof(d->data));

      d->buffer->unmap();
   }
}

int QVideoFrame::bytesPerLine() const
{
   return d->bytesPerLine[0];
}

int QVideoFrame::bytesPerLine(int plane) const
{
   return plane >= 0 && plane < d->planeCount ? d->bytesPerLine[plane] : 0;
}

uchar *QVideoFrame::bits()
{
   return d->data[0];
}

uchar *QVideoFrame::bits(int plane)
{
   return plane >= 0 && plane < d->planeCount ? d->data[plane] : nullptr;
}

const uchar *QVideoFrame::bits() const
{
   return d->data[0];
}

const uchar *QVideoFrame::bits(int plane) const
{
   return plane >= 0 && plane < d->planeCount ?  d->data[plane] : nullptr;
}

int QVideoFrame::mappedBytes() const
{
   return d->mappedBytes;
}

int QVideoFrame::planeCount() const
{
   return d->planeCount;
}

QVariant QVideoFrame::handle() const
{
   return d->buffer != nullptr ? d->buffer->handle() : QVariant();
}

qint64 QVideoFrame::startTime() const
{
   return d->startTime;
}

void QVideoFrame::setStartTime(qint64 time)
{
   d->startTime = time;
}

qint64 QVideoFrame::endTime() const
{
   return d->endTime;
}

void QVideoFrame::setEndTime(qint64 time)
{
   d->endTime = time;
}

QVariantMap QVideoFrame::availableMetaData() const
{
   return d->metadata;
}

QVariant QVideoFrame::metaData(const QString &key) const
{
   return d->metadata.value(key);
}

void QVideoFrame::setMetaData(const QString &key, const QVariant &value)
{
   if (value.isValid()) {
      d->metadata.insert(key, value);
   } else {
      d->metadata.remove(key);
   }
}

QVideoFrame::PixelFormat QVideoFrame::pixelFormatFromImageFormat(QImage::Format format)
{
   switch (format) {
      case QImage::Format_RGB32:
      case QImage::Format_RGBX8888:
         return Format_RGB32;

      case QImage::Format_ARGB32:
      case QImage::Format_RGBA8888:
         return Format_ARGB32;

      case QImage::Format_ARGB32_Premultiplied:
      case QImage::Format_RGBA8888_Premultiplied:
         return Format_ARGB32_Premultiplied;

      case QImage::Format_RGB16:
         return Format_RGB565;

      case QImage::Format_ARGB8565_Premultiplied:
         return Format_ARGB8565_Premultiplied;

      case QImage::Format_RGB555:
         return Format_RGB555;

      case QImage::Format_RGB888:
         return Format_RGB24;

      default:
         return Format_Invalid;
   }
}

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
      case Format_Jpeg:
      case Format_CameraRaw:
      case Format_AdobeDng:
         return QImage::Format_Invalid;

      case Format_User:
      default:
         return QImage::Format_Invalid;
   }
}

extern void qt_convert_BGRA32_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_BGR24_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_BGR565_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_BGR555_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_AYUV444_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_YUV444_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_YUV420P_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_YV12_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_UYVY_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_YUYV_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_NV12_to_ARGB32(const QVideoFrame &, uchar *);
extern void qt_convert_NV21_to_ARGB32(const QVideoFrame &, uchar *);
static VideoFrameConvertFunc qConvertFuncs[QVideoFrame::NPixelFormats] = {
   /* Format_Invalid */                nullptr, // Not needed
   /* Format_ARGB32 */                 nullptr, // Not needed
   /* Format_ARGB32_Premultiplied */   nullptr, // Not needed
   /* Format_RGB32 */                  nullptr, // Not needed
   /* Format_RGB24 */                  nullptr, // Not needed
   /* Format_RGB565 */                 nullptr, // Not needed
   /* Format_RGB555 */                 nullptr, // Not needed
   /* Format_ARGB8565_Premultiplied */ nullptr, // Not needed
   /* Format_BGRA32 */                 qt_convert_BGRA32_to_ARGB32,
   /* Format_BGRA32_Premultiplied */   qt_convert_BGRA32_to_ARGB32,
   /* Format_BGR32 */                  qt_convert_BGRA32_to_ARGB32,
   /* Format_BGR24 */                  qt_convert_BGR24_to_ARGB32,
   /* Format_BGR565 */                 qt_convert_BGR565_to_ARGB32,
   /* Format_BGR555 */                 qt_convert_BGR555_to_ARGB32,
   /* Format_BGRA5658_Premultiplied */ nullptr,
   /* Format_AYUV444 */                qt_convert_AYUV444_to_ARGB32,
   /* Format_AYUV444_Premultiplied */  nullptr,
   /* Format_YUV444 */                 qt_convert_YUV444_to_ARGB32,
   /* Format_YUV420P */                qt_convert_YUV420P_to_ARGB32,
   /* Format_YV12 */                   qt_convert_YV12_to_ARGB32,
   /* Format_UYVY */                   qt_convert_UYVY_to_ARGB32,
   /* Format_YUYV */                   qt_convert_YUYV_to_ARGB32,
   /* Format_NV12 */                   qt_convert_NV12_to_ARGB32,
   /* Format_NV21 */                   qt_convert_NV21_to_ARGB32,
   /* Format_IMC1 */                   nullptr,
   /* Format_IMC2 */                   nullptr,
   /* Format_IMC3 */                   nullptr,
   /* Format_IMC4 */                   nullptr,
   /* Format_Y8 */                     nullptr,
   /* Format_Y16 */                    nullptr,
   /* Format_Jpeg */                   nullptr, // Not needed
   /* Format_CameraRaw */              nullptr,
   /* Format_AdobeDng */               nullptr
};

static void qInitConvertFuncsAsm()
{
#ifdef QT_COMPILER_SUPPORTS_SSE2
   extern void qt_convert_BGRA32_to_ARGB32_sse2(const QVideoFrame &, uchar *);
   if (qCpuHasFeature(SSE2)) {
      qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32_sse2;
      qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32_sse2;
      qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32_sse2;
   }
#endif

#ifdef QT_COMPILER_SUPPORTS_SSSE3
   extern void qt_convert_BGRA32_to_ARGB32_ssse3(const QVideoFrame &, uchar *);
   if (qCpuHasFeature(SSSE3)) {
      qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32_ssse3;
      qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32_ssse3;
      qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32_ssse3;
   }
#endif

#ifdef QT_COMPILER_SUPPORTS_AVX2
   extern void qt_convert_BGRA32_to_ARGB32_avx2(const QVideoFrame &, uchar *);
   if (qCpuHasFeature(AVX2)) {
      qConvertFuncs[QVideoFrame::Format_BGRA32] = qt_convert_BGRA32_to_ARGB32_avx2;
      qConvertFuncs[QVideoFrame::Format_BGRA32_Premultiplied] = qt_convert_BGRA32_to_ARGB32_avx2;
      qConvertFuncs[QVideoFrame::Format_BGR32] = qt_convert_BGRA32_to_ARGB32_avx2;
   }
#endif
}

QImage qt_imageFromVideoFrame(const QVideoFrame &f)
{
   QVideoFrame &frame = const_cast<QVideoFrame &>(f);
   QImage result;

   if (!frame.isValid() || !frame.map(QAbstractVideoBuffer::ReadOnly)) {
      return result;
   }

   // Formats supported by QImage don't need conversion
   QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
   if (imageFormat != QImage::Format_Invalid) {
      result = QImage(frame.bits(), frame.width(), frame.height(), imageFormat).copy();
   }

   // Load from JPG
   else if (frame.pixelFormat() == QVideoFrame::Format_Jpeg) {
      result.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
   }

   // Need conversion
   else {
      static bool initAsmFuncsDone = false;
      if (! initAsmFuncsDone) {
         qInitConvertFuncsAsm();
         initAsmFuncsDone = true;
      }

      VideoFrameConvertFunc convert = qConvertFuncs[frame.pixelFormat()];
      if (! convert) {
         qWarning() << Q_FUNC_INFO << ": unsupported pixel format" << frame.pixelFormat();
      } else {
         result = QImage(frame.width(), frame.height(), QImage::Format_ARGB32);
         convert(frame, result.bits());
      }
   }

   frame.unmap();

   return result;
}

QDebug operator<<(QDebug dbg, QVideoFrame::PixelFormat pf)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   switch (pf) {
      case QVideoFrame::Format_Invalid:
         return dbg << "Format_Invalid";
      case QVideoFrame::Format_ARGB32:
         return dbg << "Format_ARGB32";
      case QVideoFrame::Format_ARGB32_Premultiplied:
         return dbg << "Format_ARGB32_Premultiplied";
      case QVideoFrame::Format_RGB32:
         return dbg << "Format_RGB32";
      case QVideoFrame::Format_RGB24:
         return dbg << "Format_RGB24";
      case QVideoFrame::Format_RGB565:
         return dbg << "Format_RGB565";
      case QVideoFrame::Format_RGB555:
         return dbg << "Format_RGB555";
      case QVideoFrame::Format_ARGB8565_Premultiplied:
         return dbg << "Format_ARGB8565_Premultiplied";
      case QVideoFrame::Format_BGRA32:
         return dbg << "Format_BGRA32";
      case QVideoFrame::Format_BGRA32_Premultiplied:
         return dbg << "Format_BGRA32_Premultiplied";
      case QVideoFrame::Format_BGR32:
         return dbg << "Format_BGR32";
      case QVideoFrame::Format_BGR24:
         return dbg << "Format_BGR24";
      case QVideoFrame::Format_BGR565:
         return dbg << "Format_BGR565";
      case QVideoFrame::Format_BGR555:
         return dbg << "Format_BGR555";
      case QVideoFrame::Format_BGRA5658_Premultiplied:
         return dbg << "Format_BGRA5658_Premultiplied";
      case QVideoFrame::Format_AYUV444:
         return dbg << "Format_AYUV444";
      case QVideoFrame::Format_AYUV444_Premultiplied:
         return dbg << "Format_AYUV444_Premultiplied";
      case QVideoFrame::Format_YUV444:
         return dbg << "Format_YUV444";
      case QVideoFrame::Format_YUV420P:
         return dbg << "Format_YUV420P";
      case QVideoFrame::Format_YV12:
         return dbg << "Format_YV12";
      case QVideoFrame::Format_UYVY:
         return dbg << "Format_UYVY";
      case QVideoFrame::Format_YUYV:
         return dbg << "Format_YUYV";
      case QVideoFrame::Format_NV12:
         return dbg << "Format_NV12";
      case QVideoFrame::Format_NV21:
         return dbg << "Format_NV21";
      case QVideoFrame::Format_IMC1:
         return dbg << "Format_IMC1";
      case QVideoFrame::Format_IMC2:
         return dbg << "Format_IMC2";
      case QVideoFrame::Format_IMC3:
         return dbg << "Format_IMC3";
      case QVideoFrame::Format_IMC4:
         return dbg << "Format_IMC4";
      case QVideoFrame::Format_Y8:
         return dbg << "Format_Y8";
      case QVideoFrame::Format_Y16:
         return dbg << "Format_Y16";
      case QVideoFrame::Format_Jpeg:
         return dbg << "Format_Jpeg";
      case QVideoFrame::Format_AdobeDng:
         return dbg << "Format_AdobeDng";
      case QVideoFrame::Format_CameraRaw:
         return dbg << "Format_CameraRaw";

      default:
         return dbg << QString("UserType(%1)" ).formatArg(int(pf));
   }
}

QDebug operator<<(QDebug dbg, QVideoFrame::FieldType f)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   switch (f) {
      case QVideoFrame::TopField:
         return dbg << "TopField";
      case QVideoFrame::BottomField:
         return dbg << "BottomField";
      case QVideoFrame::InterlacedFrame:
         return dbg << "InterlacedFrame";
      default:
         return dbg << "ProgressiveFrame";
   }
}
static QString qFormatTimeStamps(qint64 start, qint64 end)
{
   // Early out for invalid.
   if (start < 0) {
      return QString("[no timestamp]");
   }

   bool onlyOne = (start == end);

   // [hh:]mm:ss.ms
   const int s_millis = start % 1000000;
   start /= 1000000;
   const int s_seconds = start % 60;
   start /= 60;
   const int s_minutes = start % 60;
   start /= 60;

   if (onlyOne) {
      if (start > 0)
         return QString("@%1:%2:%3.%4")
            .formatArg(start, 1, 10, '0')
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0');
      else
         return QString("@%1:%2.%3")
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0');

   } else if (end == -1) {
      // Similar to start-start, except it means keep displaying it?
      if (start > 0)
         return QString("%1:%2:%3.%4 - forever")
            .formatArg(start, 1, 10, '0')
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0');
      else
         return QString("%1:%2.%3 - forever")
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0');
   } else {
      const int e_millis = end % 1000000;
      end /= 1000000;
      const int e_seconds = end % 60;
      end /= 60;
      const int e_minutes = end % 60;
      end /= 60;

      if (start > 0 || end > 0)
         return QString("%1:%2:%3.%4 - %5:%6:%7.%8")
            .formatArg(start, 1, 10, '0')
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0')
            .formatArg(end, 1, 10, '0')
            .formatArg(e_minutes, 2, 10, '0')
            .formatArg(e_seconds, 2, 10, '0')
            .formatArg(e_millis, 2, 10, '0');
      else
         return QString("%1:%2.%3 - %4:%5.%6")
            .formatArg(s_minutes, 2, 10, '0')
            .formatArg(s_seconds, 2, 10, '0')
            .formatArg(s_millis, 2, 10, '0')
            .formatArg(e_minutes, 2, 10, '0')
            .formatArg(e_seconds, 2, 10, '0')
            .formatArg(e_millis, 2, 10, '0');
   }
}

QDebug operator<<(QDebug dbg, const QVideoFrame &f)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   dbg << "QVideoFrame(" << f.size() << ", "
       << f.pixelFormat() << ", "
       << f.handleType() << ", "
       << f.mapMode() << ", "
       << qFormatTimeStamps(f.startTime(), f.endTime());

   if (f.availableMetaData().count()) {
      auto iter_end = f.availableMetaData().constEnd();

      for (auto iter = f.availableMetaData().constBegin(); iter != iter_end; f.availableMetaData() ) {
         dbg << ", metaData: " << iter.key() << iter.value().toString();
      }
   }

   dbg << ')';
   return dbg;
}

