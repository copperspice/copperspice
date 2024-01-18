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

#include <qdeclarativetransitionmanager_p_p.h>
#include <qdeclarativestate_p_p.h>
#include <qdeclarativestate_p.h>
#include <qdeclarativebinding_p.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativeproperty_p.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(stateChangeDebug, STATECHANGE_DEBUG);

class QDeclarativeTransitionManagerPrivate
{
 public:
   QDeclarativeTransitionManagerPrivate()
      : state(0) {}

   void applyBindings();
   typedef QList<QDeclarativeSimpleAction> SimpleActionList;
   QDeclarativeState *state;
   QDeclarativeGuard<QDeclarativeTransition> transition;
   QDeclarativeStateOperation::ActionList bindingsList;
   SimpleActionList completeList;
};

QDeclarativeTransitionManager::QDeclarativeTransitionManager()
   : d(new QDeclarativeTransitionManagerPrivate)
{
}

void QDeclarativeTransitionManager::setState(QDeclarativeState *s)
{
   d->state = s;
}

QDeclarativeTransitionManager::~QDeclarativeTransitionManager()
{
   delete d;
   d = 0;
}

void QDeclarativeTransitionManager::complete()
{
   d->applyBindings();

   for (int ii = 0; ii < d->completeList.count(); ++ii) {
      const QDeclarativeProperty &prop = d->completeList.at(ii).property();
      prop.write(d->completeList.at(ii).value());
   }

   d->completeList.clear();

   if (d->state) {
      static_cast<QDeclarativeStatePrivate *>(QObjectPrivate::get(d->state))->complete();
   }
}

void QDeclarativeTransitionManagerPrivate::applyBindings()
{
   foreach(const QDeclarativeAction & action, bindingsList) {
      if (!action.toBinding.isNull()) {
         QDeclarativePropertyPrivate::setBinding(action.property, action.toBinding.data());
      } else if (action.event) {
         if (action.reverseEvent) {
            action.event->reverse();
         } else {
            action.event->execute();
         }
      }

   }

   bindingsList.clear();
}

void QDeclarativeTransitionManager::transition(const QList<QDeclarativeAction> &list,
      QDeclarativeTransition *transition)
{
   cancel();

   QDeclarativeStateOperation::ActionList applyList = list;
   // Determine which actions are binding changes.
   foreach(const QDeclarativeAction & action, applyList) {
      if (action.toBinding) {
         d->bindingsList << action;
      }
      if (action.fromBinding) {
         QDeclarativePropertyPrivate::setBinding(action.property, 0);   // Disable current binding
      }
      if (action.event && action.event->changesBindings()) {  //### assume isReversable()?
         d->bindingsList << action;
         action.event->clearBindings();
      }
   }

   // Animated transitions need both the start and the end value for
   // each property change.  In the presence of bindings, the end values
   // are non-trivial to calculate.  As a "best effort" attempt, we first
   // apply all the property and binding changes, then read all the actual
   // final values, then roll back the changes and proceed as normal.
   //
   // This doesn't catch everything, and it might be a little fragile in
   // some cases - but whatcha going to do?

   if (!d->bindingsList.isEmpty()) {

      // Apply all the property and binding changes
      for (int ii = 0; ii < applyList.size(); ++ii) {
         const QDeclarativeAction &action = applyList.at(ii);
         if (!action.toBinding.isNull()) {
            QDeclarativePropertyPrivate::setBinding(action.property, action.toBinding.data(),
                                                    QDeclarativePropertyPrivate::BypassInterceptor | QDeclarativePropertyPrivate::DontRemoveBinding);
         } else if (!action.event) {
            QDeclarativePropertyPrivate::write(action.property, action.toValue,
                                               QDeclarativePropertyPrivate::BypassInterceptor | QDeclarativePropertyPrivate::DontRemoveBinding);
         } else if (action.event->isReversable()) {
            if (action.reverseEvent) {
               action.event->reverse(QDeclarativeActionEvent::FastForward);
            } else {
               action.event->execute(QDeclarativeActionEvent::FastForward);
            }
         }
      }

      // Read all the end values for binding changes
      for (int ii = 0; ii < applyList.size(); ++ii) {
         QDeclarativeAction *action = &applyList[ii];
         if (action->event) {
            action->event->saveTargetValues();
            continue;
         }
         const QDeclarativeProperty &prop = action->property;
         if (!action->toBinding.isNull() || !action->toValue.isValid()) {
            action->toValue = prop.read();
         }
      }

      // Revert back to the original values
      foreach(const QDeclarativeAction & action, applyList) {
         if (action.event) {
            if (action.event->isReversable()) {
               action.event->clearBindings();
               action.event->rewind();
               action.event->clearBindings();  //### shouldn't be needed
            }
            continue;
         }

         if (action.toBinding) {
            QDeclarativePropertyPrivate::setBinding(action.property, 0);   // Make sure this is disabled during the transition
         }

         QDeclarativePropertyPrivate::write(action.property, action.fromValue,
                                            QDeclarativePropertyPrivate::BypassInterceptor | QDeclarativePropertyPrivate::DontRemoveBinding);
      }
   }

   if (transition) {
      QList<QDeclarativeProperty> touched;
      d->transition = transition;
      d->transition->prepare(applyList, touched, this);

      // Modify the action list to remove actions handled in the transition
      for (int ii = 0; ii < applyList.count(); ++ii) {
         const QDeclarativeAction &action = applyList.at(ii);

         if (action.event) {

            if (action.actionDone) {
               applyList.removeAt(ii);
               --ii;
            }

         } else {

            if (touched.contains(action.property)) {
               if (action.toValue != action.fromValue)
                  d->completeList <<
                                  QDeclarativeSimpleAction(action, QDeclarativeSimpleAction::EndState);

               applyList.removeAt(ii);
               --ii;
            }

         }
      }
   }

   // Any actions remaining have not been handled by the transition and should
   // be applied immediately.  We skip applying bindings, as they are all
   // applied at the end in applyBindings() to avoid any nastiness mid
   // transition
   foreach(const QDeclarativeAction & action, applyList) {
      if (action.event && !action.event->changesBindings()) {
         if (action.event->isReversable() && action.reverseEvent) {
            action.event->reverse();
         } else {
            action.event->execute();
         }
      } else if (!action.event && !action.toBinding) {
         action.property.write(action.toValue);
      }
   }

   if (stateChangeDebug()) {
      foreach(const QDeclarativeAction & action, applyList) {
         if (action.event) {
            qWarning() << "    No transition for event:" << action.event->typeName();
         } else
            qWarning() << "    No transition for:" << action.property.object()
                       << action.property.name() << "From:" << action.fromValue
                       << "To:" << action.toValue;
      }
   }

   if (!transition) {
      d->applyBindings();
   }
}

void QDeclarativeTransitionManager::cancel()
{
   if (d->transition) {
      // ### this could potentially trigger a complete in rare circumstances
      d->transition->stop();
      d->transition = 0;
   }

   for (int i = 0; i < d->bindingsList.count(); ++i) {
      QDeclarativeAction action = d->bindingsList[i];
      if (!action.toBinding.isNull() && action.deletableToBinding) {
         QDeclarativePropertyPrivate::setBinding(action.property, 0);
         action.toBinding.data()->destroy();
         action.toBinding.clear();
         action.deletableToBinding = false;
      } else if (action.event) {
         //### what do we do here?
      }

   }
   d->bindingsList.clear();
   d->completeList.clear();
}

QT_END_NAMESPACE
