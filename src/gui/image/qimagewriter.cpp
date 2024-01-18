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

#include <qimagewriter.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qimageiohandler.h>
#include <qjsonarray.h>
#include <qset.h>
#include <qvariant.h>

// factory loader
#include <qcoreapplication.h>
#include <qfactoryloader_p.h>

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

#ifdef QT_BUILTIN_GIF_READER
#include <qgifhandler_p.h>
#endif

#ifndef QT_NO_IMAGEFORMAT_TIFF
#include <qtiffhandler_p.h>
#endif

#ifndef QT_NO_IMAGEFORMAT_ICO
#include <qicohandler_p.h>
#endif

#include <algorithm>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QImageIOHandlerInterface_ID, "/imageformats");
   return &retval;
}

static QImageIOHandler *createWriteHandlerHelper(QIODevice *device, const QString &format)
{
   QString form = format.toLower();
   QImageIOHandler *handler = nullptr;

  QString suffix;

   // check if any plugins can write the image
   QFactoryLoader *factoryObj = loader();

   // what keys are available
   const QSet<QString> keySet = factoryObj->keySet();

   bool found = false;

   if (device && format.isEmpty()) {
      // if there's no format, see if device is a file, and if so, find
      // the file suffix and find support for that format among our plugins.
      // this allows plugins to override our built-in handlers.

      if (QFile *file = qobject_cast<QFile *>(device)) {
         suffix = QFileInfo(file->fileName()).suffix().toLower();

         if (! suffix.isEmpty() && keySet.contains(suffix)) {
            found = true;
         }
      }
   }

   QString testFormat = ! form.isEmpty() ? form : suffix;

   if (found) {
      // when format is missing, check if we can find a plugin for the suffix
      QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(factoryObj->instance(suffix));

      if (plugin && (plugin->capabilities(device, suffix) & QImageIOPlugin::CanWrite)) {
         handler = plugin->create(device, suffix);
      }
   }

   // check if any built-in handlers can write the image
   if (! handler && ! testFormat.isEmpty()) {
      if (false) {

#ifndef QT_NO_IMAGEFORMAT_PNG
      } else if (testFormat == "png") {
         handler = new QPngHandler;
#endif

#ifndef QT_NO_IMAGEFORMAT_JPEG
      } else if (testFormat == "jpg" || testFormat == "jpeg") {
         handler = new QJpegHandler;
#endif

#ifdef QT_BUILTIN_GIF_READER
      } else if (testFormat == "gif") {
         handler = new QGifHandler;
#endif

#ifndef QT_NO_IMAGEFORMAT_BMP
      } else if (testFormat == "bmp") {
         handler = new QBmpHandler;
      } else if (testFormat == "dib") {
         handler = new QBmpHandler(QBmpHandler::DibFormat);
#endif

#ifndef QT_NO_IMAGEFORMAT_XPM
      } else if (testFormat == "xpm") {
         handler = new QXpmHandler;
#endif

#ifndef QT_NO_IMAGEFORMAT_XBM
      } else if (testFormat == "xbm") {
         handler = new QXbmHandler;
         handler->setOption(QImageIOHandler::SubType, testFormat);
#endif

#ifndef QT_NO_IMAGEFORMAT_PPM
      } else if (testFormat == "pbm" || testFormat == "pbmraw" || testFormat == "pgm"
         || testFormat == "pgmraw" || testFormat == "ppm" || testFormat == "ppmraw") {
         handler = new QPpmHandler;
         handler->setOption(QImageIOHandler::SubType, testFormat);
#endif

#ifndef QT_NO_IMAGEFORMAT_TIFF
      } else if (testFormat == "tif" || testFormat == "tiff") {
         handler = new QTiffHandler;
#endif

#ifndef QT_NO_IMAGEFORMAT_ICO
      } else if (testFormat == "ico") {
         handler = new QIcoHandler;
#endif
      }
   }

   if (! testFormat.isEmpty()) {

      for (auto item : keySet) {
         QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(factoryObj->instance(item));

         if (plugin && (plugin->capabilities(device, testFormat) & QImageIOPlugin::CanWrite)) {
            delete handler;
            handler = plugin->create(device, testFormat);
            break;
         }
      }
   }

   if (! handler) {
      return nullptr;
   }

   handler->setDevice(device);
   if (! testFormat.isEmpty()) {
      handler->setFormat(testFormat);
   }

   return handler;
}

