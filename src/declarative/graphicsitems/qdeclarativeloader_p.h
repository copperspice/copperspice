/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVELOADER_P_H
#define QDECLARATIVELOADER_P_H

#include <qdeclarativeimplicitsizeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeLoaderPrivate;

class QDeclarativeLoader : public QDeclarativeImplicitSizeItem
{
   CS_OBJECT(QDeclarativeLoader)
   CS_ENUM(Status)

   CS_PROPERTY_READ(source, source)
   CS_PROPERTY_WRITE(source, setSource)
   CS_PROPERTY_NOTIFY(source, sourceChanged)
   CS_PROPERTY_READ(*sourceComponent, sourceComponent)
   CS_PROPERTY_WRITE(*sourceComponent, setSourceComponent)
   CS_PROPERTY_RESET(*sourceComponent, resetSourceComponent)
   CS_PROPERTY_NOTIFY(*sourceComponent, sourceChanged)
   CS_PROPERTY_READ(*item, item)
   CS_PROPERTY_NOTIFY(*item, itemChanged)
   CS_PROPERTY_READ(status, status)
   CS_PROPERTY_NOTIFY(status, statusChanged)
   CS_PROPERTY_READ(progress, progress)
   CS_PROPERTY_NOTIFY(progress, progressChanged)

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

   CS_SIGNAL_1(Public, void itemChanged())
   CS_SIGNAL_2(itemChanged)
   CS_SIGNAL_1(Public, void sourceChanged())
   CS_SIGNAL_2(sourceChanged)
   CS_SIGNAL_1(Public, void statusChanged())
   CS_SIGNAL_2(statusChanged)
   CS_SIGNAL_1(Public, void progressChanged())
   CS_SIGNAL_2(progressChanged)
   CS_SIGNAL_1(Public, void loaded())
   CS_SIGNAL_2(loaded)

 protected:
   void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   QVariant itemChange(GraphicsItemChange change, const QVariant &value);
   bool eventFilter(QObject *watched, QEvent *e);
   void componentComplete();

 private:
   Q_DISABLE_COPY(QDeclarativeLoader)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeLoader)

   CS_SLOT_1(Private, void _q_sourceLoaded())
   CS_SLOT_2(_q_sourceLoaded)

   CS_SLOT_1(Private, void _q_updateSize())
   CS_SLOT_2(_q_updateSize)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeLoader)

#endif // QDECLARATIVELOADER_H
