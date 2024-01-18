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

#ifndef QPLATFORM_SHAREDGRAPHICSCACHE_H
#define QPLATFORM_SHAREDGRAPHICSCACHE_H

#include <qobject.h>
#include <qimage.h>

class Q_GUI_EXPORT QPlatformSharedGraphicsCache: public QObject
{
   GUI_CS_OBJECT(QPlatformSharedGraphicsCache)

 public:
   enum PixelFormat {
      Alpha8
   };

   enum BufferType {
      OpenGLTexture
   };

   explicit QPlatformSharedGraphicsCache(QObject *parent = nullptr) : QObject(parent) {}

   virtual void beginRequestBatch() = 0;
   virtual void ensureCacheInitialized(const QByteArray &cacheId, BufferType bufferType,
      PixelFormat pixelFormat) = 0;

   virtual void requestItems(const QByteArray &cacheId, const QVector<quint32> &itemIds) = 0;
   virtual void insertItems(const QByteArray &cacheId, const QVector<quint32> &itemIds,
      const QVector<QImage> &items) = 0;

   virtual void releaseItems(const QByteArray &cacheId, const QVector<quint32> &itemIds) = 0;
   virtual void endRequestBatch() = 0;

   virtual bool requestBatchStarted() const = 0;

   virtual uint textureIdForBuffer(void *bufferId) = 0;
   virtual void referenceBuffer(void *bufferId) = 0;
   virtual bool dereferenceBuffer(void *bufferId) = 0;
   virtual QSize sizeOfBuffer(void *bufferId) = 0;
   virtual void *eglImageForBuffer(void *bufferId) = 0;

   GUI_CS_SIGNAL_1(Public, void itemsMissing(const QByteArray &cacheId, const QVector <quint32> &itemIds))
   GUI_CS_SIGNAL_2(itemsMissing, cacheId, itemIds)

   GUI_CS_SIGNAL_1(Public, void itemsAvailable(const QByteArray &cacheId, void *bufferId,
         const QVector <quint32> &itemIds, const QVector <QPoint> &positionsInBuffer))
   GUI_CS_SIGNAL_2(itemsAvailable, cacheId, bufferId, itemIds, positionsInBuffer)

   GUI_CS_SIGNAL_1(Public, void itemsInvalidated(const QByteArray &cacheId, const QVector <quint32> &itemIds))
   GUI_CS_SIGNAL_2(itemsInvalidated, cacheId, itemIds)

   GUI_CS_SIGNAL_1(Public, void itemsUpdated(const QByteArray &cacheId, void *bufferId,
         const QVector <quint32> &itemIds, const QVector <QPoint> &positionsInBuffer))
   GUI_CS_SIGNAL_2(itemsUpdated, cacheId, bufferId, itemIds, positionsInBuffer)
};

#endif
