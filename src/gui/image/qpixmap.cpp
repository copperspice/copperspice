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

#include <qpixmap.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmapcache.h>
#include <qdatetime.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qpaintengine.h>
#include <qscreen.h>
#include <qthread.h>
#include <qplatform_integration.h>
#include <qplatform_pixmap.h>

#include <qimagepixmapcleanuphooks_p.h>
#include <qapplication_p.h>
#include <qpixmap_raster_p.h>
#include <qhexstring_p.h>

static bool qt_pixmap_thread_test()
{
   if (! qApp) {
      qFatal("QPixmap::pixmap_thread() QApplication must be created before constructing a QPixmap");
      return false;
   }

   if (qApp->thread() != QThread::currentThread()) {
      bool fail = false;

      if (! QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::ThreadedPixmaps)) {
         printf("QPixmap::pixmap_thread() Platform does not support threaded pixmaps\n");
         fail = true;
      }

      if (fail) {
         qWarning("QPixmap::pixmap_thread() Unsafe to use pixmaps outside of a GUI thread");
         return false;
      }
   }

   return true;
}

void QPixmap::doInit(int w, int h, int type)
{
   if ((w > 0 && h > 0) || type == QPlatformPixmap::BitmapType) {
      data = QPlatformPixmap::create(w, h, (QPlatformPixmap::PixelType) type);
   } else {
      data = nullptr;
   }
}

QPixmap::QPixmap()
   : QPaintDevice()
{
   (void) qt_pixmap_thread_test();
   doInit(0, 0, QPlatformPixmap::PixmapType);
}

QPixmap::QPixmap(int w, int h)
   : QPaintDevice()
{
   if (!qt_pixmap_thread_test()) {
      doInit(0, 0, QPlatformPixmap::PixmapType);
   } else {
      doInit(w, h, QPlatformPixmap::PixmapType);
   }
}

QPixmap::QPixmap(const QSize &size)
   : QPaintDevice()
{
   if (!qt_pixmap_thread_test()) {
      doInit(0, 0, QPlatformPixmap::PixmapType);
   } else {
      doInit(size.width(), size.height(), QPlatformPixmap::PixmapType);
   }
}

// internal
QPixmap::QPixmap(const QSize &s, int type)
{
   if (! qt_pixmap_thread_test()) {
      doInit(0, 0, static_cast<QPlatformPixmap::PixelType>(type));
   } else {
      doInit(s.width(), s.height(), static_cast<QPlatformPixmap::PixelType>(type));
   }
}

// internal
QPixmap::QPixmap(QPlatformPixmap *d)
   : QPaintDevice(), data(d)
{
}

QPixmap::QPixmap(const QString &fileName, const QString &format, Qt::ImageConversionFlags flags)
   : QPaintDevice()
{
   doInit(0, 0, QPlatformPixmap::PixmapType);

   if (! qt_pixmap_thread_test()) {
      return;
   }

   load(fileName, format, flags);
}

QPixmap::QPixmap(const QPixmap &pixmap)
   : QPaintDevice()
{
   if (! qt_pixmap_thread_test()) {
      doInit(0, 0, QPlatformPixmap::PixmapType);
      return;
   }

   if (pixmap.paintingActive()) {                // make a deep copy
      pixmap.copy().swap(*this);
   } else {
      data = pixmap.data;
   }
}

#ifndef QT_NO_IMAGEFORMAT_XPM

QPixmap::QPixmap(const char *const xpm[])
   : QPaintDevice()
{
   doInit(0, 0, QPlatformPixmap::PixmapType);
   if (! xpm) {
      return;
   }

   QImage image(xpm);
   if (! image.isNull()) {
      if (data && data->pixelType() == QPlatformPixmap::BitmapType) {
         *this = QBitmap::fromImage(image);
      } else {
         *this = fromImage(image);
      }
   }
}
#endif

QPixmap::~QPixmap()
{
   Q_ASSERT(! data || data->ref.load() >= 1); // Catch if ref-counting changes again
}

// internal
int QPixmap::devType() const
{
   return QInternal::Pixmap;
}

