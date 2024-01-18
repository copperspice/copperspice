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

#ifndef QDECLARATIVEPositioners_P_P_H
#define QDECLARATIVEPositioners_P_P_H

#include <qdeclarativepositioners_p.h>
#include <qdeclarativeimplicitsizeitem_p_p.h>
#include <qdeclarativestate_p.h>
#include <qdeclarativetransitionmanager_p_p.h>
#include <qdeclarativestateoperations_p.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QDeclarativeBasePositionerPrivate : public QDeclarativeImplicitSizeItemPrivate,
   public QDeclarativeItemChangeListener
{
   Q_DECLARE_PUBLIC(QDeclarativeBasePositioner)

 public:
   QDeclarativeBasePositionerPrivate()
      : spacing(0), type(QDeclarativeBasePositioner::None)
      , moveTransition(0), addTransition(0), queuedPositioning(false)
      , doingPositioning(false), anchorConflict(false), layoutDirection(Qt::LeftToRight) {
   }

   void init(QDeclarativeBasePositioner::PositionerType at) {
      type = at;
   }

   int spacing;

   QDeclarativeBasePositioner::PositionerType type;
   QDeclarativeTransition *moveTransition;
   QDeclarativeTransition *addTransition;
   QDeclarativeStateOperation::ActionList addActions;
   QDeclarativeStateOperation::ActionList moveActions;
   QDeclarativeTransitionManager addTransitionManager;
   QDeclarativeTransitionManager moveTransitionManager;

   void watchChanges(QGraphicsObject *other);
   void unwatchChanges(QGraphicsObject *other);
   bool queuedPositioning : 1;
   bool doingPositioning : 1;
   bool anchorConflict : 1;

   Qt::LayoutDirection layoutDirection;


   void schedulePositioning() {
      Q_Q(QDeclarativeBasePositioner);
      if (!queuedPositioning) {
         QTimer::singleShot(0, q, SLOT(prePositioning()));
         queuedPositioning = true;
      }
   }

   void mirrorChange() {
      Q_Q(QDeclarativeBasePositioner);
      if (type != QDeclarativeBasePositioner::Vertical) {
         q->prePositioning();
      }
   }
   bool isLeftToRight() const {
      if (type == QDeclarativeBasePositioner::Vertical) {
         return true;
      } else {
         return effectiveLayoutMirror ? layoutDirection == Qt::RightToLeft : layoutDirection == Qt::LeftToRight;
      }
   }

   virtual void itemSiblingOrderChanged(QDeclarativeItem *other) {
      Q_UNUSED(other);
      //Delay is due to many children often being reordered at once
      //And we only want to reposition them all once
      schedulePositioning();
   }

   void itemGeometryChanged(QDeclarativeItem *, const QRectF &newGeometry, const QRectF &oldGeometry) {
      Q_Q(QDeclarativeBasePositioner);
      if (newGeometry.size() != oldGeometry.size()) {
         q->prePositioning();
      }
   }

   virtual void itemVisibilityChanged(QDeclarativeItem *) {
      schedulePositioning();
   }
   virtual void itemOpacityChanged(QDeclarativeItem *) {
      Q_Q(QDeclarativeBasePositioner);
      q->prePositioning();
   }

   void itemDestroyed(QDeclarativeItem *item) {
      Q_Q(QDeclarativeBasePositioner);
      q->positionedItems.removeOne(QDeclarativeBasePositioner::PositionedItem(item));
   }

   static Qt::LayoutDirection getLayoutDirection(const QDeclarativeBasePositioner *positioner) {
      return positioner->d_func()->layoutDirection;
   }

   static Qt::LayoutDirection getEffectiveLayoutDirection(const QDeclarativeBasePositioner *positioner) {
      if (positioner->d_func()->effectiveLayoutMirror) {
         return positioner->d_func()->layoutDirection == Qt::RightToLeft ? Qt::LeftToRight : Qt::RightToLeft;
      } else {
         return positioner->d_func()->layoutDirection;
      }
   }
};

QT_END_NAMESPACE
#endif