class QImageWriterPrivate
{
 public:
   QImageWriterPrivate(QImageWriter *qq);

   bool canWriteHelper();
   // device
   QString format;
   QIODevice *device;
   bool deleteDevice;
   QImageIOHandler *handler;

   // image options
   int quality;
   int compression;
   float gamma;
   QString description;
   QString text;

   QByteArray subType;
   bool optimizedWrite;
   bool progressiveScanWrite;
   QImageIOHandler::Transformations transformation;

   // error
   QImageWriter::ImageWriterError imageWriterError;
   QString errorString;

   QImageWriter *q;
};

/*!
    \internal
*/
QImageWriterPrivate::QImageWriterPrivate(QImageWriter *qq)
{
   device       = nullptr;
   deleteDevice = false;
   handler      = nullptr;
   quality      = -1;
   compression  = 0;
   gamma        = 0.0;

   optimizedWrite = false;
   progressiveScanWrite = false;
   imageWriterError = QImageWriter::UnknownError;
   errorString = QImageWriter::tr("Unknown error");
   transformation = QImageIOHandler::TransformationNone;

   q = qq;
}

bool QImageWriterPrivate::canWriteHelper()
{
   if (!device) {
      imageWriterError = QImageWriter::DeviceError;
      errorString = QImageWriter::tr("Device is not set");
      return false;
   }

   if (!device->isOpen()) {
      device->open(QIODevice::WriteOnly);
   }

   if (!device->isWritable()) {
      imageWriterError = QImageWriter::DeviceError;
      errorString = QImageWriter::tr("Device not writable");
      return false;
   }

   if (!handler && (handler = createWriteHandlerHelper(device, format)) == nullptr) {
      imageWriterError = QImageWriter::UnsupportedFormatError;
      errorString = QImageWriter::tr("Unsupported image format");
      return false;
   }

   return true;
}

/*!
    Constructs an empty QImageWriter object. Before writing, you must
    call setFormat() to set an image format, then setDevice() or
    setFileName().
*/
QImageWriter::QImageWriter()
   : d(new QImageWriterPrivate(this))
{
}

/*!
    Constructs a QImageWriter object using the device \a device and
    image format \a format.
*/
QImageWriter::QImageWriter(QIODevice *device, const QString &format)
   : d(new QImageWriterPrivate(this))
{
   d->device = device;
   d->format = format;
}

/*!
    Constructs a QImageWriter objects that will write to a file with
    the name \a fileName, using the image format \a format. If \a
    format is not provided, QImageWriter will detect the image format
    by inspecting the extension of \a fileName.
*/
QImageWriter::QImageWriter(const QString &fileName, const QString &format)
   : d(new QImageWriterPrivate(this))
{
   QFile *file = new QFile(fileName);
   d->device = file;
   d->deleteDevice = true;
   d->format = format;
}

/*!
    Destructs the QImageWriter object.
*/
QImageWriter::~QImageWriter()
{
   if (d->deleteDevice) {
      delete d->device;
   }
   delete d->handler;
   delete d;
}

void QImageWriter::setFormat(const QString &format)
{
   d->format = format;
}

QString QImageWriter::format() const
{
   return d->format;
}

void QImageWriter::setDevice(QIODevice *device)
{
   if (d->device && d->deleteDevice) {
      delete d->device;
   }

   d->device = device;
   d->deleteDevice = false;
   delete d->handler;
   d->handler = nullptr;
}

/*!
    Returns the device currently assigned to QImageWriter, or 0 if no
    device has been assigned.
*/
QIODevice *QImageWriter::device() const
{
   return d->device;
}

/*!
    Sets the file name of QImageWriter to \a fileName. Internally,
    QImageWriter will create a QFile and open it in \l
    QIODevice::WriteOnly mode, and use this file when writing images.

    \sa fileName(), setDevice()
*/
void QImageWriter::setFileName(const QString &fileName)
{
   setDevice(new QFile(fileName));
   d->deleteDevice = true;
}


QString QImageWriter::fileName() const
{
   QFile *file = qobject_cast<QFile *>(d->device);
   return file ? file->fileName() : QString();
}


void QImageWriter::setQuality(int quality)
{
   d->quality = quality;
}
int QImageWriter::quality() const
{
   return d->quality;
}