QPixmap QPixmap::copy(const QRect &rect) const
{
   if (isNull()) {
      return QPixmap();
   }

   QRect r(0, 0, width(), height());
   if (!rect.isEmpty()) {
      r = r.intersected(rect);
   }

   QPlatformPixmap *d = data->createCompatiblePlatformPixmap();
   d->copy(data.data(), r);

   return QPixmap(d);
}

void QPixmap::scroll(int dx, int dy, const QRect &rect, QRegion *exposed)
{
   if (isNull() || (dx == 0 && dy == 0)) {
      return;
   }

   QRect dest = rect & this->rect();
   QRect src = dest.translated(-dx, -dy) & dest;

   if (src.isEmpty()) {
      if (exposed) {
         *exposed += dest;
      }
      return;
   }

   detach();

   if (! data->scroll(dx, dy, src)) {
      // Fallback
      QPixmap pix = *this;

      QPainter painter(&pix);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.drawPixmap(src.translated(dx, dy), *this, src);
      painter.end();

      *this = pix;
   }

   if (exposed) {
      *exposed += dest;
      *exposed -= src.translated(dx, dy);
   }
}

QPixmap &QPixmap::operator=(const QPixmap &pixmap)
{
   if (paintingActive()) {
      qWarning("QPixmap::operator=() Unable to assign a pixmap to another pixmap during painting");
      return *this;
   }

   if (pixmap.paintingActive()) {                // make a deep copy
      pixmap.copy().swap(*this);
   } else {
      data = pixmap.data;
   }

   return *this;
}

QPixmap::operator QVariant() const
{
   return QVariant(QVariant::Pixmap, this);
}

QImage QPixmap::toImage() const
{
   if (isNull()) {
      return QImage();
   }

   return data->toImage();
}

QTransform QPixmap::trueMatrix(const QTransform &m, int w, int h)
{
   return QImage::trueMatrix(m, w, h);
}

QMatrix QPixmap::trueMatrix(const QMatrix &m, int w, int h)
{
   return trueMatrix(QTransform(m), w, h).toAffine();
}

bool QPixmap::isQBitmap() const
{
   return data && data->type == QPlatformPixmap::BitmapType;
}

bool QPixmap::isNull() const
{
   return ! data || data->isNull();
}

int QPixmap::width() const
{
   return data ? data->width() : 0;
}

int QPixmap::height() const
{
   return data ? data->height() : 0;
}

QSize QPixmap::size() const
{
   return data ? QSize(data->width(), data->height()) : QSize(0, 0);
}

QRect QPixmap::rect() const
{
   return data ? QRect(0, 0, data->width(), data->height()) : QRect();
}

int QPixmap::depth() const
{
   return data ? data->depth() : 0;
}

void QPixmap::setMask(const QBitmap &mask)
{
   if (paintingActive()) {
      qWarning("QPixmap::setMask() Unable to set mask while pixmap is being painted");
      return;
   }

   if (!mask.isNull() && mask.size() != size()) {
      qWarning("QPixmap::setMask() Mask size differs from pixmap size");
      return;
   }

   if (isNull()) {
      return;
   }

   if (static_cast<const QPixmap &>(mask).data == data) {
      // trying to selfmask
      return;
   }

   detach();

   QImage image = data->toImage();

   if (mask.size().isEmpty()) {
      if (image.depth() != 1) { // hw: ????
         image = image.convertToFormat(QImage::Format_RGB32);
      }

   } else {
      const int w = image.width();
      const int h = image.height();

      switch (image.depth()) {
         case 1: {
            const QImage imageMask = mask.toImage().convertToFormat(image.format());
            for (int y = 0; y < h; ++y) {
               const uchar *mscan = imageMask.scanLine(y);
               uchar *tscan = image.scanLine(y);
               int bytesPerLine = image.bytesPerLine();
               for (int i = 0; i < bytesPerLine; ++i) {
                  tscan[i] &= mscan[i];
               }
            }
            break;
         }
         default: {
            const QImage imageMask = mask.toImage().convertToFormat(QImage::Format_MonoLSB);
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            for (int y = 0; y < h; ++y) {
               const uchar *mscan = imageMask.scanLine(y);
               QRgb *tscan = (QRgb *)image.scanLine(y);
               for (int x = 0; x < w; ++x) {
                  if (!(mscan[x >> 3] & (1 << (x & 7)))) {
                     tscan[x] = 0;
                  }
               }
            }
            break;
         }
      }
   }

   data->fromImage(image, Qt::AutoColor);
}

