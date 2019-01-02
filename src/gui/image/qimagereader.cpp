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

#include <qimagereader.h>
#include <qbytearray.h>

#ifdef QIMAGEREADER_DEBUG
#include <qdebug.h>
#endif

#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qimageiohandler.h>
#include <qlist.h>
#include <qrect.h>

#include <qsize.h>
#include <qcolor.h>
#include <qvariant.h>

// factory loader
#include <qcoreapplication.h>
#include <qfactoryloader_p.h>
#include <QMutexLocker>

// image handlers
#include <qbmphandler_p.h>
#include <qppmhandler_p.h>
#include <qxbmhandler_p.h>
#include <qxpmhandler_p.h>

#ifndef QT_NO_IMAGEFORMAT_PNG
#include <qpnghandler_p.h>
#endif

#ifndef QT_NO_IMAGEFORMAT_JPEG
#include <qjpeghandler_p.h>
#endif

#ifndef QT_NO_IMAGEFORMAT_TIFF
#include <qtiffhandler_p.h>
#endif

#ifndef QT_NO_IMAGEFORMAT_ICO
#include <qicohandler_p.h>
#endif

#ifdef QT_BUILTIN_GIF_READER
#include <qgifhandler_p.h>
#endif

#include <algorithm>

Q_DECLARE_METATYPE(QList<QByteArray>)

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
   (QImageIOHandlerFactoryInterface_iid, QLatin1String("/imageformats")))

struct cs_BuiltInFormatStruct {
   using TestDevice = QImageIOHandler * (*)(QIODevice *);

   const char *extension;
   const char *mimeType;
   TestDevice checkFormat;
};

static const cs_BuiltInFormatStruct cs_BuiltInFormats[] = {

#ifndef QT_NO_IMAGEFORMAT_PNG
   {
      "png", "image/png",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QPngHandler::canRead(device))
         {
            return new QPngHandler;
         }

         return nullptr;
      }
   },
#endif

#ifndef QT_NO_IMAGEFORMAT_JPEG
   {
      "jpg",  "image/jpeg",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QJpegHandler::canRead(device))
         {
            return new QJpegHandler;
         }

         return nullptr;
      }
   },

   {
      "jpeg", "image/jpeg",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QJpegHandler::canRead(device))
         {
            return new QJpegHandler;
         }

         return nullptr;
      }
   },
#endif

#ifdef QT_BUILTIN_GIF_READER
   {
      "gif", "image/gif",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QGifHandler::canRead(device))
         {
            return new QGifHandler;
         }

         return nullptr;
      }
   },
#endif

   {
      "bmp", "image/bmp",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QBmpHandler::canRead(device))
         {
            return new QBmpHandler;
         }

         return nullptr;
      }
   },

#ifndef QT_NO_IMAGEFORMAT_PPM

   // emerald - support additional image formats

   // testFormat == "pbm"
   // testFormat == "pbmraw"

   // testFormat == "pgm"
   // testFormat == "pgmraw"

   // testFormat == "ppm"
   // testFormat == "ppmraw"

   {
      "ppm", "image/x-portable-pixmap",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QPpmHandler::canRead(device))
         {
            auto handler = new QPpmHandler;
            handler->setOption(QImageIOHandler::SubType, QString("ppm"));

            return handler;
         }

         return nullptr;
      }
   },

   {
      "pgm", "image/x-portable-graymap",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QPpmHandler::canRead(device))
         {
            auto handler = new QPpmHandler;
            handler->setOption(QImageIOHandler::SubType, QString("pgm"));

            return handler;
         }

         return nullptr;
      }
   },

   {
      "pbm", "image/x-portable-bitmap",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QPpmHandler::canRead(device))
         {
            auto handler = new QPpmHandler;
            handler->setOption(QImageIOHandler::SubType, QString("pbm"));

            return handler;
         }

         return nullptr;
      }
   },
#endif

