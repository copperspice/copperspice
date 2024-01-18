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

#include <qdeclarativeimagebase_p.h>
#include <qdeclarativeimagebase_p_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativepixmapcache_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeImageBase::QDeclarativeImageBase(QDeclarativeItem *parent)
   : QDeclarativeImplicitSizeItem(*(new QDeclarativeImageBasePrivate), parent)
{
}

QDeclarativeImageBase::QDeclarativeImageBase(QDeclarativeImageBasePrivate &dd, QDeclarativeItem *parent)
   : QDeclarativeImplicitSizeItem(dd, parent)
{
}

QDeclarativeImageBase::~QDeclarativeImageBase()
{
}

QDeclarativeImageBase::Status QDeclarativeImageBase::status() const
{
   Q_D(const QDeclarativeImageBase);
   return d->status;
}


qreal QDeclarativeImageBase::progress() const
{
   Q_D(const QDeclarativeImageBase);
   return d->progress;
}


bool QDeclarativeImageBase::asynchronous() const
{
   Q_D(const QDeclarativeImageBase);
   return d->async;
}

void QDeclarativeImageBase::setAsynchronous(bool async)
{
   Q_D(QDeclarativeImageBase);
   if (d->async != async) {
      d->async = async;
      emit asynchronousChanged();
   }
}

QUrl QDeclarativeImageBase::source() const
{
   Q_D(const QDeclarativeImageBase);
   return d->url;
}

void QDeclarativeImageBase::setSource(const QUrl &url)
{
   Q_D(QDeclarativeImageBase);
   //equality is fairly expensive, so we bypass for simple, common case
   if ((d->url.isEmpty() == url.isEmpty()) && url == d->url) {
      return;
   }

   d->url = url;
   emit sourceChanged(d->url);

   if (isComponentComplete()) {
      load();
   }
}

void QDeclarativeImageBase::setSourceSize(const QSize &size)
{
   Q_D(QDeclarativeImageBase);
   if (d->sourcesize == size) {
      return;
   }

   d->sourcesize = size;
   d->explicitSourceSize = true;
   emit sourceSizeChanged();
   if (isComponentComplete()) {
      load();
   }
}

QSize QDeclarativeImageBase::sourceSize() const
{
   Q_D(const QDeclarativeImageBase);

   int width = d->sourcesize.width();
   int height = d->sourcesize.height();
   return QSize(width != -1 ? width : d->pix.width(), height != -1 ? height : d->pix.height());
}

void QDeclarativeImageBase::resetSourceSize()
{
   Q_D(QDeclarativeImageBase);
   if (!d->explicitSourceSize) {
      return;
   }
   d->explicitSourceSize = false;
   d->sourcesize = QSize();
   emit sourceSizeChanged();
   if (isComponentComplete()) {
      load();
   }
}

bool QDeclarativeImageBase::cache() const
{
   Q_D(const QDeclarativeImageBase);
   return d->cache;
}

void QDeclarativeImageBase::setCache(bool cache)
{
   Q_D(QDeclarativeImageBase);
   if (d->cache == cache) {
      return;
   }

   d->cache = cache;
   emit cacheChanged();
   if (isComponentComplete()) {
      load();
   }
}

void QDeclarativeImageBase::setMirror(bool mirror)
{
   Q_D(QDeclarativeImageBase);
   if (mirror == d->mirror) {
      return;
   }

   d->mirror = mirror;

   if (isComponentComplete()) {
      update();
   }

   emit mirrorChanged();
}

bool QDeclarativeImageBase::mirror() const
{
   Q_D(const QDeclarativeImageBase);
   return d->mirror;
}

void QDeclarativeImageBase::load()
{
   Q_D(QDeclarativeImageBase);

   if (d->url.isEmpty()) {
      d->pix.clear(this);
      d->status = Null;
      d->progress = 0.0;
      pixmapChange();
      emit progressChanged(d->progress);
      emit statusChanged(d->status);
      update();
   } else {
      QDeclarativePixmap::Options options;
      if (d->async) {
         options |= QDeclarativePixmap::Asynchronous;
      }
      if (d->cache) {
         options |= QDeclarativePixmap::Cache;
      }
      d->pix.clear(this);
      d->pix.load(qmlEngine(this), d->url, d->explicitSourceSize ? sourceSize() : QSize(), options);

      if (d->pix.isLoading()) {
         d->progress = 0.0;
         d->status = Loading;
         emit progressChanged(d->progress);
         emit statusChanged(d->status);

         static int thisRequestProgress = -1;
         static int thisRequestFinished = -1;
         if (thisRequestProgress == -1) {
            thisRequestProgress =
               QDeclarativeImageBase::staticMetaObject.indexOfSlot("requestProgress(qint64,qint64)");
            thisRequestFinished =
               QDeclarativeImageBase::staticMetaObject.indexOfSlot("requestFinished()");
         }

         d->pix.connectFinished(this, thisRequestFinished);
         d->pix.connectDownloadProgress(this, thisRequestProgress);

      } else {
         requestFinished();
      }
   }
}

void QDeclarativeImageBase::requestFinished()
{
   Q_D(QDeclarativeImageBase);

   QDeclarativeImageBase::Status oldStatus = d->status;
   qreal oldProgress = d->progress;

   if (d->pix.isError()) {
      d->status = Error;
      qmlInfo(this) << d->pix.error();
   } else {
      d->status = Ready;
   }

   d->progress = 1.0;

   pixmapChange();

   if (d->sourcesize.width() != d->pix.width() || d->sourcesize.height() != d->pix.height()) {
      emit sourceSizeChanged();
   }

   if (d->status != oldStatus) {
      emit statusChanged(d->status);
   }
   if (d->progress != oldProgress) {
      emit progressChanged(d->progress);
   }

   update();
}

void QDeclarativeImageBase::requestProgress(qint64 received, qint64 total)
{
   Q_D(QDeclarativeImageBase);
   if (d->status == Loading && total > 0) {
      d->progress = qreal(received) / total;
      emit progressChanged(d->progress);
   }
}

void QDeclarativeImageBase::componentComplete()
{
   Q_D(QDeclarativeImageBase);
   QDeclarativeItem::componentComplete();
   if (d->url.isValid()) {
      load();
   }
}

void QDeclarativeImageBase::pixmapChange()
{
   Q_D(QDeclarativeImageBase);
   setImplicitWidth(d->pix.width());
   setImplicitHeight(d->pix.height());
}

QT_END_NAMESPACE