void QImageWriter::setCompression(int compression)
{
   d->compression = compression;
}

/*!
    Returns the compression of the image.

    \sa setCompression()
*/
int QImageWriter::compression() const
{
   return d->compression;
}

/*!
    This is an image format specific function that sets the gamma
    level of the image to \a gamma. For image formats that do not
    support setting the gamma level, this value is ignored.

    The value range of \a gamma depends on the image format. For
    example, the "png" format supports a gamma range from 0.0 to 1.0.

    \sa quality()
*/
void QImageWriter::setGamma(float gamma)
{
   d->gamma = gamma;
}

/*!
    Returns the gamma level of the image.

    \sa setGamma()
*/
float QImageWriter::gamma() const
{
   return d->gamma;
}

void QImageWriter::setSubType(const QByteArray &type)
{
   d->subType = type;
}

QByteArray QImageWriter::subType() const
{
   return d->subType;
}

QList<QByteArray> QImageWriter::supportedSubTypes() const
{
   if (! supportsOption(QImageIOHandler::SupportedSubTypes)) {
      return QList<QByteArray>();
   }

   return d->handler->option(QImageIOHandler::SupportedSubTypes).value< QList<QByteArray>>();
}

void QImageWriter::setOptimizedWrite(bool optimize)
{
   d->optimizedWrite = optimize;
}

bool QImageWriter::optimizedWrite() const
{
   return d->optimizedWrite;
}

void QImageWriter::setProgressiveScanWrite(bool progressive)
{
   d->progressiveScanWrite = progressive;
}

bool QImageWriter::progressiveScanWrite() const
{
   return d->progressiveScanWrite;
}

void QImageWriter::setTransformation(QImageIOHandler::Transformations transform)
{
   d->transformation = transform;
}

QImageIOHandler::Transformations QImageWriter::transformation() const
{
   return d->transformation;
}

void QImageWriter::setText(const QString &key, const QString &text)
{
   if (!d->description.isEmpty()) {
      d->description += QLatin1String("\n\n");
   }

   d->description += key.simplified() + QLatin1String(": ") + text.simplified();
}

bool QImageWriter::canWrite() const
{
   if (QFile *file = qobject_cast<QFile *>(d->device)) {
      const bool remove = !file->isOpen() && !file->exists();
      const bool result = d->canWriteHelper();
      if (!result && remove) {
         file->remove();
      }
      return result;
   }

   return d->canWriteHelper();
}

extern void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient);


bool QImageWriter::write(const QImage &image)
{
   if (! canWrite()) {
      return false;
   }

   QImage img = image;

   if (d->handler->supportsOption(QImageIOHandler::Quality)) {
      d->handler->setOption(QImageIOHandler::Quality, d->quality);
   }

   if (d->handler->supportsOption(QImageIOHandler::CompressionRatio)) {
      d->handler->setOption(QImageIOHandler::CompressionRatio, d->compression);
   }

   if (d->handler->supportsOption(QImageIOHandler::Gamma)) {
      d->handler->setOption(QImageIOHandler::Gamma, d->gamma);
   }

   if (!d->description.isEmpty() && d->handler->supportsOption(QImageIOHandler::Description)) {
      d->handler->setOption(QImageIOHandler::Description, d->description);
   }

   if (!d->subType.isEmpty() && d->handler->supportsOption(QImageIOHandler::SubType)) {
      d->handler->setOption(QImageIOHandler::SubType, d->subType);
   }

   if (d->handler->supportsOption(QImageIOHandler::OptimizedWrite)) {
      d->handler->setOption(QImageIOHandler::OptimizedWrite, d->optimizedWrite);
   }

   if (d->handler->supportsOption(QImageIOHandler::ProgressiveScanWrite)) {
      d->handler->setOption(QImageIOHandler::ProgressiveScanWrite, d->progressiveScanWrite);
   }

   if (d->handler->supportsOption(QImageIOHandler::ImageTransformation)) {
      d->handler->setOption(QImageIOHandler::ImageTransformation, int(d->transformation));
   } else {
      qt_imageTransform(img, d->transformation);
   }

   if (!d->handler->write(img)) {
      return false;
   }

   if (QFile *file = qobject_cast<QFile *>(d->device)) {
      file->flush();
   }

   return true;
}

QImageWriter::ImageWriterError QImageWriter::error() const
{
   return d->imageWriterError;
}

QString QImageWriter::errorString() const
{
   return d->errorString;
}

bool QImageWriter::supportsOption(QImageIOHandler::ImageOption option) const
{
   if (! d->handler && (d->handler = createWriteHandlerHelper(d->device, d->format)) == nullptr) {
      d->imageWriterError = QImageWriter::UnsupportedFormatError;

      d->errorString = QImageWriter::tr("Unsupported image format");
      return false;
   }

   return d->handler->supportsOption(option);
}

void supportedImageHandlerFormats(QFactoryLoader *factoryObj, QImageIOPlugin::Capability cap, QList<QString> *result)
{
   auto keySet = factoryObj->keySet();

   QImageIOPlugin *plugin = nullptr;

   for (auto item : keySet) {
      plugin = qobject_cast<QImageIOPlugin *>(factoryObj->instance(item));

       if (plugin && (plugin->capabilities(nullptr, item) & cap) != 0) {
         result->append(item);
      }
   }
}

void supportedImageHandlerMimeTypes(QFactoryLoader *factoryObj, QImageIOPlugin::Capability cap, QList<QString> *result)
{
   auto keySet = factoryObj->keySet();

   for (auto item : keySet) {
      auto librarySet = factoryObj->librarySet(item);

      for (auto library : librarySet) {
         const QMetaObject *metaobj = library->m_metaObject;

         int index = metaobj->indexOfClassInfo("MimeTypes");

         if (index != -1) {
            // only one, may need to allow for multiple mime types
            QString mimeType = metaobj->classInfo(index).value();

            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(factoryObj->instance(library));

            if (plugin && (plugin->capabilities(nullptr, item.toUtf8()) & cap) != 0) {
               result->append(mimeType.toLatin1());
            }
         }
      }
   }
}

QList<QString> QImageWriter::supportedImageFormats()
{
   QList<QString> formats;
   formats << "bmp";

#ifndef QT_NO_IMAGEFORMAT_PPM
   formats << "pbm" << "pgm" << "ppm";
#endif

#ifndef QT_NO_IMAGEFORMAT_XBM
   formats << "xbm";
#endif

#ifndef QT_NO_IMAGEFORMAT_XPM
   formats << "xpm";
#endif

#ifndef QT_NO_IMAGEFORMAT_PNG
   formats << "png";
#endif

#ifndef QT_NO_IMAGEFORMAT_JPEG
   formats << "jpg" << "jpeg";
#endif

   // keep for now
#ifndef QT_NO_IMAGEFORMAT_TIFF
   formats << "tif" << "tiff";
#endif

#ifdef QT_BUILTIN_GIF_READER
   formats << "gif";
#endif

#ifndef QT_NO_IMAGEFORMAT_ICO
   formats << "ico";
#endif

   supportedImageHandlerFormats(loader(), QImageIOPlugin::CanWrite, &formats);

   std::sort(formats.begin(), formats.end());
   formats.erase(std::unique(formats.begin(), formats.end()), formats.end());

   return formats;
}

QList<QString> QImageWriter::supportedMimeTypes()
{
   QList<QString> mimeTypes;

#ifndef QT_NO_IMAGEFORMAT_BMP
   mimeTypes << "image/bmp";
#endif

#ifndef QT_NO_IMAGEFORMAT_PPM
   mimeTypes << "image/x-portable-bitmap";
   mimeTypes << "image/x-portable-graymap";
   mimeTypes << "image/x-portable-pixmap";
#endif

#ifndef QT_NO_IMAGEFORMAT_XBM
   mimeTypes << "image/x-xbitmap";
#endif

#ifndef QT_NO_IMAGEFORMAT_XPM
   mimeTypes << "image/x-xpixmap";
#endif

#ifndef QT_NO_IMAGEFORMAT_PNG
   mimeTypes << "image/png";
#endif

#ifndef QT_NO_IMAGEFORMAT_JPEG
   mimeTypes << "image/jpeg";
#endif

   supportedImageHandlerMimeTypes(loader(), QImageIOPlugin::CanWrite, &mimeTypes);

   std::sort(mimeTypes.begin(), mimeTypes.end());
   mimeTypes.erase(std::unique(mimeTypes.begin(), mimeTypes.end()), mimeTypes.end());
   return mimeTypes;
}
