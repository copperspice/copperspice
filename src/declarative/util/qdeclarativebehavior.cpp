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

#include <qdeclarativebehavior_p.h>
#include <qdeclarativeanimation_p.h>
#include <qdeclarativetransition_p.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativeengine_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBehaviorPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeBehavior)

 public:
   QDeclarativeBehaviorPrivate() : animation(0), enabled(true), finalized(false)
      , blockRunningChanged(false) {}

   QDeclarativeProperty property;
   QVariant currentValue;
   QVariant targetValue;
   QDeclarativeGuard<QDeclarativeAbstractAnimation> animation;
   bool enabled;
   bool finalized;
   bool blockRunningChanged;
};

/*!
    \qmlclass Behavior QDeclarativeBehavior
    \ingroup qml-animation-transition
    \since 4.7
    \brief The Behavior element allows you to specify a default animation for a property change.

    A Behavior defines the default animation to be applied whenever a
    particular property value changes.

    For example, the following Behavior defines a NumberAnimation to be run
    whenever the \l Rectangle's \c width value changes. When the MouseArea
    is clicked, the \c width is changed, triggering the behavior's animation:

    \snippet doc/src/snippets/declarative/behavior.qml 0

    Note that a property cannot have more than one assigned Behavior. To provide
    multiple animations within a Behavior, use ParallelAnimation or
    SequentialAnimation.

    If a \l{QML States}{state change} has a \l Transition that matches the same property as a
    Behavior, the \l Transition animation overrides the Behavior for that
    state change. For general advice on using Behaviors to animate state changes, see
    \l{Using QML Behaviors with States}.

    \sa {QML Animation and Transitions}, {declarative/animation/behaviors}{Behavior example}, QtDeclarative
*/


QDeclarativeBehavior::QDeclarativeBehavior(QObject *parent)
   : QObject(*(new QDeclarativeBehaviorPrivate), parent)
{
}

QDeclarativeBehavior::~QDeclarativeBehavior()
{
}

/*!
    \qmlproperty Animation Behavior::animation
    \default

    This property holds the animation to run when the behavior is triggered.
*/

QDeclarativeAbstractAnimation *QDeclarativeBehavior::animation()
{
   Q_D(QDeclarativeBehavior);
   return d->animation;
}

void QDeclarativeBehavior::setAnimation(QDeclarativeAbstractAnimation *animation)
{
   Q_D(QDeclarativeBehavior);
   if (d->animation) {
      qmlInfo(this) << tr("Cannot change the animation assigned to a Behavior.");
      return;
   }

   d->animation = animation;
   if (d->animation) {
      d->animation->setDefaultTarget(d->property);
      d->animation->setDisableUserControl();
      connect(d->animation->qtAnimation(),
              SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)),
              this,
              SLOT(qtAnimationStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
   }
}


void QDeclarativeBehavior::qtAnimationStateChanged(QAbstractAnimation::State newState, QAbstractAnimation::State)
{
   Q_D(QDeclarativeBehavior);
   if (!d->blockRunningChanged) {
      d->animation->notifyRunningChanged(newState == QAbstractAnimation::Running);
   }
}


/*!
    \qmlproperty bool Behavior::enabled

    This property holds whether the behavior will be triggered when the tracked
    property changes value.

    By default a Behavior is enabled.
*/

bool QDeclarativeBehavior::enabled() const
{
   Q_D(const QDeclarativeBehavior);
   return d->enabled;
}

void QDeclarativeBehavior::setEnabled(bool enabled)
{
   Q_D(QDeclarativeBehavior);
   if (d->enabled == enabled) {
      return;
   }
   d->enabled = enabled;
   emit enabledChanged();
}

void QDeclarativeBehavior::write(const QVariant &value)
{
   Q_D(QDeclarativeBehavior);
   qmlExecuteDeferred(this);
   if (!d->animation || !d->enabled || !d->finalized) {
      QDeclarativePropertyPrivate::write(d->property, value,
                                         QDeclarativePropertyPrivate::BypassInterceptor | QDeclarativePropertyPrivate::DontRemoveBinding);
      d->targetValue = value;
      return;
   }

   if (d->animation->isRunning() && value == d->targetValue) {
      return;
   }

   d->currentValue = d->property.read();
   d->targetValue = value;

   if (d->animation->qtAnimation()->duration() != -1
         && d->animation->qtAnimation()->state() != QAbstractAnimation::Stopped) {
      d->blockRunningChanged = true;
      d->animation->qtAnimation()->stop();
   }

   QDeclarativeStateOperation::ActionList actions;
   QDeclarativeAction action;
   action.property = d->property;
   action.fromValue = d->currentValue;
   action.toValue = value;
   actions << action;

   QList<QDeclarativeProperty> after;
   d->animation->transition(actions, after, QDeclarativeAbstractAnimation::Forward);
   d->animation->qtAnimation()->start();
   d->blockRunningChanged = false;
   if (!after.contains(d->property)) {
      QDeclarativePropertyPrivate::write(d->property, value,
                                         QDeclarativePropertyPrivate::BypassInterceptor | QDeclarativePropertyPrivate::DontRemoveBinding);
   }
}

void QDeclarativeBehavior::setTarget(const QDeclarativeProperty &property)
{
   Q_D(QDeclarativeBehavior);
   d->property = property;
   d->currentValue = property.read();
   if (d->animation) {
      d->animation->setDefaultTarget(property);
   }

   QDeclarativeEnginePrivate *engPriv = QDeclarativeEnginePrivate::get(qmlEngine(this));
   engPriv->registerFinalizedParserStatusObject(this, this->metaObject()->indexOfSlot("componentFinalized()"));
}

void QDeclarativeBehavior::componentFinalized()
{
   Q_D(QDeclarativeBehavior);
   d->finalized = true;
}

QT_END_NAMESPACE