qreal QPixmap::devicePixelRatio() const
{
   if (! data) {
      return qreal(1.0);
   }

   return data->devicePixelRatio();
}

void QPixmap::setDevicePixelRatio(qreal scaleFactor)
{
   if (isNull()) {
      return;
   }

   if (scaleFactor == data->devicePixelRatio()) {
      return;
   }

   detach();
   data->setDevicePixelRatio(scaleFactor);
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
QBitmap QPixmap::createHeuristicMask(bool clipTight) const
{
   QBitmap m = QBitmap::fromImage(toImage().createHeuristicMask(clipTight));
   return m;
}
#endif

QBitmap QPixmap::createMaskFromColor(const QColor &maskColor, Qt::MaskMode mode) const
{
   QImage image = toImage().convertToFormat(QImage::Format_ARGB32);
   return QBitmap::fromImage(image.createMaskFromColor(maskColor.rgba(), mode));
}

bool QPixmap::load(const QString &fileName, const QString &format, Qt::ImageConversionFlags flags)
{
   if (! fileName.isEmpty()) {
      QFileInfo info(fileName);

      // if no extension is provided try to match the file against known plugin extensions

      if (info.completeSuffix().isEmpty() || info.exists()) {

         QString key = "qt_pixmap" + info.absoluteFilePath()
            + HexString<uint>(info.lastModified().toTime_t())
            + HexString<quint64>(info.size())
            + HexString<uint>(data ? data->pixelType() : QPlatformPixmap::PixmapType);

         if (QPixmapCache::find(key, this)) {
            return true;
         }

         data = QPlatformPixmap::create(0, 0, data ? data->pixelType() : QPlatformPixmap::PixmapType);

         if (data->fromFile(fileName, format, flags)) {
            QPixmapCache::insert(key, *this);
            return true;
         }
      }
   }

   if (isNull()) {
      if (! fileName.isEmpty()) {
         qWarning("QPixmap::load() Unable to load pixmap file %s", csPrintable(fileName));
      }

   } else {
      if (isQBitmap()) {
         *this = QBitmap();
      } else {
         data.reset();
      }
   }

   return false;
}

bool QPixmap::loadFromData(const uchar *imageData, uint len, const QString &format, Qt::ImageConversionFlags flags)
{
   if (len == 0 || imageData == nullptr) {
      data.reset();
      return false;
   }

   data = QPlatformPixmap::create(0, 0, QPlatformPixmap::PixmapType);

   if (data->fromData(imageData, len, format, flags)) {
      return true;
   }

   data.reset();

   return false;
}

bool QPixmap::save(const QString &fileName, const QString &format, int quality) const
{
   if (isNull()) {
      return false;   // nothing to save
   }

   QImageWriter writer(fileName, format);
   return doImageIO(&writer, quality);
}

bool QPixmap::save(QIODevice *device, const QString &format, int quality) const
{
   if (isNull()) {
      return false;   // nothing to save
   }

   QImageWriter writer(device, format);
   return doImageIO(&writer, quality);
}

// internal
bool QPixmap::doImageIO(QImageWriter *writer, int quality) const
{
   if (quality > 100  || quality < -1) {
      qWarning("QPixmap::save() Quality of pixmap is out of range [-1,100]");
   }

   if (quality >= 0) {
      writer->setQuality(qMin(quality, 100));
   }

   return writer->write(toImage());
}

void QPixmap::fill(const QPaintDevice *device, const QPoint &p)
{
   (void) device;
   (void) p;

   qWarning("QPixmap::fill() Method deprecated");
}

void QPixmap::fill(const QColor &color)
{
   if (isNull()) {
      return;
   }

   // Some people are probably already calling fill while a painter is active, so to not break
   // their programs, only print a warning and return when the fill operation could cause a crash.

   if (paintingActive() && (color.alpha() != 255) && !hasAlphaChannel()) {
      qWarning("QPixmap::fill() Unable to fill while pixmap is being painted");
      return;
   }

   if (data->ref.load() == 1) {
      // detach() will also remove this pixmap from caches, so
      // it has to be called even when ref == 1.
      detach();

   } else {
      // Don't bother to make a copy of the data object, since
      // it will be filled with new pixel data anyway.
      QPlatformPixmap *d = data->createCompatiblePlatformPixmap();
      d->resize(data->width(), data->height());
      data = d;
   }

   data->fill(color);
}

qint64 QPixmap::cacheKey() const
{
   if (isNull()) {
      return 0;
   }

   Q_ASSERT(data);
   return data->cacheKey();
}

QPixmap QPixmap::grabWidget(QObject *widget, const QRect &rectangle)
{
   QPixmap pixmap;
   qWarning("QPixmap::grabWidget() Method is deprecated, use QWidget::grab()");

   if (! widget) {
      return pixmap;
   }

   QWidget *obj = dynamic_cast<QWidget *>(widget);
   pixmap = obj->grab(rectangle);

   return pixmap;
}

QDataStream &operator<<(QDataStream &stream, const QPixmap &pixmap)
{
   return stream << pixmap.toImage();
}

QDataStream &operator>>(QDataStream &stream, QPixmap &pixmap)
{
   QImage image;
   stream >> image;

   if (image.isNull()) {
      pixmap = QPixmap();
   } else if (image.depth() == 1) {
      pixmap = QBitmap::fromImage(image);
   } else {
      pixmap = QPixmap::fromImage(image);
   }

   return stream;
}

// internal
bool QPixmap::isDetached() const
{
   return data && data->ref.load() == 1;
}

bool QPixmap::convertFromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
   detach();

   if (image.isNull() || ! data) {
      *this = QPixmap::fromImage(image, flags);
   } else {
      data->fromImage(image, flags);
   }

   return !isNull();
}

