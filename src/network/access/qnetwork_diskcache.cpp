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

#include <qnetwork_diskcache.h>
#include <qnetwork_diskcache_p.h>

#include <qcryptographichash.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qfile.h>
#include <qscopedpointer.h>
#include <qurl.h>

#define CACHE_POSTFIX  QString(".d")
#define PREPARED_SLASH QString("prepared/")
#define DATA_DIR       QString("data")

static constexpr const int CacheMagic         = 0xe8;
static constexpr const int CacheVersion       = 8;
static constexpr const int MaxCompressionSize = (1024 * 1024 * 3);

#ifndef QT_NO_NETWORKDISKCACHE

QNetworkDiskCache::QNetworkDiskCache(QObject *parent)
   : QAbstractNetworkCache(*new QNetworkDiskCachePrivate, parent)
{
}

QNetworkDiskCache::~QNetworkDiskCache()
{
   Q_D(QNetworkDiskCache);

   QHashIterator<QIODevice *, QCacheItem *> it(d->inserting);

   while (it.hasNext()) {
      it.next();
      delete it.value();
   }
}

QString QNetworkDiskCache::cacheDirectory() const
{
   Q_D(const QNetworkDiskCache);
   return d->cacheDirectory;
}

void QNetworkDiskCache::setCacheDirectory(const QString &cacheDir)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::setCacheDirectory()" << cacheDir;
#endif

   Q_D(QNetworkDiskCache);

   if (cacheDir.isEmpty()) {
      return;
   }

   d->cacheDirectory = cacheDir;

   QDir dir(d->cacheDirectory);

   d->cacheDirectory = dir.absolutePath();

   if (!d->cacheDirectory.endsWith('/')) {
      d->cacheDirectory += '/';
   }

   d->dataDirectory = d->cacheDirectory + DATA_DIR + QString::number(CacheVersion) + QChar('/');
   d->prepareLayout();
}

qint64 QNetworkDiskCache::cacheSize() const
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::cacheSize()";
#endif

   Q_D(const QNetworkDiskCache);

   if (d->cacheDirectory.isEmpty()) {
      return 0;
   }

   if (d->currentCacheSize < 0) {
      QNetworkDiskCache *that = const_cast<QNetworkDiskCache *>(this);
      that->d_func()->currentCacheSize = that->expire();
   }
   return d->currentCacheSize;
}

QIODevice *QNetworkDiskCache::prepare(const QNetworkCacheMetaData &metaData)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::prepare()" << metaData.url();
#endif

   Q_D(QNetworkDiskCache);

   if (! metaData.isValid() || ! metaData.url().isValid() || !metaData.saveToDisk()) {
      return nullptr;
   }

   if (d->cacheDirectory.isEmpty()) {
      qWarning("QNetworkDiskCache::prepare() Cache directory is not set");
      return nullptr;
   }

   for (const QNetworkCacheMetaData::RawHeader &header : metaData.rawHeaders()) {
      if (header.first.toLower() == "content-length") {

         const qint64 size = header.second.toLongLong();
         if (size > (maximumCacheSize() * 3) / 4) {
            return nullptr;
         }

         break;
      }
   }

   QScopedPointer<QCacheItem> cacheItem(new QCacheItem);
   cacheItem->metaData = metaData;

   QIODevice *device = nullptr;

   if (cacheItem->canCompress()) {
      cacheItem->data.open(QBuffer::ReadWrite);
      device = &(cacheItem->data);

   } else {
      QString templateName = d->tmpCacheFileName();

      try {
         cacheItem->file = new QTemporaryFile(templateName, &cacheItem->data);
      } catch(...) {
         cacheItem->file = nullptr;
      }

      if (! cacheItem->file || !cacheItem->file->open()) {
         qWarning("QNetworkDiskCache::prepare() Unable to open temporary file");
         cacheItem.reset();
         return nullptr;
      }
      cacheItem->writeHeader(cacheItem->file);
      device = cacheItem->file;
   }

   d->inserting[device] = cacheItem.take();
   return device;
}

void QNetworkDiskCache::insert(QIODevice *device)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::insert()" << device;
#endif

   Q_D(QNetworkDiskCache);

   QHash<QIODevice *, QCacheItem *>::iterator it = d->inserting.find(device);
   if (it == d->inserting.end()) {
      qWarning("QNetworkDiskCache::insert() Called on a device which is unknown");
      return;
   }

   d->storeItem(it.value());
   delete it.value();
   d->inserting.erase(it);
}

