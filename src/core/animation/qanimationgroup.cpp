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

#include <qanimationgroup.h>
#include <qanimationgroup_p.h>

#include <qalgorithms.h>
#include <qcoreevent.h>
#include <qdebug.h>

#ifndef QT_NO_ANIMATION

QAnimationGroup::QAnimationGroup(QObject *parent)
   : QAbstractAnimation(*new QAnimationGroupPrivate, parent)
{
}

QAnimationGroup::QAnimationGroup(QAnimationGroupPrivate &dd, QObject *parent)
   : QAbstractAnimation(dd, parent)
{
}

QAnimationGroup::~QAnimationGroup()
{
}

QAbstractAnimation *QAnimationGroup::animationAt(int index) const
{
   Q_D(const QAnimationGroup);

   if (index < 0 || index >= d->animations.size()) {
      qWarning("QAnimationGroup::animationAt() Index is out of bounds");
      return nullptr;
   }

   return d->animations.at(index);
}

int QAnimationGroup::animationCount() const
{
   Q_D(const QAnimationGroup);
   return d->animations.size();
}

int QAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
   Q_D(const QAnimationGroup);
   return d->animations.indexOf(animation);
}

void QAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);
   insertAnimation(d->animations.count(), animation);
}

void QAnimationGroup::insertAnimation(int index, QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);

   if (index < 0 || index > d->animations.size()) {
      qWarning("QAnimationGroup::insertAnimation() Index is out of bounds");
      return;
   }

   if (QAnimationGroup *oldGroup = animation->group()) {
      oldGroup->removeAnimation(animation);
   }

   d->animations.insert(index, animation);
   QAbstractAnimationPrivate::get(animation)->group = this;
   // this will make sure that ChildAdded event is sent to 'this'
   animation->setParent(this);
   d->animationInsertedAt(index);
}

void QAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);

   if (! animation) {
      qWarning("QAnimationGroup::remove() Unable to remove null animation");
      return;
   }

   int index = d->animations.indexOf(animation);

   if (index == -1) {
      qWarning("QAnimationGroup::remove() Animation is not part of this group");
      return;
   }

   takeAnimation(index);
}

QAbstractAnimation *QAnimationGroup::takeAnimation(int index)
{
   Q_D(QAnimationGroup);

   if (index < 0 || index >= d->animations.size()) {
      qWarning("QAnimationGroup::takeAnimation() No animation at index %d", index);
      return nullptr;
   }

   QAbstractAnimation *animation = d->animations.at(index);
   QAbstractAnimationPrivate::get(animation)->group = nullptr;

   // ### removing from list before doing setParent to avoid inifinite recursion
   // in ChildRemoved event

   d->animations.removeAt(index);
   animation->setParent(nullptr);
   d->animationRemoved(index, animation);

   return animation;
}

void QAnimationGroup::clear()
{
   Q_D(QAnimationGroup);
   qDeleteAll(d->animations);
}

bool QAnimationGroup::event(QEvent *event)
{
   Q_D(QAnimationGroup);

   if (event->type() == QEvent::ChildAdded) {
      QChildEvent *childEvent = static_cast<QChildEvent *>(event);

      if (QAbstractAnimation *a = qobject_cast<QAbstractAnimation *>(childEvent->child())) {
         if (a->group() != this) {
            addAnimation(a);
         }
      }

   } else if (event->type() == QEvent::ChildRemoved) {
      QChildEvent *childEvent = static_cast<QChildEvent *>(event);
      QAbstractAnimation *a = static_cast<QAbstractAnimation *>(childEvent->child());
      // You can only rely on the child being a QObject because in the QEvent::ChildRemoved
      // case it might be called from the destructor.
      int index = d->animations.indexOf(a);

      if (index != -1) {
         takeAnimation(index);
      }
   }

   return QAbstractAnimation::event(event);
}

void QAnimationGroupPrivate::animationRemoved(int index, QAbstractAnimation *)
{
   Q_Q(QAnimationGroup);
   (void) index;

   if (animations.isEmpty()) {
      currentTime = 0;
      q->stop();
   }
}

#endif //QT_NO_ANIMATION