#ifndef QT_NO_IMAGEFORMAT_XBM
   {
      "xbm", "image/x-xbitmap",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QXbmHandler::canRead(device))
         {
            auto handler = new QXbmHandler;
            handler->setOption(QImageIOHandler::SubType, QString("xbm"));

            return handler;
         }

         return nullptr;
      }
   },
#endif

#ifndef QT_NO_IMAGEFORMAT_XPM
   {
      "xpm", "image/x-xpixmap",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QXpmHandler::canRead(device))
         {
            return new QXpmHandler;
         }

         return nullptr;
      }
   },
#endif

#ifndef QT_NO_IMAGEFORMAT_ICO
   {
      "ico", "image/x-icon",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QIcoHandler::canRead(device))
         {
            return new QIcoHandler;
         }

         return nullptr;
      }
   },
#endif

#ifndef QT_NO_IMAGEFORMAT_TIFF
   {
      "tif", "image/tiff",
      [](QIODevice * device) -> QImageIOHandler *
      {
         if (QTiffHandler::canRead(device))
         {
            return new QTiffHandler;
         }

         return nullptr;
      }
   },

#endif

   { "", "", nullptr }
};

static QImageIOHandler *createReadHandlerHelper(QIODevice *device,
   const QByteArray &format, bool autoDetectImageFormat, bool ignoresFormatAndExtension)
{
   if (! autoDetectImageFormat && format.isEmpty()) {
      return nullptr;
   }

   QByteArray form = format.toLower();
   QImageIOHandler *handler = nullptr;

   QByteArray suffix;

   static QMutex mutex;
   QMutexLocker locker(&mutex);

   // check if we have plugins that support the image format
   QFactoryLoader *l = loader();
   const QMultiMap<int, QString> keyMap = l->keyMap();

   int suffixPluginIndex = -1;
   if (device && format.isEmpty() && autoDetectImageFormat && !ignoresFormatAndExtension) {

      if (QFile *file = qobject_cast<QFile *>(device)) {

         if (! (suffix = QFileInfo(file->fileName()).suffix().toLower().toLatin1()).isEmpty()) {
            const int index = keyMap.key(QString::fromLatin1(suffix), -1);

            if (index != -1) {
               suffixPluginIndex = index;
            }
         }
      }
   }

   QByteArray testFormat = ! form.isEmpty() ? form : suffix;

   if (ignoresFormatAndExtension) {
      testFormat = QByteArray();
   }

   if (suffixPluginIndex != -1) {
      // check if the plugin that claims support for this format can load
      // from this device with this format

      const qint64 pos = device ? device->pos() : 0;
      const int index = keyMap.key(QString::fromLatin1(suffix), -1);

      if (index != -1) {
         QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(index));

         if (plugin && plugin->capabilities(device, testFormat) & QImageIOPlugin::CanRead) {
            handler = plugin->create(device, testFormat);
         }
      }

      if (device && ! device->isSequential()) {
         device->seek(pos);
      }
   }

   if (! handler && ! testFormat.isEmpty() && !ignoresFormatAndExtension) {
      // check if any plugin supports the format
      // they are not allowed to read from the device yet
      const qint64 pos = device ? device->pos() : 0;

      if (autoDetectImageFormat) {
         for (int i = 0; i < keyMap.size(); ++i) {

            if (i != suffixPluginIndex) {
               QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(i));

               if (plugin && plugin->capabilities(device, testFormat) & QImageIOPlugin::CanRead) {
                  handler = plugin->create(device, testFormat);
                  break;
               }
            }
         }

      } else {
         const int testIndex = keyMap.key(testFormat, -1);

         if (testIndex != -1) {
            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(testIndex));

            if (plugin && plugin->capabilities(device, testFormat) & QImageIOPlugin::CanRead) {
               handler = plugin->create(device, testFormat);
            }
         }

         if (device && !device->isSequential()) {
            device->seek(pos);
         }
      }
   }

   if (! handler && (autoDetectImageFormat || ignoresFormatAndExtension)) {
      // check if any of our plugins recognize the file from its contents.
      const qint64 pos   = device ? device->pos() : 0;
      const int keyCount = keyMap.size();

      for (int i = 0; i < keyCount; ++i) {
         if (i != suffixPluginIndex) {
            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(i));

            if (plugin && plugin->capabilities(device, QByteArray()) & QImageIOPlugin::CanRead) {
               handler = plugin->create(device, testFormat);

               break;
            }
         }
      }

      if (device && ! device->isSequential()) {
         device->seek(pos);
      }
   }

   if (! handler && (autoDetectImageFormat || ignoresFormatAndExtension)) {
      // check if any of our built-in handlers recognize the file from its contents

      if (! suffix.isEmpty()) {
         // if reading from a file with a suffix, start testing our
         // built-in handler for that suffix first

         for (int i = 0; cs_BuiltInFormats[i].checkFormat != nullptr; ++i) {

            if (cs_BuiltInFormats[i].extension == suffix) {
               const qint64 pos = device->pos();

               handler = cs_BuiltInFormats[i].checkFormat(device);

               if (! device->isSequential()) {
                  device->seek(pos);
               }

               break;
            }
         }
      }

      if (handler == nullptr) {
         QByteArray subType;

         for (int i = 0; cs_BuiltInFormats[i].checkFormat != nullptr; ++i) {
            const qint64 pos = device->pos();

            handler = cs_BuiltInFormats[i].checkFormat(device);

            if (! device->isSequential()) {
               device->seek(pos);
            }

            if (handler != nullptr) {
               // found a matching handler, all done
               break;
            }
         }
      }
   }

   if (handler) {
      handler->setDevice(device);

      if (! form.isEmpty()) {
         handler->setFormat(form);
      }
   }

   return handler;
}