void QNetworkDiskCachePrivate::prepareLayout()
{
   QDir helper;
   helper.mkpath(cacheDirectory + PREPARED_SLASH);

   //Create directory and subdirectories 0-F
   helper.mkpath(dataDirectory);
   for (uint i = 0; i < 16 ; i++) {
      QString str = QString::number(i, 16);
      QString subdir = dataDirectory + str;
      helper.mkdir(subdir);
   }
}

void QNetworkDiskCachePrivate::storeItem(QCacheItem *cacheItem)
{
   Q_Q(QNetworkDiskCache);
   Q_ASSERT(cacheItem->metaData.saveToDisk());

   QString fileName = cacheFileName(cacheItem->metaData.url());
   Q_ASSERT(!fileName.isEmpty());

   if (QFile::exists(fileName)) {
      if (!QFile::remove(fileName)) {
         qWarning("QNetworkDiskCache::storeItem() Unable to remove the cache file %s", csPrintable(fileName));
         return;
      }
   }

   if (currentCacheSize > 0) {
      currentCacheSize += 1024 + cacheItem->size();
   }

   currentCacheSize = q->expire();

   if (!cacheItem->file) {
      QString templateName = tmpCacheFileName();
      cacheItem->file = new QTemporaryFile(templateName, &cacheItem->data);
      if (cacheItem->file->open()) {
         cacheItem->writeHeader(cacheItem->file);
         cacheItem->writeCompressedData(cacheItem->file);
      }
   }

   if (cacheItem->file && cacheItem->file->isOpen() && cacheItem->file->error() == QFile::NoError) {
      cacheItem->file->setAutoRemove(false);

      // use rename rather then remove & rename
      if (cacheItem->file->rename(fileName)) {
         currentCacheSize += cacheItem->file->size();
      } else {
         cacheItem->file->setAutoRemove(true);
      }
   }

   if (cacheItem->metaData.url() == lastItem.metaData.url()) {
      lastItem.reset();
   }
}

bool QNetworkDiskCache::remove(const QUrl &url)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::remove()" << url;
#endif

   Q_D(QNetworkDiskCache);

   // remove is also used to cancel insertions, not a common operation
   QHashIterator<QIODevice *, QCacheItem *> it(d->inserting);

   while (it.hasNext()) {
      it.next();
      QCacheItem *item = it.value();

      if (item && item->metaData.url() == url) {
         delete item;
         d->inserting.remove(it.key());
         return true;
      }
   }

   if (d->lastItem.metaData.url() == url) {
      d->lastItem.reset();
   }
   return d->removeFile(d->cacheFileName(url));
}

bool QNetworkDiskCachePrivate::removeFile(const QString &file)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::removFile()" << file;
#endif

   if (file.isEmpty()) {
      return false;
   }

   QFileInfo info(file);
   QString fileName = info.fileName();
   if (!fileName.endsWith(CACHE_POSTFIX)) {
      return false;
   }

   qint64 size = info.size();
   if (QFile::remove(file)) {
      currentCacheSize -= size;
      return true;
   }
   return false;
}

QNetworkCacheMetaData QNetworkDiskCache::metaData(const QUrl &url)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::metaData()" << url;
#endif

   Q_D(QNetworkDiskCache);
   if (d->lastItem.metaData.url() == url) {
      return d->lastItem.metaData;
   }
   return fileMetaData(d->cacheFileName(url));
}

QNetworkCacheMetaData QNetworkDiskCache::fileMetaData(const QString &fileName) const
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::fileMetaData()" << fileName;
#endif

   Q_D(const QNetworkDiskCache);
   QFile file(fileName);

   if (!file.open(QFile::ReadOnly)) {
      return QNetworkCacheMetaData();
   }

   if (!d->lastItem.read(&file, false)) {
      file.close();
      QNetworkDiskCachePrivate *that = const_cast<QNetworkDiskCachePrivate *>(d);
      that->removeFile(fileName);
   }

   return d->lastItem.metaData;
}

