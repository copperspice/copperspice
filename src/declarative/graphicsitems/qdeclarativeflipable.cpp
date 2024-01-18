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

#include <qdeclarativeflipable_p.h>
#include <qdeclarativeitem_p.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativeinfo.h>
#include <QtGui/qgraphicstransform.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFlipablePrivate : public QDeclarativeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeFlipable)
 public:
   QDeclarativeFlipablePrivate() : current(QDeclarativeFlipable::Front), front(0), back(0) {}

   void updateSceneTransformFromParent();
   void setBackTransform();

   QDeclarativeFlipable::Side current;
   QDeclarativeGuard<QGraphicsObject> front;
   QDeclarativeGuard<QGraphicsObject> back;

   bool wantBackXFlipped;
   bool wantBackYFlipped;
};

/*!
    \qmlclass Flipable QDeclarativeFlipable
    \since 4.7
    \ingroup qml-basic-interaction-elements
    \brief The Flipable item provides a surface that can be flipped.
    \inherits Item

    Flipable is an item that can be visibly "flipped" between its front and
    back sides, like a card. It is used together with \l Rotation, \l State
    and \l Transition elements to produce a flipping effect.

    The \l front and \l back properties are used to hold the items that are
    shown respectively on the front and back sides of the flipable item.

    \section1 Example Usage

    The following example shows a Flipable item that flips whenever it is
    clicked, rotating about the y-axis.

    This flipable item has a \c flipped boolean property that is toggled
    whenever the MouseArea within the flipable is clicked. When
    \c flipped is true, the item changes to the "back" state; in this
    state, the \c angle of the \l Rotation item is changed to 180
    degrees to produce the flipping effect. When \c flipped is false, the
    item reverts to the default state, in which the \c angle value is 0.

    \snippet doc/src/snippets/declarative/flipable/flipable.qml 0

    \image flipable.gif

    The \l Transition creates the animation that changes the angle over
    four seconds. When the item changes between its "back" and
    default states, the NumberAnimation animates the angle between
    its old and new values.

    See \l {QML States} for details on state changes and the default
    state, and \l {QML Animation and Transitions} for more information on how
    animations work within transitions.

    \sa {declarative/ui-components/flipable}{Flipable example}
*/

QDeclarativeFlipable::QDeclarativeFlipable(QDeclarativeItem *parent)
   : QDeclarativeItem(*(new QDeclarativeFlipablePrivate), parent)
{
}

QDeclarativeFlipable::~QDeclarativeFlipable()
{
}

/*!
  \qmlproperty Item Flipable::front
  \qmlproperty Item Flipable::back

  The front and back sides of the flipable.
*/

QGraphicsObject *QDeclarativeFlipable::front()
{
   Q_D(const QDeclarativeFlipable);
   return d->front;
}

void QDeclarativeFlipable::setFront(QGraphicsObject *front)
{
   Q_D(QDeclarativeFlipable);
   if (d->front) {
      qmlInfo(this) << tr("front is a write-once property");
      return;
   }
   d->front = front;
   d->front->setParentItem(this);
   if (Back == d->current) {
      d->front->setOpacity(0.);
   }
   emit frontChanged();
}

QGraphicsObject *QDeclarativeFlipable::back()
{
   Q_D(const QDeclarativeFlipable);
   return d->back;
}

void QDeclarativeFlipable::setBack(QGraphicsObject *back)
{
   Q_D(QDeclarativeFlipable);
   if (d->back) {
      qmlInfo(this) << tr("back is a write-once property");
      return;
   }
   d->back = back;
   d->back->setParentItem(this);
   if (Front == d->current) {
      d->back->setOpacity(0.);
   }
   connect(back, SIGNAL(widthChanged()),
           this, SLOT(retransformBack()));
   connect(back, SIGNAL(heightChanged()),
           this, SLOT(retransformBack()));
   emit backChanged();
}

void QDeclarativeFlipable::retransformBack()
{
   Q_D(QDeclarativeFlipable);
   if (d->current == QDeclarativeFlipable::Back && d->back) {
      d->setBackTransform();
   }
}

/*!
  \qmlproperty enumeration Flipable::side

  The side of the Flipable currently visible. Possible values are \c
  Flipable.Front and \c Flipable.Back.
*/
QDeclarativeFlipable::Side QDeclarativeFlipable::side() const
{
   Q_D(const QDeclarativeFlipable);
   if (d->dirtySceneTransform) {
      const_cast<QDeclarativeFlipablePrivate *>(d)->ensureSceneTransform();
   }

   return d->current;
}

// determination on the currently visible side of the flipable
// has to be done on the complete scene transform to give
// correct results.
void QDeclarativeFlipablePrivate::updateSceneTransformFromParent()
{
   Q_Q(QDeclarativeFlipable);

   QDeclarativeItemPrivate::updateSceneTransformFromParent();
   QPointF p1(0, 0);
   QPointF p2(1, 0);
   QPointF p3(1, 1);

   QPointF scenep1 = sceneTransform.map(p1);
   QPointF scenep2 = sceneTransform.map(p2);
   QPointF scenep3 = sceneTransform.map(p3);
   p1 = q->mapToParent(p1);
   p2 = q->mapToParent(p2);
   p3 = q->mapToParent(p3);

   qreal cross = (scenep1.x() - scenep2.x()) * (scenep3.y() - scenep2.y()) -
                 (scenep1.y() - scenep2.y()) * (scenep3.x() - scenep2.x());

   wantBackYFlipped = p1.x() >= p2.x();
   wantBackXFlipped = p2.y() >= p3.y();

   QDeclarativeFlipable::Side newSide;
   if (cross > 0) {
      newSide = QDeclarativeFlipable::Back;
   } else {
      newSide = QDeclarativeFlipable::Front;
   }

   if (newSide != current) {
      current = newSide;
      if (current == QDeclarativeFlipable::Back && back) {
         setBackTransform();
      }
      if (front) {
         front->setOpacity((current == QDeclarativeFlipable::Front) ? qreal(1.) : qreal(0.));
      }
      if (back) {
         back->setOpacity((current == QDeclarativeFlipable::Back) ? qreal(1.) : qreal(0.));
      }
      emit q->sideChanged();
   }
}

/* Depends on the width/height of the back item, and so needs reevaulating
   if those change.
*/
void QDeclarativeFlipablePrivate::setBackTransform()
{
   QTransform mat;
   QGraphicsItemPrivate *dBack = QGraphicsItemPrivate::get(back);
   mat.translate(dBack->width() / 2, dBack->height() / 2);
   if (dBack->width() && wantBackYFlipped) {
      mat.rotate(180, Qt::YAxis);
   }
   if (dBack->height() && wantBackXFlipped) {
      mat.rotate(180, Qt::XAxis);
   }
   mat.translate(-dBack->width() / 2, -dBack->height() / 2);
   back->setTransform(mat);
}

QT_END_NAMESPACE