class QImageReaderPrivate
{
 public:
   QImageReaderPrivate(QImageReader *qq);
   ~QImageReaderPrivate();

   // device
   QByteArray format;
   bool autoDetectImageFormat;
   bool ignoresFormatAndExtension;
   QIODevice *device;
   bool deleteDevice;
   QImageIOHandler *handler;
   bool initHandler();

   // image options
   QRect clipRect;
   QSize scaledSize;
   QRect scaledClipRect;
   int quality;
   QMap<QString, QString> text;
   void getText();

   enum {
      UsePluginDefault,
      ApplyTransform,
      DoNotApplyTransform
   } autoTransform;

   // error
   QImageReader::ImageReaderError imageReaderError;
   QString errorString;

   QImageReader *q;
};

/*!
    \internal
*/
QImageReaderPrivate::QImageReaderPrivate(QImageReader *qq)
   : autoDetectImageFormat(true), ignoresFormatAndExtension(false)
{
   device = 0;
   deleteDevice = false;
   handler = 0;
   quality = -1;
   imageReaderError = QImageReader::UnknownError;
   autoTransform = UsePluginDefault;

   q = qq;
}

/*!
    \internal
*/
QImageReaderPrivate::~QImageReaderPrivate()
{
   if (deleteDevice) {
      delete device;
   }
   delete handler;
}