QIODevice *QNetworkDiskCache::data(const QUrl &url)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::data()" << url;
#endif

   Q_D(QNetworkDiskCache);
   QScopedPointer<QBuffer> buffer;

   if (!url.isValid()) {
      return nullptr;
   }

   if (d->lastItem.metaData.url() == url && d->lastItem.data.isOpen()) {
      buffer.reset(new QBuffer);
      buffer->setData(d->lastItem.data.data());

   } else {
      QScopedPointer<QFile> file(new QFile(d->cacheFileName(url)));
      if (!file->open(QFile::ReadOnly | QIODevice::Unbuffered)) {
         return nullptr;
      }

      if (!d->lastItem.read(file.data(), true)) {
         file->close();
         remove(url);
         return nullptr;
      }

      if (d->lastItem.data.isOpen()) {
         // compressed
         buffer.reset(new QBuffer);
         buffer->setData(d->lastItem.data.data());

      } else {
         buffer.reset(new QBuffer);
         // ### verify that QFile uses the fd size and not the file name
         qint64 size = file->size() - file->pos();
         const uchar *p = nullptr;

         p = file->map(file->pos(), size);

         if (p) {
            buffer->setData((const char *)p, size);
            file.take()->setParent(buffer.data());
         } else {
            buffer->setData(file->readAll());
         }
      }
   }

   buffer->open(QBuffer::ReadOnly);
   return buffer.take();
}

void QNetworkDiskCache::updateMetaData(const QNetworkCacheMetaData &metaData)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::updateMetaData()" << metaData.url();
#endif

   QUrl url = metaData.url();
   QIODevice *oldDevice = data(url);

   if (! oldDevice) {
#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug() << "QNetworkDiskCache::updateMetaData(), no device!";
#endif
      return;
   }

   QIODevice *newDevice = prepare(metaData);

   if (! newDevice) {
#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug() << "QNetworkDiskCache::updateMetaData(), no new device" << url;
#endif
      return;
   }
   char data[1024];
   while (!oldDevice->atEnd()) {
      qint64 s = oldDevice->read(data, 1024);
      newDevice->write(data, s);
   }

   delete oldDevice;
   insert(newDevice);
}

qint64 QNetworkDiskCache::maximumCacheSize() const
{
   Q_D(const QNetworkDiskCache);
   return d->maximumCacheSize;
}

void QNetworkDiskCache::setMaximumCacheSize(qint64 size)
{
   Q_D(QNetworkDiskCache);
   bool expireCache = (size < d->maximumCacheSize);
   d->maximumCacheSize = size;
   if (expireCache) {
      d->currentCacheSize = expire();
   }
}

qint64 QNetworkDiskCache::expire()
{
   Q_D(QNetworkDiskCache);
   if (d->currentCacheSize >= 0 && d->currentCacheSize < maximumCacheSize()) {
      return d->currentCacheSize;
   }

   if (cacheDirectory().isEmpty()) {
      qWarning("QNetworkDiskCache::expire() Cache directory is not set");
      return 0;
   }

   // close file handle to prevent "in use" error when QFile::remove() is called
   d->lastItem.reset();

   QDir::Filters filters = QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot;
   QDirIterator it(cacheDirectory(), filters, QDirIterator::Subdirectories);

   QMultiMap<QDateTime, QString> cacheItems;
   qint64 totalSize = 0;

   while (it.hasNext()) {
      QString path = it.next();
      QFileInfo info = it.fileInfo();
      QString fileName = info.fileName();
      if (fileName.endsWith(CACHE_POSTFIX)) {
         cacheItems.insert(info.created(), path);
         totalSize += info.size();
      }
   }

   int removedFiles = 0;
   qint64 goal = (maximumCacheSize() * 9) / 10;
   QMultiMap<QDateTime, QString>::const_iterator i = cacheItems.constBegin();

   while (i != cacheItems.constEnd()) {
      if (totalSize < goal) {
         break;

      }

      QString name = i.value();
      QFile file(name);

      if (name.contains(PREPARED_SLASH)) {
         QHashIterator<QIODevice *, QCacheItem *> iterator(d->inserting);

         while (iterator.hasNext()) {
            iterator.next();
            QCacheItem *item = iterator.value();

            if (item && item->file && item->file->fileName() == name) {
               delete item->file;
               item->file = nullptr;
               break;
            }
         }
      }
      qint64 size = file.size();
      file.remove();
      totalSize -= size;
      ++removedFiles;
      ++i;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   if (removedFiles > 0) {
      qDebug() << "QNetworkDiskCache::expire()"
            << "Removed:" << removedFiles << "Kept:" << cacheItems.count() - removedFiles;
   }
#endif

   return totalSize;
}

void QNetworkDiskCache::clear()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QNetworkDiskCache::clear()";
#endif

   Q_D(QNetworkDiskCache);
   qint64 size = d->maximumCacheSize;
   d->maximumCacheSize = 0;
   d->currentCacheSize = expire();
   d->maximumCacheSize = size;
}