QPixmap QPixmap::scaled(const QSize &s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaled() Pixmap is empty");
      return QPixmap();
   }

   if (s.isEmpty()) {
      return QPixmap();
   }

   QSize newSize = size();
   newSize.scale(s, aspectMode);
   newSize.rwidth()  = qMax(newSize.width(), 1);
   newSize.rheight() = qMax(newSize.height(), 1);

   if (newSize == size()) {
      return *this;
   }

   QTransform wm = QTransform::fromScale((qreal)newSize.width() / width(),
         (qreal)newSize.height() / height());
   QPixmap pix = transformed(wm, mode);

   return pix;
}

QPixmap QPixmap::scaledToWidth(int w, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaleWidth() Pixmap is empty");
      return copy();
   }

   if (w <= 0) {
      return QPixmap();
   }

   qreal factor = (qreal) w / width();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

QPixmap QPixmap::scaledToHeight(int h, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaleHeight() Pixmap is empty");
      return copy();
   }

   if (h <= 0) {
      return QPixmap();
   }

   qreal factor = (qreal) h / height();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

QPixmap QPixmap::transformed(const QTransform &transform,
   Qt::TransformationMode mode) const
{
   if (isNull() || transform.type() <= QTransform::TxTranslate) {
      return *this;
   }

   return data->transformed(transform, mode);
}

QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
   return transformed(QTransform(matrix), mode);
}

bool QPixmap::hasAlpha() const
{
   return data && data->hasAlphaChannel();
}

bool QPixmap::hasAlphaChannel() const
{
   return data && data->hasAlphaChannel();
}

int QPixmap::metric(PaintDeviceMetric metric) const
{
   return data ? data->metric(metric) : 0;
}

QPaintEngine *QPixmap::paintEngine() const
{
   return data ? data->paintEngine() : nullptr;
}