/*!
    \internal
*/
bool QImageReaderPrivate::initHandler()
{
   // check some preconditions
   if (!device || (!deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly))) {
      imageReaderError = QImageReader::DeviceError;
      errorString = QImageReader::tr("Invalid device");

      return false;
   }

   // probe the file extension
   if (deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly) && autoDetectImageFormat) {
      QList<QByteArray> extensions = QImageReader::supportedImageFormats();

      if (!format.isEmpty()) {
         // Try the most probable extension first
         int currentFormatIndex = extensions.indexOf(format.toLower());
         if (currentFormatIndex > 0) {
            extensions.swap(0, currentFormatIndex);
         }
      }

      int currentExtension = 0;

      QFile *file = static_cast<QFile *>(device);
      QString fileName = file->fileName();

      do {
         file->setFileName(fileName + QLatin1Char('.')
            + QString::fromLatin1(extensions.at(currentExtension++).constData()));
         file->open(QIODevice::ReadOnly);
      } while (!file->isOpen() && currentExtension < extensions.size());

      if (!device->isOpen()) {
         imageReaderError = QImageReader::FileNotFoundError;
         errorString = QImageReader::tr("File not found");
         file->setFileName(fileName); // restore the old file name

         return false;
      }
   }

   // assign a handler
   if (! handler &&
      (handler = createReadHandlerHelper(device, format, autoDetectImageFormat, ignoresFormatAndExtension)) == 0) {
      imageReaderError = QImageReader::UnsupportedFormatError;
      errorString = QImageReader::tr("Unsupported image format");
      return false;
   }

   return true;
}

/*!
    \internal
*/
void QImageReaderPrivate::getText()
{
   if (!text.isEmpty() || (!handler && !initHandler()) || !handler->supportsOption(QImageIOHandler::Description)) {
      return;
   }

   for (const QString &pair : handler->option(QImageIOHandler::Description).toString().split(
         QLatin1String("\n\n"))) {
      int index = pair.indexOf(QLatin1Char(':'));

      if (index >= 0 && pair.indexOf(QLatin1Char(' ')) < index) {
         text.insert(QLatin1String("Description"), pair.simplified());
      } else {
         QString key = pair.left(index);
         text.insert(key, pair.mid(index + 2).simplified());
      }
   }
}

QImageReader::QImageReader()
   : d(new QImageReaderPrivate(this))
{
}

QImageReader::QImageReader(QIODevice *device, const QByteArray &format)
   : d(new QImageReaderPrivate(this))
{
   d->device = device;
   d->format = format;
}

QImageReader::QImageReader(const QString &fileName, const QByteArray &format)
   : d(new QImageReaderPrivate(this))
{
   QFile *file = new QFile(fileName);
   d->device = file;
   d->deleteDevice = true;
   d->format = format;
}

QImageReader::~QImageReader()
{
   delete d;
}

void QImageReader::setFormat(const QByteArray &format)
{
   d->format = format;
}

QByteArray QImageReader::format() const
{
   if (d->format.isEmpty()) {
      if (! d->initHandler()) {
         return QByteArray();
      }
      return d->handler->canRead() ? d->handler->format() : QByteArray();
   }

   return d->format;
}

void QImageReader::setAutoDetectImageFormat(bool enabled)
{
   d->autoDetectImageFormat = enabled;
}


bool QImageReader::autoDetectImageFormat() const
{
   return d->autoDetectImageFormat;
}


void QImageReader::setDecideFormatFromContent(bool ignored)
{
   d->ignoresFormatAndExtension = ignored;
}

bool QImageReader::decideFormatFromContent() const
{
   return d->ignoresFormatAndExtension;
}

void QImageReader::setDevice(QIODevice *device)
{
   if (d->device && d->deleteDevice) {
      delete d->device;
   }
   d->device = device;
   d->deleteDevice = false;
   delete d->handler;
   d->handler = 0;
   d->text.clear();
}

QIODevice *QImageReader::device() const
{
   return d->device;
}

void QImageReader::setFileName(const QString &fileName)
{
   setDevice(new QFile(fileName));
   d->deleteDevice = true;
}

QString QImageReader::fileName() const
{
   QFile *file = qobject_cast<QFile *>(d->device);
   return file ? file->fileName() : QString();
}

void QImageReader::setQuality(int quality)
{
   d->quality = quality;
}

int QImageReader::quality() const
{
   return d->quality;
}

QSize QImageReader::size() const
{
   if (!d->initHandler()) {
      return QSize();
   }

   if (d->handler->supportsOption(QImageIOHandler::Size)) {
      return d->handler->option(QImageIOHandler::Size).toSize();
   }

   return QSize();
}

