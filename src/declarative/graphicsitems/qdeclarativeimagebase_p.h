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

#ifndef QDECLARATIVEIMAGEBASE_P_H
#define QDECLARATIVEIMAGEBASE_P_H

#include <qdeclarativeimplicitsizeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeImageBasePrivate;

class QDeclarativeImageBase : public QDeclarativeImplicitSizeItem
{
   DECL_CS_OBJECT(QDeclarativeImageBase)

   DECL_CS_ENUM(Status)

   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)
   DECL_CS_PROPERTY_READ(source, source)
   DECL_CS_PROPERTY_WRITE(source, setSource)
   DECL_CS_PROPERTY_NOTIFY(source, sourceChanged)
   DECL_CS_PROPERTY_READ(progress, progress)
   DECL_CS_PROPERTY_NOTIFY(progress, progressChanged)
   DECL_CS_PROPERTY_READ(asynchronous, asynchronous)
   DECL_CS_PROPERTY_WRITE(asynchronous, setAsynchronous)
   DECL_CS_PROPERTY_NOTIFY(asynchronous, asynchronousChanged)
   DECL_CS_PROPERTY_READ(cache, cache)
   DECL_CS_PROPERTY_WRITE(cache, setCache)
   DECL_CS_PROPERTY_NOTIFY(cache, cacheChanged)
   DECL_CS_PROPERTY_REVISION(cache, 1)
   DECL_CS_PROPERTY_READ(sourceSize, sourceSize)
   DECL_CS_PROPERTY_WRITE(sourceSize, setSourceSize)
   DECL_CS_PROPERTY_RESET(sourceSize, resetSourceSize)
   DECL_CS_PROPERTY_NOTIFY(sourceSize, sourceSizeChanged)
   DECL_CS_PROPERTY_READ(mirror, mirror)
   DECL_CS_PROPERTY_WRITE(mirror, setMirror)
   DECL_CS_PROPERTY_NOTIFY(mirror, mirrorChanged)
   DECL_CS_PROPERTY_REVISION(mirror, 1)

 public:
   QDeclarativeImageBase(QDeclarativeItem *parent = 0);
   ~QDeclarativeImageBase();
   enum Status { Null, Ready, Loading, Error };
   Status status() const;
   qreal progress() const;

   QUrl source() const;
   virtual void setSource(const QUrl &url);

   bool asynchronous() const;
   void setAsynchronous(bool);

   bool cache() const;
   void setCache(bool);

   virtual void setSourceSize(const QSize &);
   QSize sourceSize() const;
   void resetSourceSize();

   virtual void setMirror(bool mirror);
   bool mirror() const;

   DECL_CS_SIGNAL_1(Public, void sourceChanged(const QUrl &un_named_arg1))
   DECL_CS_SIGNAL_2(sourceChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void sourceSizeChanged())
   DECL_CS_SIGNAL_2(sourceSizeChanged)
   DECL_CS_SIGNAL_1(Public, void statusChanged(QDeclarativeImageBase::Status un_named_arg1))
   DECL_CS_SIGNAL_2(statusChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void progressChanged(qreal progress))
   DECL_CS_SIGNAL_2(progressChanged, progress)
   DECL_CS_SIGNAL_1(Public, void asynchronousChanged())
   DECL_CS_SIGNAL_2(asynchronousChanged)

   DECL_CS_SIGNAL_1(Public, void cacheChanged())
   DECL_CS_SIGNAL_2(cacheChanged)
   DECL_CS_REVISION(cacheChanged, 1)

   DECL_CS_SIGNAL_1(Public, void mirrorChanged())
   DECL_CS_SIGNAL_2(mirrorChanged)
   DECL_CS_REVISION(mirrorChanged, 1)

 protected:
   virtual void load();
   virtual void componentComplete();
   virtual void pixmapChange();
   QDeclarativeImageBase(QDeclarativeImageBasePrivate &dd, QDeclarativeItem *parent);

 private :
   DECL_CS_SLOT_1(Private, virtual void requestFinished())
   DECL_CS_SLOT_2(requestFinished)
   DECL_CS_SLOT_1(Private, void requestProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   DECL_CS_SLOT_2(requestProgress)

   Q_DISABLE_COPY(QDeclarativeImageBase)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeImageBase)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEIMAGEBASE_H