QString QNetworkDiskCachePrivate::uniqueFileName(const QUrl &url)
{
   QUrl cleanUrl = url;
   cleanUrl.setPassword(QString());
   cleanUrl.setFragment(QString());

   QCryptographicHash hash(QCryptographicHash::Sha1);
   hash.addData(cleanUrl.toEncoded());

   // convert sha1 to base36 form and return first 8 bytes for use as string
   QByteArray id =  QByteArray::number(*(qint64 *)hash.result().data(), 36).left(8);

   // generates <one-char subdir>/<8-char filname.d>
   uint code = (uint)id.at(id.length() - 1) % 16;
   QString pathFragment = QString::number(code, 16) + QChar('/') + id + CACHE_POSTFIX;

   return pathFragment;
}

QString QNetworkDiskCachePrivate::tmpCacheFileName() const
{
   //The subdirectory is presumed to be already read for use.
   return cacheDirectory + PREPARED_SLASH + "XXXXXX" + CACHE_POSTFIX;
}

QString QNetworkDiskCachePrivate::cacheFileName(const QUrl &url) const
{
   if (! url.isValid()) {
      return QString();
   }

   QString fullpath = dataDirectory + uniqueFileName(url);
   return  fullpath;
}

// compress small text and JavaScript files.
bool QCacheItem::canCompress() const
{
   bool sizeOk = false;
   bool typeOk = false;

   for (const QNetworkCacheMetaData::RawHeader &header : metaData.rawHeaders()) {
      if (header.first.toLower() == "content-length") {
         qint64 size = header.second.toLongLong();

         if (size > MaxCompressionSize) {
            return false;
         } else {
            sizeOk = true;
         }
      }

      if (header.first.toLower() == "content-type") {
         QByteArray type = header.second;
         if (type.startsWith("text/") || (type.startsWith("application/")
                   && (type.endsWith("javascript") || type.endsWith("ecmascript")))) {
            typeOk = true;
         } else {
            return false;
         }
      }

      if (sizeOk && typeOk) {
         return true;
      }
   }
   return false;
}

void QCacheItem::writeHeader(QFile *device) const
{
   QDataStream out(device);

   out << qint32(CacheMagic);
   out << qint32(CacheVersion);
   out << static_cast<qint32>(out.version());
   out << metaData;
   bool compressed = canCompress();
   out << compressed;
}

void QCacheItem::writeCompressedData(QFile *device) const
{
   QDataStream out(device);

   out << qCompress(data.data());
}

bool QCacheItem::read(QFile *device, bool readData)
{
   reset();

   QDataStream in(device);

   qint32 marker;
   qint32 v;
   in >> marker;
   in >> v;

   if (marker != CacheMagic) {
      return true;
   }

   // If the cache magic is correct, but the version is not then remove it
   if (v != CacheVersion) {
      return false;
   }

   qint32 streamVersion;
   in >> streamVersion;
   if (streamVersion > in.version()) {
      return false;
   }

   in.setVersion(streamVersion);

   bool compressed;
   QByteArray dataBA;
   in >> metaData;
   in >> compressed;
   if (readData && compressed) {
      in >> dataBA;
      data.setData(qUncompress(dataBA));
      data.open(QBuffer::ReadOnly);
   }

   // quick and dirty check if metadata's URL field and the file's name are in synch
   QString expectedFilename = QNetworkDiskCachePrivate::uniqueFileName(metaData.url());
   if (!device->fileName().endsWith(expectedFilename)) {
      return false;
   }

   return metaData.isValid();
}

#endif // QT_NO_NETWORKDISKCACHE