QImage::Format QImageReader::imageFormat() const
{
   if (!d->initHandler()) {
      return QImage::Format_Invalid;
   }

   if (d->handler->supportsOption(QImageIOHandler::ImageFormat)) {
      return (QImage::Format)d->handler->option(QImageIOHandler::ImageFormat).toInt();
   }

   return QImage::Format_Invalid;
}

QStringList QImageReader::textKeys() const
{
   d->getText();
   return d->text.keys();
}

QString QImageReader::text(const QString &key) const
{
   d->getText();
   return d->text.value(key);
}


void QImageReader::setClipRect(const QRect &rect)
{
   d->clipRect = rect;
}

QRect QImageReader::clipRect() const
{
   return d->clipRect;
}

void QImageReader::setScaledSize(const QSize &size)
{
   d->scaledSize = size;
}


QSize QImageReader::scaledSize() const
{
   return d->scaledSize;
}

void QImageReader::setScaledClipRect(const QRect &rect)
{
   d->scaledClipRect = rect;
}

QRect QImageReader::scaledClipRect() const
{
   return d->scaledClipRect;
}

void QImageReader::setBackgroundColor(const QColor &color)
{
   if (!d->initHandler()) {
      return;
   }

   if (d->handler->supportsOption(QImageIOHandler::BackgroundColor)) {
      d->handler->setOption(QImageIOHandler::BackgroundColor, color);
   }
}

QColor QImageReader::backgroundColor() const
{
   if (!d->initHandler()) {
      return QColor();
   }

   if (d->handler->supportsOption(QImageIOHandler::BackgroundColor)) {
      return qvariant_cast<QColor>(d->handler->option(QImageIOHandler::BackgroundColor));
   }
   return QColor();
}


bool QImageReader::supportsAnimation() const
{
   if (! d->initHandler()) {
      return false;
   }

   if (d->handler->supportsOption(QImageIOHandler::Animation)) {
      return d->handler->option(QImageIOHandler::Animation).toBool();
   }

   return false;
}

QByteArray QImageReader::subType() const
{
   if (!d->initHandler()) {
      return QByteArray();
   }

   if (d->handler->supportsOption(QImageIOHandler::SubType)) {
      return d->handler->option(QImageIOHandler::SubType).toByteArray();
   }
   return QByteArray();
}

QList<QByteArray> QImageReader::supportedSubTypes() const
{
   if (!d->initHandler()) {
      return QList<QByteArray>();
   }

   if (!d->handler->supportsOption(QImageIOHandler::SupportedSubTypes)) {
      return d->handler->option(QImageIOHandler::SupportedSubTypes).value< QList<QByteArray>>();
   }
   return QList<QByteArray>();
}

QImageIOHandler::Transformations QImageReader::transformation() const
{
   int option = QImageIOHandler::TransformationNone;
   if (d->initHandler() && d->handler->supportsOption(QImageIOHandler::ImageTransformation)) {
      option = d->handler->option(QImageIOHandler::ImageTransformation).toInt();
   }
   return QImageIOHandler::Transformations(option);
}

void QImageReader::setAutoTransform(bool enabled)
{
   d->autoTransform = enabled ? QImageReaderPrivate::ApplyTransform
      : QImageReaderPrivate::DoNotApplyTransform;
}

bool QImageReader::autoTransform() const
{
   switch (d->autoTransform) {
      case QImageReaderPrivate::ApplyTransform:
         return true;

      case QImageReaderPrivate::DoNotApplyTransform:
         return false;

      case QImageReaderPrivate::UsePluginDefault:
         if (d->initHandler()) {
            return d->handler->supportsOption(QImageIOHandler::TransformedByDefault);
         }
         // no break

      default:
         break;
   }

   return false;
}

void QImageReader::setGamma(float gamma)
{
   if (d->initHandler() && d->handler->supportsOption(QImageIOHandler::Gamma)) {
      d->handler->setOption(QImageIOHandler::Gamma, gamma);
   }
}

