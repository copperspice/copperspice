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

#ifndef QNETWORKDISKCACHE_P_H
#define QNETWORKDISKCACHE_P_H

#include <qabstractnetworkcache_p.h>
#include <qbuffer.h>
#include <qhash.h>
#include <qtemporaryfile.h>

#ifndef QT_NO_NETWORKDISKCACHE



class QFile;

class QCacheItem
{
 public:
   QCacheItem() : file(0) {
   }

   ~QCacheItem() {
      reset();
   }

   QNetworkCacheMetaData metaData;
   QBuffer data;
   QTemporaryFile *file;
   inline qint64 size() const {
      return file ? file->size() : data.size();
   }

   inline void reset() {
      metaData = QNetworkCacheMetaData();
      data.close();
      delete file;
      file = 0;
   }
   void writeHeader(QFile *device) const;
   void writeCompressedData(QFile *device) const;
   bool read(QFile *device, bool readData);

   bool canCompress() const;
};

class QNetworkDiskCachePrivate : public QAbstractNetworkCachePrivate
{
 public:
   QNetworkDiskCachePrivate()
      : QAbstractNetworkCachePrivate()
      , maximumCacheSize(1024 * 1024 * 50)
      , currentCacheSize(-1) {
   }

   static QString uniqueFileName(const QUrl &url);
   QString cacheFileName(const QUrl &url) const;
   QString tmpCacheFileName() const;
   bool removeFile(const QString &file);
   void storeItem(QCacheItem *item);
   void prepareLayout();
   static quint32 crc32(const char *data, uint len);

   mutable QCacheItem lastItem;
   QString cacheDirectory;
   QString dataDirectory;
   qint64 maximumCacheSize;
   qint64 currentCacheSize;

   QHash<QIODevice *, QCacheItem *> inserting;
   Q_DECLARE_PUBLIC(QNetworkDiskCache)
};


#endif // QT_NO_NETWORKDISKCACHE

#endif // QNETWORKDISKCACHE_P_H
