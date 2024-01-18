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

#include <qdeclarativestate_p.h>
#include <qdeclarativestategroup_p.h>
#include <qdeclarativestate_p_p.h>
#include <qdeclarativestateoperations_p.h>
#include <qdeclarativeanimation_p.h>
#include <qdeclarativeanimation_p_p.h>
#include <qdeclarativetransitionmanager_p_p.h>
#include <QParallelAnimationGroup>

QT_BEGIN_NAMESPACE

//ParallelAnimationWrapper allows us to do a "callback" when the animation finishes, rather than connecting
//and disconnecting signals and slots frequently
class ParallelAnimationWrapper : public QParallelAnimationGroup
{
   DECL_CS_OBJECT(ParallelAnimationWrapper)

 public:
   ParallelAnimationWrapper(QObject *parent = nullptr) : QParallelAnimationGroup(parent) {}
   QDeclarativeTransitionPrivate *trans;

 protected:
   virtual void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

};

class QDeclarativeTransitionPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeTransition)

 public:
   QDeclarativeTransitionPrivate()
      : fromState(QLatin1String("*")), toState(QLatin1String("*")),
        reversed(false), reversible(false), endState(0) {
      group.trans = this;
   }

   QString fromState;
   QString toState;
   bool reversed;
   bool reversible;
   ParallelAnimationWrapper group;
   QDeclarativeTransitionManager *endState;

   void complete() {
      endState->complete();
   }
   static void append_animation(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list,
                                QDeclarativeAbstractAnimation *a);
   static int animation_count(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list);
   static QDeclarativeAbstractAnimation *animation_at(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list,
         int pos);
   static void clear_animations(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list);
   QList<QDeclarativeAbstractAnimation *> animations;
};

void QDeclarativeTransitionPrivate::append_animation(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list,
      QDeclarativeAbstractAnimation *a)
{
   QDeclarativeTransition *q = static_cast<QDeclarativeTransition *>(list->object);
   q->d_func()->animations.append(a);
   q->d_func()->group.addAnimation(a->qtAnimation());
   a->setDisableUserControl();
}

int QDeclarativeTransitionPrivate::animation_count(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list)
{
   QDeclarativeTransition *q = static_cast<QDeclarativeTransition *>(list->object);
   return q->d_func()->animations.count();
}

QDeclarativeAbstractAnimation *QDeclarativeTransitionPrivate::animation_at(
   QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list, int pos)
{
   QDeclarativeTransition *q = static_cast<QDeclarativeTransition *>(list->object);
   return q->d_func()->animations.at(pos);
}

void QDeclarativeTransitionPrivate::clear_animations(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list)
{
   QDeclarativeTransition *q = static_cast<QDeclarativeTransition *>(list->object);
   while (q->d_func()->animations.count()) {
      QDeclarativeAbstractAnimation *firstAnim = q->d_func()->animations.at(0);
      q->d_func()->group.removeAnimation(firstAnim->qtAnimation());
      q->d_func()->animations.removeAll(firstAnim);
   }
}

void ParallelAnimationWrapper::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
   QParallelAnimationGroup::updateState(newState, oldState);
   if (newState == Stopped && (duration() == -1
                               || (direction() == QAbstractAnimation::Forward && currentLoopTime() == duration())
                               || (direction() == QAbstractAnimation::Backward && currentLoopTime() == 0))) {
      trans->complete();
   }
}



QDeclarativeTransition::QDeclarativeTransition(QObject *parent)
   : QObject(*(new QDeclarativeTransitionPrivate), parent)
{
}

QDeclarativeTransition::~QDeclarativeTransition()
{
}

void QDeclarativeTransition::stop()
{
   Q_D(QDeclarativeTransition);
   d->group.stop();
}

void QDeclarativeTransition::setReversed(bool r)
{
   Q_D(QDeclarativeTransition);
   d->reversed = r;
}