float QImageReader::gamma() const
{
   if (d->initHandler() && d->handler->supportsOption(QImageIOHandler::Gamma)) {
      return d->handler->option(QImageIOHandler::Gamma).toFloat();
   }
   return 0.0;
}

bool QImageReader::canRead() const
{
   if (!d->initHandler()) {
      return false;
   }

   return d->handler->canRead();
}

QImage QImageReader::read()
{
   // Because failed image reading might have side effects, we explicitly
   // return a null image instead of the image we've just created.
   QImage image;
   return read(&image) ? image : QImage();
}

extern void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient);

bool QImageReader::read(QImage *image)
{
   if (! image) {
      qWarning("QImageReader::read: cannot read into null pointer");
      return false;
   }

   if (!d->handler && !d->initHandler()) {
      return false;
   }

   // set the handler specific options.
   if (d->handler->supportsOption(QImageIOHandler::ScaledSize) && d->scaledSize.isValid()) {
      if ((d->handler->supportsOption(QImageIOHandler::ClipRect) && !d->clipRect.isNull())
         || d->clipRect.isNull()) {
         // Only enable the ScaledSize option if there is no clip rect, or
         // if the handler also supports ClipRect.
         d->handler->setOption(QImageIOHandler::ScaledSize, d->scaledSize);
      }
   }

   if (d->handler->supportsOption(QImageIOHandler::ClipRect) && !d->clipRect.isNull()) {
      d->handler->setOption(QImageIOHandler::ClipRect, d->clipRect);
   }

   if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !d->scaledClipRect.isNull()) {
      d->handler->setOption(QImageIOHandler::ScaledClipRect, d->scaledClipRect);
   }
   if (d->handler->supportsOption(QImageIOHandler::Quality)) {
      d->handler->setOption(QImageIOHandler::Quality, d->quality);
   }

   // read the image
   if (!d->handler->read(image)) {
      d->imageReaderError = InvalidDataError;
      d->errorString = QImageReader::tr("Unable to read image data");
      return false;
   }

   // provide default implementations for any unsupported image options
   if (d->handler->supportsOption(QImageIOHandler::ClipRect) && !d->clipRect.isNull()) {
      if (d->handler->supportsOption(QImageIOHandler::ScaledSize) && d->scaledSize.isValid()) {

         if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !d->scaledClipRect.isNull()) {
            // all features are supported by the handler; nothing to do.
         } else {
            // the image is already scaled, so apply scaled clipping.
            if (!d->scaledClipRect.isNull()) {
               *image = image->copy(d->scaledClipRect);
            }
         }

      } else {
         if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !d->scaledClipRect.isNull()) {
            // supports scaled clipping but not scaling, most
            // likely a broken handler.
         } else {
            if (d->scaledSize.isValid()) {
               *image = image->scaled(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            if (d->scaledClipRect.isValid()) {
               *image = image->copy(d->scaledClipRect);
            }
         }
      }

   } else {
      if (d->handler->supportsOption(QImageIOHandler::ScaledSize) && d->scaledSize.isValid() && d->clipRect.isNull()) {
         // in this case, there's nothing we can do. if the
         // plugin supports scaled size but not ClipRect, then
         // we have to ignore ClipRect."

         if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !d->scaledClipRect.isNull()) {
            // nothing to do (ClipRect is ignored!)
         } else {
            // provide all workarounds.
            if (d->scaledClipRect.isValid()) {
               *image = image->copy(d->scaledClipRect);
            }
         }

      } else {

         if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect) && !d->scaledClipRect.isNull()) {
            // this makes no sense; a handler that supports
            // ScaledClipRect but not ScaledSize is broken, and we
            // can't work around it.

         } else {
            // provide all workarounds.
            if (d->clipRect.isValid()) {
               *image = image->copy(d->clipRect);
            }

            if (d->scaledSize.isValid()) {
               *image = image->scaled(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }

            if (d->scaledClipRect.isValid()) {
               *image = image->copy(d->scaledClipRect);
            }

         }
      }
   }
   static bool disable2xImageLoading = ! qgetenv("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING").isEmpty();

   if (!disable2xImageLoading && QFileInfo(fileName()).baseName().endsWith("@2x")) {
      image->setDevicePixelRatio(2.0);
   }

   if (autoTransform()) {
      qt_imageTransform(*image, transformation());
   }

   return true;
}