QBitmap QPixmap::mask() const
{
   if (! data || ! hasAlphaChannel()) {
      return QBitmap();
   }

   const QImage img = toImage();
   bool shouldConvert = (img.format() != QImage::Format_ARGB32 && img.format() != QImage::Format_ARGB32_Premultiplied);

   const QImage image = (shouldConvert ? img.convertToFormat(QImage::Format_ARGB32_Premultiplied) : img);
   const int w = image.width();
   const int h = image.height();

   QImage mask(w, h, QImage::Format_MonoLSB);
   if (mask.isNull()) { // allocation failed
      return QBitmap();
   }

   mask.setColorCount(2);
   mask.setColor(0, QColor(Qt::color0).rgba());
   mask.setColor(1, QColor(Qt::color1).rgba());

   const int bpl = mask.bytesPerLine();

   for (int y = 0; y < h; ++y) {
      const QRgb *src = reinterpret_cast<const QRgb *>(image.scanLine(y));
      uchar *dest = mask.scanLine(y);
      memset(dest, 0, bpl);
      for (int x = 0; x < w; ++x) {
         if (qAlpha(*src) > 0) {
            dest[x >> 3] |= (1 << (x & 7));
         }
         ++src;
      }
   }

   return QBitmap::fromImage(mask);
}

int QPixmap::defaultDepth()
{
   return QGuiApplication::primaryScreen()->depth();
}

void QPixmap::detach()
{
   if (! data) {
      return;
   }

   QPlatformPixmap *pd = handle();
   QPlatformPixmap::ClassId id = pd->classId();

   if (id == QPlatformPixmap::RasterClass) {
      QRasterPlatformPixmap *rasterData = static_cast<QRasterPlatformPixmap *>(pd);
      rasterData->image.detach();
   }

   if (data->is_cached && data->ref.load() == 1) {
      QImagePixmapCleanupHooks::executePlatformPixmapModificationHooks(data.data());
   }

   if (data->ref.load() != 1) {
      *this = copy();
   }

   ++data->detach_no;
}

QPixmap QPixmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
   if (image.isNull()) {
      return QPixmap();
   }

   QScopedPointer<QPlatformPixmap> data(QGuiApplicationPrivate::platformIntegration()->
                  createPlatformPixmap(QPlatformPixmap::PixmapType));

   data->fromImage(image, flags);

   return QPixmap(data.take());
}

QPixmap QPixmap::fromImageInPlace(QImage &image, Qt::ImageConversionFlags flags)
{
   if (image.isNull()) {
      return QPixmap();
   }

   QScopedPointer<QPlatformPixmap> tmpImage(
         QGuiApplicationPrivate::platformIntegration()->createPlatformPixmap(QPlatformPixmap::PixmapType));

   tmpImage->fromImageInPlace(image, flags);

   return QPixmap(tmpImage.take());
}

QPixmap QPixmap::fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags)
{
   QScopedPointer<QPlatformPixmap> tmpImage(
         QGuiApplicationPrivate::platformIntegration()->createPlatformPixmap(QPlatformPixmap::PixmapType));

   tmpImage->fromImageReader(imageReader, flags);

   return QPixmap(tmpImage.take());
}

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
   qWarning("QPixmap::grabWindow() Method deprecated, use QScreen::grabWindow()");

   return QGuiApplication::primaryScreen()->grabWindow(window, x, y, w, h);
}

QPlatformPixmap *QPixmap::handle() const
{
   return data.data();
}

QPlatformPixmap::ClassId QPixmap::classId() const {
   return data->classId();
}

QDebug operator<<(QDebug dbg, const QPixmap &r)
{
   QDebugStateSaver saver(dbg);
   dbg.resetFormat();
   dbg.nospace();

   dbg << "QPixmap(";

   if (r.isNull()) {
      dbg << "is null";

   } else {
      dbg << r.size() << ", depth =" << r.depth()
         << ", devicePixelRatio =" << r.devicePixelRatio()
         << ", cacheKey =" << showbase << hex << r.cacheKey() << dec << noshowbase;
   }

   dbg << ')';

   return dbg;
}
