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

#ifndef QDECLARATIVELOADER_P_H
#define QDECLARATIVELOADER_P_H

#include <qdeclarativeimplicitsizeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeLoaderPrivate;

class QDeclarativeLoader : public QDeclarativeImplicitSizeItem
{
   DECL_CS_OBJECT(QDeclarativeLoader)
   DECL_CS_ENUM(Status)

   DECL_CS_PROPERTY_READ(source, source)
   DECL_CS_PROPERTY_WRITE(source, setSource)
   DECL_CS_PROPERTY_NOTIFY(source, sourceChanged)
   DECL_CS_PROPERTY_READ(*sourceComponent, sourceComponent)
   DECL_CS_PROPERTY_WRITE(*sourceComponent, setSourceComponent)
   DECL_CS_PROPERTY_RESET(*sourceComponent, resetSourceComponent)
   DECL_CS_PROPERTY_NOTIFY(*sourceComponent, sourceChanged)
   DECL_CS_PROPERTY_READ(*item, item)
   DECL_CS_PROPERTY_NOTIFY(*item, itemChanged)
   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)
   DECL_CS_PROPERTY_READ(progress, progress)
   DECL_CS_PROPERTY_NOTIFY(progress, progressChanged)

 public:
   QDeclarativeLoader(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativeLoader();

   QUrl source() const;
   void setSource(const QUrl &);

   QDeclarativeComponent *sourceComponent() const;
   void setSourceComponent(QDeclarativeComponent *);
   void resetSourceComponent();

   enum Status { Null, Ready, Loading, Error };
   Status status() const;
   qreal progress() const;

   QGraphicsObject *item() const;

   DECL_CS_SIGNAL_1(Public, void itemChanged())
   DECL_CS_SIGNAL_2(itemChanged)
   DECL_CS_SIGNAL_1(Public, void sourceChanged())
   DECL_CS_SIGNAL_2(sourceChanged)
   DECL_CS_SIGNAL_1(Public, void statusChanged())
   DECL_CS_SIGNAL_2(statusChanged)
   DECL_CS_SIGNAL_1(Public, void progressChanged())
   DECL_CS_SIGNAL_2(progressChanged)
   DECL_CS_SIGNAL_1(Public, void loaded())
   DECL_CS_SIGNAL_2(loaded)

 protected:
   void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   QVariant itemChange(GraphicsItemChange change, const QVariant &value);
   bool eventFilter(QObject *watched, QEvent *e);
   void componentComplete();

 private:
   Q_DISABLE_COPY(QDeclarativeLoader)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeLoader)

   DECL_CS_SLOT_1(Private, void _q_sourceLoaded())
   DECL_CS_SLOT_2(_q_sourceLoaded)

   DECL_CS_SLOT_1(Private, void _q_updateSize())
   DECL_CS_SLOT_2(_q_updateSize)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeLoader)

#endif // QDECLARATIVELOADER_H