bool QImageReader::jumpToNextImage()
{
   if (!d->initHandler()) {
      return false;
   }
   return d->handler->jumpToNextImage();
}


bool QImageReader::jumpToImage(int imageNumber)
{
   if (!d->initHandler()) {
      return false;
   }
   return d->handler->jumpToImage(imageNumber);
}

int QImageReader::loopCount() const
{
   if (!d->initHandler()) {
      return -1;
   }
   return d->handler->loopCount();
}


int QImageReader::imageCount() const
{
   if (!d->initHandler()) {
      return -1;
   }
   return d->handler->imageCount();
}

int QImageReader::nextImageDelay() const
{
   if (!d->initHandler()) {
      return -1;
   }
   return d->handler->nextImageDelay();
}

int QImageReader::currentImageNumber() const
{
   if (!d->initHandler()) {
      return -1;
   }
   return d->handler->currentImageNumber();
}

QRect QImageReader::currentImageRect() const
{
   if (!d->initHandler()) {
      return QRect();
   }
   return d->handler->currentImageRect();
}

QImageReader::ImageReaderError QImageReader::error() const
{
   return d->imageReaderError;
}

QString QImageReader::errorString() const
{
   if (d->errorString.isEmpty()) {
      return QImageReader::tr("Unknown error");
   }
   return d->errorString;
}

bool QImageReader::supportsOption(QImageIOHandler::ImageOption option) const
{
   if (!d->initHandler()) {
      return false;
   }
   return d->handler->supportsOption(option);
}

QByteArray QImageReader::imageFormat(const QString &fileName)
{
   QFile file(fileName);
   if (!file.open(QFile::ReadOnly)) {
      return QByteArray();
   }

   return imageFormat(&file);
}

QByteArray QImageReader::imageFormat(QIODevice *device)
{
   QByteArray format;
   QImageIOHandler *handler = createReadHandlerHelper(device, format, /* autoDetectImageFormat = */ true, false);

   if (handler) {
      if (handler->canRead()) {
         format = handler->format();
      }
      delete handler;
   }
   return format;
}

void supportedImageHandlerFormats(QFactoryLoader *loader, QImageIOPlugin::Capability cap, QList<QByteArray> *result);
void supportedImageHandlerMimeTypes(QFactoryLoader *loader, QImageIOPlugin::Capability cap, QList<QByteArray> *result);

QList<QByteArray> QImageReader::supportedImageFormats()
{
   QList<QByteArray> formats;

   for (int i = 0; cs_BuiltInFormats[i].checkFormat != nullptr; ++i) {
      formats << cs_BuiltInFormats[i].extension;
   }

   supportedImageHandlerFormats(loader(), QImageIOPlugin::CanRead, &formats);

   std::sort(formats.begin(), formats.end());
   formats.erase(std::unique(formats.begin(), formats.end()), formats.end());

   return formats;
}

QList<QByteArray> QImageReader::supportedMimeTypes()
{
   QList<QByteArray> mimeTypes;

   for (int i = 0; cs_BuiltInFormats[i].checkFormat != nullptr; ++i) {
      mimeTypes << cs_BuiltInFormats[i].mimeType;
   }

   supportedImageHandlerMimeTypes(loader(), QImageIOPlugin::CanRead, &mimeTypes);

   std::sort(mimeTypes.begin(), mimeTypes.end());
   mimeTypes.erase(std::unique(mimeTypes.begin(), mimeTypes.end()), mimeTypes.end());
   return mimeTypes;
}
