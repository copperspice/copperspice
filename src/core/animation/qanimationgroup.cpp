/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qanimationgroup.h>
#include <qalgorithms.h>
#include <qdebug.h>
#include <qcoreevent.h>
#include <qanimationgroup_p.h>

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE


/*!
    Constructs a QAnimationGroup.
    \a parent is passed to QObject's constructor.
*/
QAnimationGroup::QAnimationGroup(QObject *parent)
   : QAbstractAnimation(*new QAnimationGroupPrivate, parent)
{
}

/*!
    \internal
*/
QAnimationGroup::QAnimationGroup(QAnimationGroupPrivate &dd, QObject *parent)
   : QAbstractAnimation(dd, parent)
{
}

/*!
    Destroys the animation group. It will also destroy all its animations.
*/
QAnimationGroup::~QAnimationGroup()
{
}

/*!
    Returns a pointer to the animation at \a index in this group. This
    function is useful when you need access to a particular animation.  \a
    index is between 0 and animationCount() - 1.

    \sa animationCount(), indexOfAnimation()
*/
QAbstractAnimation *QAnimationGroup::animationAt(int index) const
{
   Q_D(const QAnimationGroup);

   if (index < 0 || index >= d->animations.size()) {
      qWarning("QAnimationGroup::animationAt: index is out of bounds");
      return 0;
   }

   return d->animations.at(index);
}


/*!
    Returns the number of animations managed by this group.

    \sa indexOfAnimation(), addAnimation(), animationAt()
*/
int QAnimationGroup::animationCount() const
{
   Q_D(const QAnimationGroup);
   return d->animations.size();
}

/*!
    Returns the index of \a animation. The returned index can be passed
    to the other functions that take an index as an argument.

    \sa insertAnimation(), animationAt(), takeAnimation()
*/
int QAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
   Q_D(const QAnimationGroup);
   return d->animations.indexOf(animation);
}

/*!
    Adds \a animation to this group. This will call insertAnimation with
    index equals to animationCount().

    \note The group takes ownership of the animation.

    \sa removeAnimation()
*/
void QAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);
   insertAnimation(d->animations.count(), animation);
}

/*!
    Inserts \a animation into this animation group at \a index.
    If \a index is 0 the animation is inserted at the beginning.
    If \a index is animationCount(), the animation is inserted at the end.

    \note The group takes ownership of the animation.

    \sa takeAnimation(), addAnimation(), indexOfAnimation(), removeAnimation()
*/
void QAnimationGroup::insertAnimation(int index, QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);

   if (index < 0 || index > d->animations.size()) {
      qWarning("QAnimationGroup::insertAnimation: index is out of bounds");
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

/*!
    Removes \a animation from this group. The ownership of \a animation is
    transferred to the caller.

    \sa takeAnimation(), insertAnimation(), addAnimation()
*/
void QAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
   Q_D(QAnimationGroup);

   if (!animation) {
      qWarning("QAnimationGroup::remove: cannot remove null animation");
      return;
   }
   int index = d->animations.indexOf(animation);
   if (index == -1) {
      qWarning("QAnimationGroup::remove: animation is not part of this group");
      return;
   }

   takeAnimation(index);
}

/*!
    Returns the animation at \a index and removes it from the animation group.

    \note The ownership of the animation is transferred to the caller.

    \sa removeAnimation(), addAnimation(), insertAnimation(), indexOfAnimation()
*/
QAbstractAnimation *QAnimationGroup::takeAnimation(int index)
{
   Q_D(QAnimationGroup);
   if (index < 0 || index >= d->animations.size()) {
      qWarning("QAnimationGroup::takeAnimation: no animation at index %d", index);
      return 0;
   }
   QAbstractAnimation *animation = d->animations.at(index);
   QAbstractAnimationPrivate::get(animation)->group = 0;
   // ### removing from list before doing setParent to avoid inifinite recursion
   // in ChildRemoved event
   d->animations.removeAt(index);
   animation->setParent(0);
   d->animationRemoved(index, animation);
   return animation;
}

/*!
    Removes and deletes all animations in this animation group, and resets the current
    time to 0.

    \sa addAnimation(), removeAnimation()
*/
void QAnimationGroup::clear()
{
   Q_D(QAnimationGroup);
   qDeleteAll(d->animations);
}

/*!
    \reimp
*/
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
   Q_UNUSED(index);
   if (animations.isEmpty()) {
      currentTime = 0;
      q->stop();
   }
}

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION
