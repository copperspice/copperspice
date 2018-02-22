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

#ifndef QNETWORKDISKCACHE_H
#define QNETWORKDISKCACHE_H

#include <qabstractnetworkcache.h>


#ifndef QT_NO_NETWORKDISKCACHE

class QNetworkDiskCachePrivate;

class Q_NETWORK_EXPORT QNetworkDiskCache : public QAbstractNetworkCache
{
   NET_CS_OBJECT(QNetworkDiskCache)

 public:
   explicit QNetworkDiskCache(QObject *parent = nullptr);
   ~QNetworkDiskCache();

   QString cacheDirectory() const;
   void setCacheDirectory(const QString &cacheDir);

   qint64 maximumCacheSize() const;
   void setMaximumCacheSize(qint64 size);

   qint64 cacheSize() const override;
   QNetworkCacheMetaData metaData(const QUrl &url) override;
   void updateMetaData(const QNetworkCacheMetaData &metaData) override;
   QIODevice *data(const QUrl &url) override;
   bool remove(const QUrl &url) override;
   QIODevice *prepare(const QNetworkCacheMetaData &metaData) override;
   void insert(QIODevice *device) override;

   QNetworkCacheMetaData fileMetaData(const QString &fileName) const;

   NET_CS_SLOT_1(Public, void clear() override)
   NET_CS_SLOT_2(clear)

 protected:
   virtual qint64 expire();

 private:
   Q_DECLARE_PRIVATE(QNetworkDiskCache)
   Q_DISABLE_COPY(QNetworkDiskCache)
};

#endif // QT_NO_NETWORKDISKCACHE



#endif // QNETWORKDISKCACHE_H