void QDeclarativeTransition::prepare(QDeclarativeStateOperation::ActionList &actions,
                                     QList<QDeclarativeProperty> &after,
                                     QDeclarativeTransitionManager *endState)
{
   Q_D(QDeclarativeTransition);

   qmlExecuteDeferred(this);

   if (d->reversed) {
      for (int ii = d->animations.count() - 1; ii >= 0; --ii) {
         d->animations.at(ii)->transition(actions, after, QDeclarativeAbstractAnimation::Backward);
      }
   } else {
      for (int ii = 0; ii < d->animations.count(); ++ii) {
         d->animations.at(ii)->transition(actions, after, QDeclarativeAbstractAnimation::Forward);
      }
   }

   d->endState = endState;
   d->group.setDirection(d->reversed ? QAbstractAnimation::Backward : QAbstractAnimation::Forward);
   d->group.start();
}

/*!
    \qmlproperty string Transition::from
    \qmlproperty string Transition::to

    These properties indicate the state changes that trigger the transition.

    The default values for these properties is "*" (that is, any state).

    For example, the following transition has not set the \c to and \c from
    properties, so the animation is always applied when changing between
    the two states (i.e. when the mouse is pressed and released).

    \snippet doc/src/snippets/declarative/transition-from-to.qml 0

    If the transition was changed to this:

    \snippet doc/src/snippets/declarative/transition-from-to-modified.qml modified transition

    The animation would only be applied when changing from the default state to
    the "brighter" state (i.e. when the mouse is pressed, but not on release).

    \sa reversible
*/
QString QDeclarativeTransition::fromState() const
{
   Q_D(const QDeclarativeTransition);
   return d->fromState;
}

void QDeclarativeTransition::setFromState(const QString &f)
{
   Q_D(QDeclarativeTransition);
   if (f == d->fromState) {
      return;
   }

   d->fromState = f;
   emit fromChanged();
}

/*!
    \qmlproperty bool Transition::reversible
    This property holds whether the transition should be automatically reversed when the conditions that triggered this transition are reversed.

    The default value is false.

    By default, transitions run in parallel and are applied to all state
    changes if the \l from and \l to states have not been set. In this
    situation, the transition is automatically applied when a state change
    is reversed, and it is not necessary to set this property to reverse
    the transition.

    However, if a SequentialAnimation is used, or if the \l from or \l to
    properties have been set, this property will need to be set to reverse
    a transition when a state change is reverted. For example, the following
    transition applies a sequential animation when the mouse is pressed,
    and reverses the sequence of the animation when the mouse is released:

    \snippet doc/src/snippets/declarative/transition-reversible.qml 0

    If the transition did not set the \c to and \c reversible values, then
    on the mouse release, the transition would play the PropertyAnimation
    before the ColorAnimation instead of reversing the sequence.
*/
bool QDeclarativeTransition::reversible() const
{
   Q_D(const QDeclarativeTransition);
   return d->reversible;
}

void QDeclarativeTransition::setReversible(bool r)
{
   Q_D(QDeclarativeTransition);
   if (r == d->reversible) {
      return;
   }

   d->reversible = r;
   emit reversibleChanged();
}

QString QDeclarativeTransition::toState() const
{
   Q_D(const QDeclarativeTransition);
   return d->toState;
}

void QDeclarativeTransition::setToState(const QString &t)
{
   Q_D(QDeclarativeTransition);
   if (t == d->toState) {
      return;
   }

   d->toState = t;
   emit toChanged();
}

/*!
    \qmlproperty list<Animation> Transition::animations
    \default

    This property holds a list of the animations to be run for this transition.

    \snippet examples/declarative/toys/dynamicscene/dynamicscene.qml top-level transitions

    The top-level animations are run in parallel. To run them sequentially,
    define them within a SequentialAnimation:

    \snippet doc/src/snippets/declarative/transition-reversible.qml sequential animations
*/
QDeclarativeListProperty<QDeclarativeAbstractAnimation> QDeclarativeTransition::animations()
{
   Q_D(QDeclarativeTransition);
   return QDeclarativeListProperty<QDeclarativeAbstractAnimation>(this, &d->animations,
          QDeclarativeTransitionPrivate::append_animation,
          QDeclarativeTransitionPrivate::animation_count,
          QDeclarativeTransitionPrivate::animation_at,
          QDeclarativeTransitionPrivate::clear_animations);
}

QT_END_NAMESPACE

