/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include <algorithm>

#include <qstatemachine.h>

#ifndef QT_NO_STATEMACHINE

#include <qstate.h>
#include <qstate_p.h>
#include <qstatemachine_p.h>

#include <qabstracttransition.h>
#include <qabstracttransition_p.h>
#include <qabstractstate.h>
#include <qabstractstate_p.h>
#include <qalgorithms.h>
#include <qfinalstate.h>
#include <qhistorystate.h>
#include <qhistorystate_p.h>
#include <qsignaltransition.h>
#include <qsignaleventgenerator_p.h>
#include <qthread_p.h>
#include <qmetaobject.h>
#include <qdebug.h>

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
#include <qeventtransition.h>
#include <qeventtransition_p.h>
#endif

#ifndef QT_NO_ANIMATION
#include <qpropertyanimation.h>
#include <qanimationgroup.h>
#include <qvariantanimation_p.h>
#endif

// messages not required  #define QSTATEMACHINE_DEBUG

QStateMachinePrivate::QStateMachinePrivate()
{
   state       = NotRunning;
   _startState = 0;

   isMachine   = true;
   processing  = false;
   stop        = false;
   processingScheduled = false;

   stopProcessingReason = EventQueueEmpty;
   error = QStateMachine::NoError;
   globalRestorePolicy = QStateMachine::DontRestoreProperties;

   m_signalEventGenerator = 0;

#ifndef QT_NO_ANIMATION
   animated = true;
#endif

}

QStateMachinePrivate::~QStateMachinePrivate()
{
   qDeleteAll(internalEventQueue);
   qDeleteAll(externalEventQueue);
}

QStateMachinePrivate *QStateMachinePrivate::get(QStateMachine *q)
{
   if (q) {
      return q->d_func();
   }
   return 0;
}

QState *QStateMachinePrivate::rootState() const
{
   return const_cast<QStateMachine *>(q_func());
}

static QEvent *cloneEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::None:
         return new QEvent(*e);
      case QEvent::Timer:
         return new QTimerEvent(*static_cast<QTimerEvent *>(e));
      default:
         Q_ASSERT_X(false, "cloneEvent()", "not implemented");
         break;
   }
   return 0;
}

const QStateMachinePrivate::Handler qt_kernel_statemachine_handler = {
   cloneEvent
};

const QStateMachinePrivate::Handler *QStateMachinePrivate::handler = &qt_kernel_statemachine_handler;

Q_CORE_EXPORT const QStateMachinePrivate::Handler *qcoreStateMachineHandler()
{
   return &qt_kernel_statemachine_handler;
}

static int indexOfDescendant(QState *s, QAbstractState *desc)
{
   QList<QAbstractState *> childStates = QStatePrivate::get(s)->childStates();
   for (int i = 0; i < childStates.size(); ++i) {
      QAbstractState *c = childStates.at(i);
      if ((c == desc) || QStateMachinePrivate::isDescendantOf(desc, c)) {
         return i;
      }
   }
   return -1;
}

bool QStateMachinePrivate::stateEntryLessThan(QAbstractState *s1, QAbstractState *s2)
{
   if (s1->parent() == s2->parent()) {
      return s1->parent()->children().indexOf(s1)
             < s2->parent()->children().indexOf(s2);
   } else if (isDescendantOf(s1, s2)) {
      return false;
   } else if (isDescendantOf(s2, s1)) {
      return true;
   } else {
      Q_ASSERT(s1->machine() != 0);
      QStateMachinePrivate *mach = QStateMachinePrivate::get(s1->machine());
      QState *lca = mach->findLCA(QList<QAbstractState *>() << s1 << s2);
      Q_ASSERT(lca != 0);
      return (indexOfDescendant(lca, s1) < indexOfDescendant(lca, s2));
   }
}

bool QStateMachinePrivate::stateExitLessThan(QAbstractState *s1, QAbstractState *s2)
{
   if (s1->parent() == s2->parent()) {
      return s1->parent()->children().indexOf(s1)
             < s2->parent()->children().indexOf(s2);
   } else if (isDescendantOf(s1, s2)) {
      return true;
   } else if (isDescendantOf(s2, s1)) {
      return false;
   } else {
      Q_ASSERT(s1->machine() != 0);
      QStateMachinePrivate *mach = QStateMachinePrivate::get(s1->machine());
      QState *lca = mach->findLCA(QList<QAbstractState *>() << s1 << s2);
      Q_ASSERT(lca != 0);
      return (indexOfDescendant(lca, s1) < indexOfDescendant(lca, s2));
   }
}

QState *QStateMachinePrivate::findLCA(const QList<QAbstractState *> &states) const
{
   if (states.isEmpty()) {
      return 0;
   }
   QList<QState *> ancestors = properAncestors(states.at(0), rootState()->parentState());
   for (int i = 0; i < ancestors.size(); ++i) {
      QState *anc = ancestors.at(i);
      bool ok = true;
      for (int j = states.size() - 1; (j > 0) && ok; --j) {
         const QAbstractState *s = states.at(j);
         if (!isDescendantOf(s, anc)) {
            ok = false;
         }
      }
      if (ok) {
         return anc;
      }
   }
   return 0;
}

bool QStateMachinePrivate::isPreempted(const QAbstractState *s, const QSet<QAbstractTransition *> &transitions) const
{
   QSet<QAbstractTransition *>::const_iterator it;
   for (it = transitions.constBegin(); it != transitions.constEnd(); ++it) {
      QAbstractTransition *t = *it;
      QList<QAbstractState *> lst = t->targetStates();
      if (!lst.isEmpty()) {
         lst.prepend(t->sourceState());
         QAbstractState *lca = findLCA(lst);
         if (isDescendantOf(s, lca)) {
#ifdef QSTATEMACHINE_DEBUG
            qDebug() << q_func() << ':' << transitions << "preempts selection of a transition from"
                     << s << "because" << s << "is a descendant of" << lca;
#endif
            return true;
         }
      }
   }
   return false;
}

QSet<QAbstractTransition *> QStateMachinePrivate::selectTransitions(QEvent *event) const
{
   Q_Q(const QStateMachine);

   QSet<QAbstractTransition *> enabledTransitions;
   QSet<QAbstractState *>::const_iterator it;
   const_cast<QStateMachine *>(q)->beginSelectTransitions(event);

   for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
      QAbstractState *state = *it;

      if (!isAtomic(state)) {
         continue;
      }

      if (isPreempted(state, enabledTransitions)) {
         continue;
      }

      QList<QState *> lst = properAncestors(state, rootState()->parentState());
      if (QState *grp = toStandardState(state)) {
         lst.prepend(grp);
      }
      bool found = false;

      for (int j = 0; (j < lst.size()) && !found; ++j) {
         QState *s = lst.at(j);
         QList<QAbstractTransition *> transitions = QStatePrivate::get(s)->transitions();

         for (int k = 0; k < transitions.size(); ++k) {
            QAbstractTransition *t = transitions.at(k);

            if (QAbstractTransitionPrivate::get(t)->callEventTest(event)) {

#ifdef QSTATEMACHINE_DEBUG
               qDebug() << q << ": selecting transition" << t;
#endif
               enabledTransitions.insert(t);
               found = true;
               break;
            }
         }
      }
   }
   const_cast<QStateMachine *>(q)->endSelectTransitions(event);
   return enabledTransitions;
}

void QStateMachinePrivate::microstep(QEvent *event, const QList<QAbstractTransition *> &enabledTransitions)
{
#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q_func() << ": begin microstep( enabledTransitions:" << enabledTransitions << ')';
   qDebug() << q_func() << ": configuration before exiting states:" << configuration;
#endif

   QList<QAbstractState *> exitedStates = exitStates(event, enabledTransitions);

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q_func() << ": configuration after exiting states:" << configuration;
#endif

   executeTransitionContent(event, enabledTransitions);
   QList<QAbstractState *> enteredStates = enterStates(event, enabledTransitions);

#ifndef QT_NO_PROPERTIES
   applyProperties(enabledTransitions, exitedStates, enteredStates);
#endif

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q_func() << ": configuration after entering states:" << configuration;
   qDebug() << q_func() << ": end microstep";
#endif
}

QList<QAbstractState *> QStateMachinePrivate::exitStates(QEvent *event,
      const QList<QAbstractTransition *> &enabledTransitions)
{
   //  qDebug() << "exitStates(" << enabledTransitions << ')';
   QSet<QAbstractState *> statesToExit;
   //  QSet<QAbstractState*> statesToSnapshot;

   for (int i = 0; i < enabledTransitions.size(); ++i) {
      QAbstractTransition *t = enabledTransitions.at(i);
      QList<QAbstractState *> lst = t->targetStates();
      if (lst.isEmpty()) {
         continue;
      }
      lst.prepend(t->sourceState());
      QAbstractState *lca = findLCA(lst);
      if (lca == 0) {
         setError(QStateMachine::NoCommonAncestorForTransitionError, t->sourceState());
         lst = pendingErrorStates.toList();
         lst.prepend(t->sourceState());

         lca = findLCA(lst);
         Q_ASSERT(lca != 0);
      }

      {
         QSet<QAbstractState *>::const_iterator it;
         for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
            QAbstractState *s = *it;
            if (isDescendantOf(s, lca)) {
               statesToExit.insert(s);
            }
         }
      }
   }
   QList<QAbstractState *> statesToExit_sorted = statesToExit.toList();
   std::sort(statesToExit_sorted.begin(), statesToExit_sorted.end(), stateExitLessThan);

   for (int i = 0; i < statesToExit_sorted.size(); ++i) {
      QAbstractState *s = statesToExit_sorted.at(i);
      if (QState *grp = toStandardState(s)) {
         QList<QHistoryState *> hlst = QStatePrivate::get(grp)->historyStates();
         for (int j = 0; j < hlst.size(); ++j) {
            QHistoryState *h = hlst.at(j);
            QHistoryStatePrivate::get(h)->configuration.clear();
            QSet<QAbstractState *>::const_iterator it;
            for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
               QAbstractState *s0 = *it;
               if (QHistoryStatePrivate::get(h)->historyType == QHistoryState::DeepHistory) {
                  if (isAtomic(s0) && isDescendantOf(s0, s)) {
                     QHistoryStatePrivate::get(h)->configuration.append(s0);
                  }
               } else if (s0->parentState() == s) {
                  QHistoryStatePrivate::get(h)->configuration.append(s0);
               }
            }
#ifdef QSTATEMACHINE_DEBUG
            qDebug() << q_func() << ": recorded" << ((QHistoryStatePrivate::get(h)->historyType == QHistoryState::DeepHistory) ?
                     "deep" : "shallow")
                     << "history for" << s << "in" << h << ':' << QHistoryStatePrivate::get(h)->configuration;
#endif
         }
      }
   }
   for (int i = 0; i < statesToExit_sorted.size(); ++i) {
      QAbstractState *s = statesToExit_sorted.at(i);

#ifdef QSTATEMACHINE_DEBUG
      qDebug() << q_func() << ": exiting" << s;
#endif
      QAbstractStatePrivate::get(s)->callOnExit(event);
      configuration.remove(s);
      QAbstractStatePrivate::get(s)->emitExited();
   }
   return statesToExit_sorted;
}

void QStateMachinePrivate::executeTransitionContent(QEvent *event,
      const QList<QAbstractTransition *> &enabledTransitions)
{
   for (int i = 0; i < enabledTransitions.size(); ++i) {
      QAbstractTransition *t = enabledTransitions.at(i);

#ifdef QSTATEMACHINE_DEBUG
      qDebug() << q_func() << ": triggering" << t;
#endif

      QAbstractTransitionPrivate::get(t)->callOnTransition(event);
      QAbstractTransitionPrivate::get(t)->emitTriggered();
   }
}

QList<QAbstractState *> QStateMachinePrivate::enterStates(QEvent *event,
      const QList<QAbstractTransition *> &enabledTransitions)
{

#ifdef QSTATEMACHINE_DEBUG
   Q_Q(QStateMachine);

   qDebug() << "enterStates(" << enabledTransitions << ')';
#endif

   QSet<QAbstractState *> statesToEnter;
   QSet<QAbstractState *> statesForDefaultEntry;

   if (pendingErrorStates.isEmpty()) {
      for (int i = 0; i < enabledTransitions.size(); ++i) {
         QAbstractTransition *t = enabledTransitions.at(i);
         QList<QAbstractState *> lst = t->targetStates();
         if (lst.isEmpty()) {
            continue;
         }
         lst.prepend(t->sourceState());
         QState *lca = findLCA(lst);
         for (int j = 1; j < lst.size(); ++j) {
            QAbstractState *s = lst.at(j);
            addStatesToEnter(s, lca, statesToEnter, statesForDefaultEntry);
            if (isParallel(lca)) {
               QList<QAbstractState *> lcac = QStatePrivate::get(lca)->childStates();

               for (QAbstractState * child : lcac) {
                  if (!statesToEnter.contains(child)) {
                     addStatesToEnter(child, lca, statesToEnter, statesForDefaultEntry);
                  }
               }
            }
         }
      }
   }

   // Did an error occur while selecting transitions? Then we enter the error state.
   if (!pendingErrorStates.isEmpty()) {
      statesToEnter.clear();
      statesToEnter = pendingErrorStates;
      statesForDefaultEntry = pendingErrorStatesForDefaultEntry;
      pendingErrorStates.clear();
      pendingErrorStatesForDefaultEntry.clear();
   }

   QList<QAbstractState *> statesToEnter_sorted = statesToEnter.toList();
   std::sort(statesToEnter_sorted.begin(), statesToEnter_sorted.end(), stateEntryLessThan);

   for (int i = 0; i < statesToEnter_sorted.size(); ++i) {
      QAbstractState *s = statesToEnter_sorted.at(i);

#ifdef QSTATEMACHINE_DEBUG
      qDebug() << q << ": entering" << s;
#endif
      configuration.insert(s);
      registerTransitions(s);
      QAbstractStatePrivate::get(s)->callOnEntry(event);
      QAbstractStatePrivate::get(s)->emitEntered();

      if (statesForDefaultEntry.contains(s)) {
         // ### executeContent(s.initial.transition.children())
      }

      if (isFinal(s)) {
         QState *parent = s->parentState();
         if (parent) {
            if (parent != rootState()) {
#ifdef QSTATEMACHINE_DEBUG
               qDebug() << q << ": emitting finished signal for" << parent;
#endif
               QStatePrivate::get(parent)->emitFinished();
            }
            QState *grandparent = parent->parentState();
            if (grandparent && isParallel(grandparent)) {
               bool allChildStatesFinal = true;
               QList<QAbstractState *> childStates = QStatePrivate::get(grandparent)->childStates();
               for (int j = 0; j < childStates.size(); ++j) {
                  QAbstractState *cs = childStates.at(j);
                  if (!isInFinalState(cs)) {
                     allChildStatesFinal = false;
                     break;
                  }
               }
               if (allChildStatesFinal && (grandparent != rootState())) {
#ifdef QSTATEMACHINE_DEBUG
                  qDebug() << q << ": emitting finished signal for" << grandparent;
#endif
                  QStatePrivate::get(grandparent)->emitFinished();
               }
            }
         }
      }
   }
   {
      QSet<QAbstractState *>::const_iterator it;
      for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
         if (isFinal(*it) && (*it)->parentState() == rootState()) {
            processing = false;
            stopProcessingReason = Finished;
            break;
         }
      }
   }
   //    qDebug() << "configuration:" << configuration.toList();
   return statesToEnter_sorted;
}

void QStateMachinePrivate::addStatesToEnter(QAbstractState *s, QState *root,
      QSet<QAbstractState *> &statesToEnter,
      QSet<QAbstractState *> &statesForDefaultEntry)
{
   if (QHistoryState *h = toHistoryState(s)) {
      QList<QAbstractState *> hconf = QHistoryStatePrivate::get(h)->configuration;

      if (!hconf.isEmpty()) {
         for (int k = 0; k < hconf.size(); ++k) {
            QAbstractState *s0 = hconf.at(k);
            addStatesToEnter(s0, root, statesToEnter, statesForDefaultEntry);
         }

#ifdef QSTATEMACHINE_DEBUG
         qDebug() << q_func() << ": restoring"
                  << ((QHistoryStatePrivate::get(h)->historyType == QHistoryState::DeepHistory) ? "deep" : "shallow")
                  << "history from" << s << ':' << hconf;
#endif

      } else {
         QList<QAbstractState *> hlst;

         if (QHistoryStatePrivate::get(h)->defaultState) {
            hlst.append(QHistoryStatePrivate::get(h)->defaultState);
         }

         if (hlst.isEmpty()) {
            setError(QStateMachine::NoDefaultStateInHistoryStateError, h);

         } else {
            for (int k = 0; k < hlst.size(); ++k) {
               QAbstractState *s0 = hlst.at(k);
               addStatesToEnter(s0, root, statesToEnter, statesForDefaultEntry);
            }
#ifdef QSTATEMACHINE_DEBUG
            qDebug() << q_func() << ": initial history targets for" << s << ':' << hlst;
#endif
         }
      }
   } else {
      if (s == rootState()) {
         // Error has already been set by exitStates().
         Q_ASSERT(error != QStateMachine::NoError);
         return;
      }
      statesToEnter.insert(s);
      if (isParallel(s)) {
         QState *grp = toStandardState(s);
         QList<QAbstractState *> lst = QStatePrivate::get(grp)->childStates();
         for (int i = 0; i < lst.size(); ++i) {
            QAbstractState *child = lst.at(i);
            addStatesToEnter(child, grp, statesToEnter, statesForDefaultEntry);
         }
      } else if (isCompound(s)) {
         statesForDefaultEntry.insert(s);
         QState *grp = toStandardState(s);
         QAbstractState *initial = grp->initialState();
         if (initial != 0) {
            Q_ASSERT(initial->machine() == q_func());
            addStatesToEnter(initial, grp, statesToEnter, statesForDefaultEntry);
         } else {
            setError(QStateMachine::NoInitialStateError, grp);
            return;
         }
      }
      QList<QState *> ancs = properAncestors(s, root);
      for (int i = 0; i < ancs.size(); ++i) {
         QState *anc = ancs.at(i);
         if (!anc->parentState()) {
            continue;
         }
         statesToEnter.insert(anc);
         if (isParallel(anc)) {
            QList<QAbstractState *> lst = QStatePrivate::get(anc)->childStates();
            for (int j = 0; j < lst.size(); ++j) {
               QAbstractState *child = lst.at(j);
               bool hasDescendantInList = false;
               QSet<QAbstractState *>::const_iterator it;
               for (it = statesToEnter.constBegin(); it != statesToEnter.constEnd(); ++it) {
                  if (isDescendantOf(*it, child)) {
                     hasDescendantInList = true;
                     break;
                  }
               }
               if (!hasDescendantInList) {
                  addStatesToEnter(child, anc, statesToEnter, statesForDefaultEntry);
               }
            }
         }
      }
   }
}

#ifndef QT_NO_PROPERTIES

void QStateMachinePrivate::applyProperties(const QList<QAbstractTransition *> &transitionList,
      const QList<QAbstractState *> &exitedStates,
      const QList<QAbstractState *> &enteredStates)
{
#ifdef QT_NO_ANIMATION
   Q_UNUSED(transitionList);
   Q_UNUSED(exitedStates);
#else
   Q_Q(QStateMachine);
#endif

   // Process the property assignments of the entered states.
   QHash<QAbstractState *, QList<QPropertyAssignment> > propertyAssignmentsForState;
   QHash<RestorableId, QVariant> pendingRestorables = registeredRestorables;
   for (int i = 0; i < enteredStates.size(); ++i) {
      QState *s = toStandardState(enteredStates.at(i));
      if (!s) {
         continue;
      }

      QList<QPropertyAssignment> assignments = QStatePrivate::get(s)->propertyAssignments;
      for (int j = 0; j < assignments.size(); ++j) {
         const QPropertyAssignment &assn = assignments.at(j);
         if (globalRestorePolicy == QStateMachine::RestoreProperties) {
            registerRestorable(assn.object, assn.propertyName);
         }
         pendingRestorables.remove(RestorableId(assn.object, assn.propertyName));
         propertyAssignmentsForState[s].append(assn);
      }

      // Remove pending restorables for all parent states to avoid restoring properties
      // before the state that assigned them is exited. If state does not explicitly
      // assign a property which is assigned by the parent, it inherits the parent's assignment.
      QState *parentState = s;
      while ((parentState = parentState->parentState()) != 0) {
         assignments = QStatePrivate::get(parentState)->propertyAssignments;
         for (int j = 0; j < assignments.size(); ++j) {
            const QPropertyAssignment &assn = assignments.at(j);
            int c = pendingRestorables.remove(RestorableId(assn.object, assn.propertyName));
            if (c > 0) {
               propertyAssignmentsForState[s].append(assn);
            }
         }
      }
   }
   if (!pendingRestorables.isEmpty()) {
      QAbstractState *s;
      if (!enteredStates.isEmpty()) {
         s = enteredStates.last();   // ### handle if parallel
      } else {
         s = 0;
      }
      propertyAssignmentsForState[s] << restorablesToPropertyList(pendingRestorables);
   }

#ifndef QT_NO_ANIMATION
   // Gracefully terminate playing animations for states that are exited
   for (int i = 0; i < exitedStates.size(); ++i) {
      QAbstractState *s = exitedStates.at(i);
      QList<QAbstractAnimation *> animations = animationsForState.take(s);

      for (int j = 0; j < animations.size(); ++j) {
         QAbstractAnimation *anim = animations.at(j);
         QObject::disconnect(anim, SIGNAL(finished()), q, SLOT(_q_animationFinished()));
         stateForAnimation.remove(anim);

         // Stop the (top-level) animation.
         // ### Stopping nested animation has weird behavior.
         QAbstractAnimation *topLevelAnim = anim;
         while (QAnimationGroup *group = topLevelAnim->group()) {
            topLevelAnim = group;
         }
         topLevelAnim->stop();

         if (resetAnimationEndValues.contains(anim)) {
            qobject_cast<QVariantAnimation *>(anim)->setEndValue(QVariant()); // ### generalize
            resetAnimationEndValues.remove(anim);
         }
         QPropertyAssignment assn = propertyForAnimation.take(anim);
         Q_ASSERT(assn.object != 0);

         // If there is no property assignment that sets this property,
         // set the property to its target value.

         bool found = false;
         QHash<QAbstractState *, QList<QPropertyAssignment> >::const_iterator it;
         for (it = propertyAssignmentsForState.constBegin(); it != propertyAssignmentsForState.constEnd(); ++it) {
            const QList<QPropertyAssignment> &assignments = it.value();
            for (int k = 0; k < assignments.size(); ++k) {
               if ((assignments.at(k).object == assn.object)
                     && (assignments.at(k).propertyName == assn.propertyName)) {
                  found = true;
                  break;
               }
            }
         }
         if (!found) {
            assn.object->setProperty(assn.propertyName, assn.value);
         }
      }
   }

   // Find the animations to use for the state change.
   QList<QAbstractAnimation *> selectedAnimations;
   if (animated) {
      for (int i = 0; i < transitionList.size(); ++i) {
         QAbstractTransition *transition = transitionList.at(i);

         selectedAnimations << transition->animations();
         selectedAnimations << defaultAnimationsForSource.values(transition->sourceState());

         QList<QAbstractState *> targetStates = transition->targetStates();
         for (int j = 0; j < targetStates.size(); ++j) {
            selectedAnimations << defaultAnimationsForTarget.values(targetStates.at(j));
         }
      }
      selectedAnimations << defaultAnimations;
   }

   // Initialize animations from property assignments.
   for (int i = 0; i < selectedAnimations.size(); ++i) {
      QAbstractAnimation *anim = selectedAnimations.at(i);
      QHash<QAbstractState *, QList<QPropertyAssignment> >::iterator it;

      for (it = propertyAssignmentsForState.begin(); it != propertyAssignmentsForState.end(); ) {
         QList<QPropertyAssignment>::iterator it2;
         QAbstractState *s = it.key();
         QList<QPropertyAssignment> &assignments = it.value();

         for (it2 = assignments.begin(); it2 != assignments.end(); ) {
            QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *> > ret;
            ret = initializeAnimation(anim, *it2);
            QList<QAbstractAnimation *> handlers = ret.first;

            if (! handlers.isEmpty()) {
               for (int j = 0; j < handlers.size(); ++j) {
                  QAbstractAnimation *a = handlers.at(j);
                  propertyForAnimation.insert(a, *it2);
                  stateForAnimation.insert(a, s);
                  animationsForState[s].append(a);

                  // ### connect to just the top-level animation?
                  QObject::connect(a, SIGNAL(finished()), q, SLOT(_q_animationFinished()), Qt::UniqueConnection);
               }

               it2 = assignments.erase(it2);

            } else {
               ++it2;
            }

            for (int j = 0; j < ret.second.size(); ++j) {
               resetAnimationEndValues.insert(ret.second.at(j));
            }
         }
         if (assignments.isEmpty()) {
            it = propertyAssignmentsForState.erase(it);
         } else {
            ++it;
         }
      }
      // We require that at least one animation is valid.
      // ### generalize
      QList<QVariantAnimation *> variantAnims = anim->findChildren<QVariantAnimation *>();
      if (QVariantAnimation *va = qobject_cast<QVariantAnimation *>(anim)) {
         variantAnims.append(va);
      }

      bool hasValidEndValue = false;
      for (int j = 0; j < variantAnims.size(); ++j) {
         if (variantAnims.at(j)->endValue().isValid()) {
            hasValidEndValue = true;
            break;
         }
      }

      if (hasValidEndValue) {
         if (anim->state() == QAbstractAnimation::Running) {
            // The animation is still running. This can happen if the
            // animation is a group, and one of its children just finished,
            // and that caused a state to emit its propertiesAssigned() signal, and
            // that triggered a transition in the machine.
            // Just stop the animation so it is correctly restarted again.
            anim->stop();
         }
         anim->start();
      }
   }
#endif // !QT_NO_ANIMATION

   // Immediately set the properties that are not animated.
   {
      QHash<QAbstractState *, QList<QPropertyAssignment> >::const_iterator it;
      for (it = propertyAssignmentsForState.constBegin(); it != propertyAssignmentsForState.constEnd(); ++it) {
         const QList<QPropertyAssignment> &assignments = it.value();
         for (int i = 0; i < assignments.size(); ++i) {
            const QPropertyAssignment &assn = assignments.at(i);
            assn.object->setProperty(assn.propertyName, assn.value);
         }
      }
   }

   // Emit propertiesAssigned signal for entered states that have no animated properties.
   for (int i = 0; i < enteredStates.size(); ++i) {
      QState *s = toStandardState(enteredStates.at(i));
      if (s
#ifndef QT_NO_ANIMATION
            && !animationsForState.contains(s)
#endif
         ) {
         QStatePrivate::get(s)->emitPropertiesAssigned();
      }
   }
}

#endif // QT_NO_PROPERTIES

bool QStateMachinePrivate::isFinal(const QAbstractState *s)
{
   return s && (QAbstractStatePrivate::get(s)->stateType == QAbstractStatePrivate::FinalState);
}

bool QStateMachinePrivate::isParallel(const QAbstractState *s)
{
   const QState *ss = toStandardState(s);
   return ss && (QStatePrivate::get(ss)->childMode == QState::ParallelStates);
}

bool QStateMachinePrivate::isCompound(const QAbstractState *s) const
{
   const QState *group = toStandardState(s);
   if (!group) {
      return false;
   }
   bool isMachine = QStatePrivate::get(group)->isMachine;
   // Don't treat the machine as compound if it's a sub-state of this machine
   if (isMachine && (group != rootState())) {
      return false;
   }
   return (!isParallel(group) && !QStatePrivate::get(group)->childStates().isEmpty())
          || isMachine;
}

bool QStateMachinePrivate::isAtomic(const QAbstractState *s) const
{
   const QState *ss = toStandardState(s);
   return (ss && QStatePrivate::get(ss)->childStates().isEmpty())
          || isFinal(s)
          // Treat the machine as atomic if it's a sub-state of this machine
          || (ss && QStatePrivate::get(ss)->isMachine && (ss != rootState()));
}


bool QStateMachinePrivate::isDescendantOf(const QAbstractState *state, const QAbstractState *other)
{
   Q_ASSERT(state != 0);
   for (QAbstractState *s = state->parentState(); s != 0; s = s->parentState()) {
      if (s == other) {
         return true;
      }
   }
   return false;
}

QList<QState *> QStateMachinePrivate::properAncestors(const QAbstractState *state, const QState *upperBound)
{
   Q_ASSERT(state != 0);
   QList<QState *> result;
   for (QState *s = state->parentState(); s && s != upperBound; s = s->parentState()) {
      result.append(s);
   }
   return result;
}

QState *QStateMachinePrivate::toStandardState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::StandardState)) {
      return static_cast<QState *>(state);
   }
   return 0;
}

const QState *QStateMachinePrivate::toStandardState(const QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::StandardState)) {
      return static_cast<const QState *>(state);
   }
   return 0;
}

QFinalState *QStateMachinePrivate::toFinalState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::FinalState)) {
      return static_cast<QFinalState *>(state);
   }
   return 0;
}

QHistoryState *QStateMachinePrivate::toHistoryState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::HistoryState)) {
      return static_cast<QHistoryState *>(state);
   }
   return 0;
}

bool QStateMachinePrivate::isInFinalState(QAbstractState *s) const
{
   if (isCompound(s)) {
      QState *grp = toStandardState(s);
      QList<QAbstractState *> lst = QStatePrivate::get(grp)->childStates();
      for (int i = 0; i < lst.size(); ++i) {
         QAbstractState *cs = lst.at(i);
         if (isFinal(cs) && configuration.contains(cs)) {
            return true;
         }
      }
      return false;
   } else if (isParallel(s)) {
      QState *grp = toStandardState(s);
      QList<QAbstractState *> lst = QStatePrivate::get(grp)->childStates();
      for (int i = 0; i < lst.size(); ++i) {
         QAbstractState *cs = lst.at(i);
         if (!isInFinalState(cs)) {
            return false;
         }
      }
      return true;
   } else {
      return false;
   }
}

#ifndef QT_NO_PROPERTIES

void QStateMachinePrivate::registerRestorable(QObject *object, const QByteArray &propertyName)
{
   RestorableId id(object, propertyName);
   if (!registeredRestorables.contains(id)) {
      registeredRestorables.insert(id, object->property(propertyName));
   }
}

QList<QPropertyAssignment> QStateMachinePrivate::restorablesToPropertyList(const QHash<RestorableId, QVariant>
      &restorables) const
{
   QList<QPropertyAssignment> result;
   QHash<RestorableId, QVariant>::const_iterator it;
   for (it = restorables.constBegin(); it != restorables.constEnd(); ++it) {
      //        qDebug() << "restorable:" << it.key().first << it.key().second << it.value();
      result.append(QPropertyAssignment(it.key().first, it.key().second, it.value(), /*explicitlySet=*/false));
   }
   return result;
}

/*!
   \internal
   Returns true if the variable with the given \a id has been registered for restoration.
*/
bool QStateMachinePrivate::hasRestorable(QObject *object, const QByteArray &propertyName) const
{
   return registeredRestorables.contains(RestorableId(object, propertyName));
}

QVariant QStateMachinePrivate::restorableValue(QObject *object, const QByteArray &propertyName) const
{
   return registeredRestorables.value(RestorableId(object, propertyName), QVariant());
}


/*!
   \internal
    Unregisters the variable identified by \a id
*/
void QStateMachinePrivate::unregisterRestorable(QObject *object, const QByteArray &propertyName)
{
   //  qDebug() << "unregisterRestorable(" << object << propertyName << ')';
   RestorableId id(object, propertyName);
   registeredRestorables.remove(id);
}

#endif // QT_NO_PROPERTIES

QAbstractState *QStateMachinePrivate::findErrorState(QAbstractState *context)
{
   // Find error state recursively in parent hierarchy if not set explicitly for context state
   QAbstractState *errorState = 0;
   if (context != 0) {
      QState *s = toStandardState(context);
      if (s != 0) {
         errorState = s->errorState();
      }

      if (errorState == 0) {
         errorState = findErrorState(context->parentState());
      }
   }

   return errorState;
}

void QStateMachinePrivate::setError(QStateMachine::Error errorCode, QAbstractState *currentContext)
{
   Q_Q(QStateMachine);

   error = errorCode;
   switch (errorCode) {
      case QStateMachine::NoInitialStateError:
         Q_ASSERT(currentContext != 0);

         errorString = QStateMachine::tr("Missing initial state in compound state '%1'")
                       .arg(currentContext->objectName());

         break;
      case QStateMachine::NoDefaultStateInHistoryStateError:
         Q_ASSERT(currentContext != 0);

         errorString = QStateMachine::tr("Missing default state in history state '%1'")
                       .arg(currentContext->objectName());
         break;

      case QStateMachine::NoCommonAncestorForTransitionError:
         Q_ASSERT(currentContext != 0);

         errorString = QStateMachine::tr("No common ancestor for targets and source of transition from state '%1'")
                       .arg(currentContext->objectName());
         break;
      default:
         errorString = QStateMachine::tr("Unknown error");
   };

   pendingErrorStates.clear();
   pendingErrorStatesForDefaultEntry.clear();

   QAbstractState *currentErrorState = findErrorState(currentContext);

   // Avoid infinite loop if the error state itself has an error
   if (currentContext == currentErrorState) {
      currentErrorState = 0;
   }

   Q_ASSERT(currentErrorState != rootState());

   if (currentErrorState != 0) {
      QState *lca = findLCA(QList<QAbstractState *>() << currentErrorState << currentContext);
      addStatesToEnter(currentErrorState, lca, pendingErrorStates, pendingErrorStatesForDefaultEntry);
   } else {
      qWarning("Unrecoverable error detected in running state machine: %s",
               qPrintable(errorString));
      q->stop();
   }
}

#ifndef QT_NO_ANIMATION

QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *> >
QStateMachinePrivate::initializeAnimation(QAbstractAnimation *abstractAnimation,
      const QPropertyAssignment &prop)
{
   QList<QAbstractAnimation *> handledAnimations;
   QList<QAbstractAnimation *> localResetEndValues;
   QAnimationGroup *group = qobject_cast<QAnimationGroup *>(abstractAnimation);
   if (group) {
      for (int i = 0; i < group->animationCount(); ++i) {
         QAbstractAnimation *animationChild = group->animationAt(i);
         QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *> > ret;
         ret = initializeAnimation(animationChild, prop);
         handledAnimations << ret.first;
         localResetEndValues << ret.second;
      }
   } else {
      QPropertyAnimation *animation = qobject_cast<QPropertyAnimation *>(abstractAnimation);
      if (animation != 0
            && prop.object == animation->targetObject()
            && prop.propertyName == animation->propertyName()) {

         // Only change end value if it is undefined
         if (!animation->endValue().isValid()) {
            animation->setEndValue(prop.value);
            localResetEndValues.append(animation);
         }
         handledAnimations.append(animation);
      }
   }
   return qMakePair(handledAnimations, localResetEndValues);
}

void QStateMachinePrivate::_q_animationFinished()
{
   Q_Q(QStateMachine);

   QAbstractAnimation *anim = qobject_cast<QAbstractAnimation *>(q->sender());
   Q_ASSERT(anim != 0);
   QObject::disconnect(anim, SIGNAL(finished()), q, SLOT(_q_animationFinished()));

   if (resetAnimationEndValues.contains(anim)) {
      qobject_cast<QVariantAnimation *>(anim)->setEndValue(QVariant()); // ### generalize
      resetAnimationEndValues.remove(anim);
   }

#ifndef QT_NO_PROPERTIES
   // Set the final property value.
   QPropertyAssignment assn = propertyForAnimation.take(anim);
   Q_ASSERT(assn.object != 0);

   assn.object->setProperty(assn.propertyName, assn.value);

   if (!assn.explicitlySet) {
      unregisterRestorable(assn.object, assn.propertyName);
   }
#endif

   QAbstractState *state = stateForAnimation.take(anim);
   Q_ASSERT(state != 0);
   QHash<QAbstractState *, QList<QAbstractAnimation *> >::iterator it;
   it = animationsForState.find(state);
   Q_ASSERT(it != animationsForState.end());
   QList<QAbstractAnimation *> &animations = it.value();
   animations.removeOne(anim);
   if (animations.isEmpty()) {
      animationsForState.erase(it);
      QStatePrivate::get(toStandardState(state))->emitPropertiesAssigned();
   }
}

#endif // !QT_NO_ANIMATION

namespace {

class StartState : public QState
{
 public:
   StartState(QState *parent)
      : QState(parent) {}
 protected:
   void onEntry(QEvent *) override {}
   void onExit(QEvent *) override {}
};

class InitialTransition : public QAbstractTransition
{
 public:
   InitialTransition(QAbstractState *target)
      : QAbstractTransition() {
      setTargetState(target);
   }
 protected:
   virtual bool eventTest(QEvent *) override {
      return true;
   }

   virtual void onTransition(QEvent *) override {}
};

} // namespace

QState *QStateMachinePrivate::startState()
{
   Q_Q(QStateMachine);
   if (_startState == 0) {
      _startState = new StartState(q);
   }
   return _startState;
}

void QStateMachinePrivate::removeStartState()
{
   delete _startState;
   _startState = 0;
}

void QStateMachinePrivate::clearHistory()
{
   Q_Q(QStateMachine);
   QList<QHistoryState *> historyStates = q->findChildren<QHistoryState *>();
   for (int i = 0; i < historyStates.size(); ++i) {
      QHistoryState *h = historyStates.at(i);
      QHistoryStatePrivate::get(h)->configuration.clear();
   }
}

void QStateMachinePrivate::_q_start()
{
   Q_Q(QStateMachine);
   Q_ASSERT(state == Starting);
   Q_ASSERT(rootState() != 0);

   QAbstractState *initial = rootState()->initialState();
   configuration.clear();

   qDeleteAll(internalEventQueue);
   internalEventQueue.clear();
   qDeleteAll(externalEventQueue);

   externalEventQueue.clear();
   clearHistory();

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q << ": starting";
#endif

   state = Running;
   processingScheduled = true; // we call _q_process() below
   emit q->started();

   QState *start = startState();
   Q_ASSERT(start != 0);

   QList<QAbstractTransition *> transitions = QStatePrivate::get(start)->transitions();

   // If a transition has already been added, then we skip this step, as the
   // initial transition in that case has been overridden.
   if (transitions.isEmpty()) {
      QAbstractTransition *initialTransition = new InitialTransition(initial);
      start->addTransition(initialTransition);
      transitions.append(initialTransition);
   }

   QEvent nullEvent(QEvent::None);
   executeTransitionContent(&nullEvent, transitions);
   QList<QAbstractState *> enteredStates = enterStates(&nullEvent, transitions);
#ifndef QT_NO_PROPERTIES
   applyProperties(transitions, QList<QAbstractState *>() << start, enteredStates);
#endif
   removeStartState();

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q << ": initial configuration:" << configuration;
#endif
   _q_process();
}

void QStateMachinePrivate::_q_process()
{
   Q_Q(QStateMachine);
   Q_ASSERT(state == Running);
   Q_ASSERT(!processing);

   processing = true;
   processingScheduled = false;

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q << ": starting the event processing loop";
#endif

   while (processing) {
      if (stop) {
         processing = false;
         break;
      }
      QSet<QAbstractTransition *> enabledTransitions;
      QEvent *e = new QEvent(QEvent::None);
      enabledTransitions = selectTransitions(e);
      if (enabledTransitions.isEmpty()) {
         delete e;
         e = 0;
      }

      if (enabledTransitions.isEmpty() && ((e = dequeueInternalEvent()) != 0)) {

#ifdef QSTATEMACHINE_DEBUG
         qDebug() << q << ": dequeued internal event" << e << "of type" << e->type();
#endif
         enabledTransitions = selectTransitions(e);
         if (enabledTransitions.isEmpty()) {
            delete e;
            e = 0;
         }
      }
      if (enabledTransitions.isEmpty()) {
         if ((e = dequeueExternalEvent()) != 0) {

#ifdef QSTATEMACHINE_DEBUG
            qDebug() << q << ": dequeued external event" << e << "of type" << e->type();
#endif
            enabledTransitions = selectTransitions(e);

            if (enabledTransitions.isEmpty()) {
               delete e;
               e = 0;
            }
         } else {
            if (isInternalEventQueueEmpty()) {
               processing = false;
               stopProcessingReason = EventQueueEmpty;
            }
         }
      }

      if (!enabledTransitions.isEmpty()) {
         q->beginMicrostep(e);
         microstep(e, enabledTransitions.toList());
         q->endMicrostep(e);
      }

#ifdef QSTATEMACHINE_DEBUG
      else {
         qDebug() << q << ": no transitions enabled";
      }
#endif

      delete e;
   }

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q << ": finished the event processing loop";
#endif

   if (stop) {
      stop = false;
      stopProcessingReason = Stopped;
   }

   switch (stopProcessingReason) {
      case EventQueueEmpty:
         break;

      case Finished:
         state = NotRunning;
         cancelAllDelayedEvents();
         unregisterAllTransitions();
         emit q->finished();
         break;

      case Stopped:
         state = NotRunning;
         cancelAllDelayedEvents();
         unregisterAllTransitions();
         emit q->stopped();
         break;
   }
}

void QStateMachine::_q_start()
{
   Q_D(QStateMachine);
   d->_q_start();
}

void QStateMachine::_q_process()
{
   Q_D(QStateMachine);
   d->_q_process();
}

void QStateMachine::_q_animationFinished()
{
   Q_D(QStateMachine);
   d->_q_animationFinished();
}

void QStateMachinePrivate::postInternalEvent(QEvent *e)
{
   QMutexLocker locker(&internalEventMutex);
   internalEventQueue.append(e);
}

void QStateMachinePrivate::postExternalEvent(QEvent *e)
{
   QMutexLocker locker(&externalEventMutex);
   externalEventQueue.append(e);
}

QEvent *QStateMachinePrivate::dequeueInternalEvent()
{
   QMutexLocker locker(&internalEventMutex);
   if (internalEventQueue.isEmpty()) {
      return 0;
   }
   return internalEventQueue.takeFirst();
}

QEvent *QStateMachinePrivate::dequeueExternalEvent()
{
   QMutexLocker locker(&externalEventMutex);
   if (externalEventQueue.isEmpty()) {
      return 0;
   }
   return externalEventQueue.takeFirst();
}

bool QStateMachinePrivate::isInternalEventQueueEmpty()
{
   QMutexLocker locker(&internalEventMutex);
   return internalEventQueue.isEmpty();
}

bool QStateMachinePrivate::isExternalEventQueueEmpty()
{
   QMutexLocker locker(&externalEventMutex);
   return externalEventQueue.isEmpty();
}

void QStateMachinePrivate::processEvents(EventProcessingMode processingMode)
{
   Q_Q(QStateMachine);
   if ((state != Running) || processing || processingScheduled) {
      return;
   }
   switch (processingMode) {
      case DirectProcessing:
         if (QThread::currentThread() == q->thread()) {
            _q_process();
            break;
         } // fallthrough -- processing must be done in the machine thread
      case QueuedProcessing:
         processingScheduled = true;
         QMetaObject::invokeMethod(q, "_q_process", Qt::QueuedConnection);
         break;
   }
}

void QStateMachinePrivate::cancelAllDelayedEvents()
{
   Q_Q(QStateMachine);
   QMutexLocker locker(&delayedEventsMutex);
   QHash<int, QEvent *>::const_iterator it;
   for (it = delayedEvents.constBegin(); it != delayedEvents.constEnd(); ++it) {
      int id = it.key();
      QEvent *e = it.value();
      q->killTimer(id);
      delete e;
   }
   delayedEvents.clear();
}

namespace _QStateMachine_Internal {

class GoToStateTransition : public QAbstractTransition
{
 public:
   GoToStateTransition(QAbstractState *target)
      : QAbstractTransition() {
      setTargetState(target);
   }
 protected:
   void onTransition(QEvent *) override {
      deleteLater();
   }

   bool eventTest(QEvent *) override {
      return true;
   }
};

} // namespace

// mingw compiler tries to export QObject::findChild<GoToStateTransition>(),
// which doesn't work if its in an anonymous namespace.
using namespace _QStateMachine_Internal;
/*!
  \internal

  Causes this state machine to unconditionally transition to the given
  \a targetState.

  Provides a backdoor for using the state machine "imperatively"; i.e.  rather
  than defining explicit transitions, you drive the machine's execution by
  calling this function. It breaks the whole integrity of the
  transition-driven model, but is provided for pragmatic reasons.
*/
void QStateMachinePrivate::goToState(QAbstractState *targetState)
{
   if (!targetState) {
      qWarning("QStateMachine::goToState(): cannot go to null state");
      return;
   }

   if (configuration.contains(targetState)) {
      return;
   }

   QState *sourceState = 0;
   if (state == Running) {
      QSet<QAbstractState *>::const_iterator it;
      for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
         sourceState = toStandardState(*it);
         if (sourceState != 0) {
            break;
         }
      }
   } else {
      sourceState = startState();
   }

   Q_ASSERT(sourceState != 0);

   // Reuse previous GoToStateTransition in case of several calls to goToState() in a row
   GoToStateTransition *trans = sourceState->findChild<GoToStateTransition *>();

   if (! trans) {
      trans = new GoToStateTransition(targetState);
      sourceState->addTransition(trans);

   } else {
      trans->setTargetState(targetState);

   }

   processEvents(QueuedProcessing);
}

void QStateMachinePrivate::registerTransitions(QAbstractState *state)
{
   QState *group = toStandardState(state);
   if (! group) {
      return;
   }

   QList<QAbstractTransition *> transitions = QStatePrivate::get(group)->transitions();

   for (int i = 0; i < transitions.size(); ++i) {
      QAbstractTransition *t = transitions.at(i);

      if (QSignalTransition *st = qobject_cast<QSignalTransition *>(t)) {
         registerSignalTransition(st);
      }

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
      else if (QEventTransition *oet = qobject_cast<QEventTransition *>(t)) {
         registerEventTransition(oet);
      }
#endif
   }
}

void QStateMachinePrivate::unregisterTransition(QAbstractTransition *transition)
{
   if (QSignalTransition *st = qobject_cast<QSignalTransition *>(transition)) {
      unregisterSignalTransition(st);
   }

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   else if (QEventTransition *oet = qobject_cast<QEventTransition *>(transition)) {
      unregisterEventTransition(oet);
   }
#endif

}

void QStateMachinePrivate::registerSignalTransition(QSignalTransition *transition)
{
   Q_Q(QStateMachine);

   // signal
   QObject *sender = transition->senderObject();
   if (! sender) {
      return;
   }

   std::unique_ptr<CsSignal::Internal::BentoAbstract> signalBento = transition->get_signalBento()->clone();

   // slot
   if (! m_signalEventGenerator) {
      m_signalEventGenerator = new QSignalEventGenerator(q);
   }

   std::unique_ptr<CSBento<void (QSignalEventGenerator::*)()>> slotBento =
                  CsSignal::Internal::make_unique<CSBento<void (QSignalEventGenerator::*)()>>(&QSignalEventGenerator::execute);

   // BROOM (on hold, statemachine passed data is missing, change this form CsSignal to QObject)
   CsSignal::connect(*sender, std::move(signalBento), *m_signalEventGenerator, std::move(slotBento),
                  CsSignal::ConnectionKind::AutoConnection, true);
}

void QStateMachinePrivate::unregisterSignalTransition(QSignalTransition *transition)
{
   // signal
   QObject *sender = transition->senderObject();
   if (! sender) {
      return;
   }

   Q_ASSERT(m_signalEventGenerator != 0);

   CsSignal::Internal::BentoAbstract *signalBento = transition->get_signalBento();

/*
   // BROOM (on hold, statemachine passed data missing)
   QObject::disconnect(sender, signalBento, m_signalEventGenerator, &QSignalEventGenerator::execute);

*/

}

void QStateMachinePrivate::unregisterAllTransitions()
{
   Q_Q(QStateMachine);

   {
      QList<QSignalTransition *> transitions = rootState()->findChildren<QSignalTransition *>();
      for (int i = 0; i < transitions.size(); ++i) {
         QSignalTransition *t = transitions.at(i);

         if (t->machine() == q) {
            unregisterSignalTransition(t);
         }
      }
   }

   {
      QList<QEventTransition *> transitions = rootState()->findChildren<QEventTransition *>();
      for (int i = 0; i < transitions.size(); ++i) {
         QEventTransition *t = transitions.at(i);

         if (t->machine() == q) {
            unregisterEventTransition(t);
         }
      }
   }
}

void QSignalEventGenerator::execute()
{
   int sender_signalIndex = this->senderSignalIndex();
   Q_ASSERT(sender_signalIndex != -1);

   QObject *sender = this->sender();
   QStateMachine *machine = dynamic_cast<QStateMachine *>(parent());

   // BROOM (on hold, statemachine passed data missing)
   QStateMachinePrivate::get(machine)->handleTransitionSignal(sender, sender_signalIndex);
}


#ifndef QT_NO_STATEMACHINE_EVENTFILTER
void QStateMachinePrivate::registerEventTransition(QEventTransition *transition)
{
   Q_Q(QStateMachine);

   if (QEventTransitionPrivate::get(transition)->registered) {
      return;
   }
   if (transition->eventType() >= QEvent::User) {
      qWarning("QObject event transitions are not supported for custom types");
      return;
   }

   QObject *object = QEventTransitionPrivate::get(transition)->object;

   if (!object) {
      return;
   }

   QList<QPointer<QObject> > &eventFilters = CSInternalEvents::get_m_EventFilters(object);

   if (! eventFilters.contains(q)) {
      object->installEventFilter(q);
   }

   ++qobjectEvents[object][transition->eventType()];
   QEventTransitionPrivate::get(transition)->registered = true;

#ifdef QSTATEMACHINE_DEBUG
   qDebug() << q << ": added event transition from" << transition->sourceState()
            << ": ( object =" << object << ", event =" << transition->eventType()
            << ", targets =" << transition->targetStates() << ')';
#endif

}

void QStateMachinePrivate::unregisterEventTransition(QEventTransition *transition)
{
   Q_Q(QStateMachine);
   if (!QEventTransitionPrivate::get(transition)->registered) {
      return;
   }
   QObject *object = QEventTransitionPrivate::get(transition)->object;
   QHash<QEvent::Type, int> &events = qobjectEvents[object];
   Q_ASSERT(events.value(transition->eventType()) > 0);
   if (--events[transition->eventType()] == 0) {
      events.remove(transition->eventType());
      int sum = 0;
      QHash<QEvent::Type, int>::const_iterator it;
      for (it = events.constBegin(); it != events.constEnd(); ++it) {
         sum += it.value();
      }
      if (sum == 0) {
         qobjectEvents.remove(object);
         object->removeEventFilter(q);
      }
   }
   QEventTransitionPrivate::get(transition)->registered = false;
}

void QStateMachinePrivate::handleFilteredEvent(QObject *watched, QEvent *event)
{
   if (qobjectEvents.value(watched).contains(event->type())) {
      postInternalEvent(new QStateMachine::WrappedEvent(watched, handler->cloneEvent(event)));
      processEvents(DirectProcessing);
   }
}
#endif

void QStateMachinePrivate::handleTransitionSignal(QObject *sender, int signalIndex)   // , const TeaCupAbstract &data)
{
   // missing code to retrieve vArgs
   QList<QVariant> vArgs;       // = data.toVariantList();

#ifdef QSTATEMACHINE_DEBUG
   qDebug("Sending Signal Event");
#endif

   postInternalEvent(new QStateMachine::SignalEvent(sender, signalIndex, vArgs));
   processEvents(DirectProcessing);
}

QStateMachine::QStateMachine(QObject *parent)
   : QState(*new QStateMachinePrivate, /*parentState=*/0)
{
   // Can't pass the parent to the QState constructor, as it expects a QState
   // But this works as expected regardless of whether parent is a QState or not
   setParent(parent);
}

/*!
  \internal
*/
QStateMachine::QStateMachine(QStateMachinePrivate &dd, QObject *parent)
   : QState(dd, /*parentState=*/0)
{
   setParent(parent);
}

/*!
  Destroys this state machine.
*/
QStateMachine::~QStateMachine()
{
}

/*!
  \enum QStateMachine::EventPriority

  This enum type specifies the priority of an event posted to the state
  machine using postEvent().

  Events of high priority are processed before events of normal priority.

  \value NormalPriority The event has normal priority.
  \value HighPriority The event has high priority.
*/

/*! \enum QStateMachine::Error

    This enum type defines errors that can occur in the state machine at run time. When the state
    machine encounters an unrecoverable error at run time, it will set the error code returned
    by error(), the error message returned by errorString(), and enter an error state based on
    the context of the error.

    \value NoError No error has occurred.
    \value NoInitialStateError The machine has entered a QState with children which does not have an
           initial state set. The context of this error is the state which is missing an initial
           state.
    \value NoDefaultStateInHistoryStateError The machine has entered a QHistoryState which does not have
           a default state set. The context of this error is the QHistoryState which is missing a
           default state.
    \value NoCommonAncestorForTransitionError The machine has selected a transition whose source
           and targets are not part of the same tree of states, and thus are not part of the same
           state machine. Commonly, this could mean that one of the states has not been given
           any parent or added to any machine. The context of this error is the source state of
           the transition.

    \sa setErrorState()
*/

/*!
   \enum QStateMachine::RestorePolicy

   This enum specifies the restore policy type. The restore policy
   takes effect when the machine enters a state which sets one or more
   properties. If the restore policy is set to RestoreProperties,
   the state machine will save the original value of the property before the
   new value is set.

   Later, when the machine either enters a state which does not set
   a value for the given property, the property will automatically be restored
   to its initial value.

   Only one initial value will be saved for any given property. If a value for a property has
   already been saved by the state machine, it will not be overwritten until the property has been
   successfully restored.

   \value DontRestoreProperties The state machine should not save the initial values of properties
          and restore them later.
   \value RestoreProperties The state machine should save the initial values of properties
          and restore them later.

   \sa QStateMachine::globalRestorePolicy QState::assignProperty()
*/


/*!
  Returns the error code of the last error that occurred in the state machine.
*/
QStateMachine::Error QStateMachine::error() const
{
   Q_D(const QStateMachine);
   return d->error;
}

/*!
  Returns the error string of the last error that occurred in the state machine.
*/
QString QStateMachine::errorString() const
{
   Q_D(const QStateMachine);
   return d->errorString;
}

/*!
  Clears the error string and error code of the state machine.
*/
void QStateMachine::clearError()
{
   Q_D(QStateMachine);
   d->errorString.clear();
   d->error = NoError;
}

/*!
   Returns the restore policy of the state machine.

   \sa setGlobalRestorePolicy()
*/
QStateMachine::RestorePolicy QStateMachine::globalRestorePolicy() const
{
   Q_D(const QStateMachine);
   return d->globalRestorePolicy;
}

/*!
   Sets the restore policy of the state machine to \a restorePolicy. The default
   restore policy is QAbstractState::DontRestoreProperties.

   \sa globalRestorePolicy()
*/
void QStateMachine::setGlobalRestorePolicy(QStateMachine::RestorePolicy restorePolicy)
{
   Q_D(QStateMachine);
   d->globalRestorePolicy = restorePolicy;
}

/*!
  Adds the given \a state to this state machine. The state becomes a top-level
  state.

  If the state is already in a different machine, it will first be removed
  from its old machine, and then added to this machine.

  \sa removeState(), setInitialState()
*/
void QStateMachine::addState(QAbstractState *state)
{
   if (!state) {
      qWarning("QStateMachine::addState: cannot add null state");
      return;
   }
   if (QAbstractStatePrivate::get(state)->machine() == this) {
      qWarning("QStateMachine::addState: state has already been added to this machine");
      return;
   }
   state->setParent(this);
}

/*!
  Removes the given \a state from this state machine.  The state machine
  releases ownership of the state.

  \sa addState()
*/
void QStateMachine::removeState(QAbstractState *state)
{
   if (!state) {
      qWarning("QStateMachine::removeState: cannot remove null state");
      return;
   }
   if (QAbstractStatePrivate::get(state)->machine() != this) {
      qWarning("QStateMachine::removeState: state %p's machine (%p)"
               " is different from this machine (%p)",
               state, QAbstractStatePrivate::get(state)->machine(), this);
      return;
   }
   state->setParent(0);
}

/*!
  Returns whether this state machine is running.

  start(), stop()
*/
bool QStateMachine::isRunning() const
{
   Q_D(const QStateMachine);
   return (d->state == QStateMachinePrivate::Running);
}

/*!
  Starts this state machine.  The machine will reset its configuration and
  transition to the initial state.  When a final top-level state (QFinalState)
  is entered, the machine will emit the finished() signal.

  \note A state machine will not run without a running event loop, such as
  the main application event loop started with QCoreApplication::exec() or
  QApplication::exec().

  \sa started(), finished(), stop(), initialState()
*/
void QStateMachine::start()
{
   Q_D(QStateMachine);

   if (initialState() == 0) {
      qWarning("QStateMachine::start: No initial state set for machine. Refusing to start.");
      return;
   }

   switch (d->state) {
      case QStateMachinePrivate::NotRunning:
         d->state = QStateMachinePrivate::Starting;
         QMetaObject::invokeMethod(this, "_q_start", Qt::QueuedConnection);
         break;

      case QStateMachinePrivate::Starting:
         break;

      case QStateMachinePrivate::Running:
         qWarning("QStateMachine::start(): already running");
         break;
   }
}

/*!
  Stops this state machine. The state machine will stop processing events and
  then emit the stopped() signal.

  \sa stopped(), start()
*/
void QStateMachine::stop()
{
   Q_D(QStateMachine);
   switch (d->state) {
      case QStateMachinePrivate::NotRunning:
         break;
      case QStateMachinePrivate::Starting:
         // the machine will exit as soon as it enters the event processing loop
         d->stop = true;
         break;
      case QStateMachinePrivate::Running:
         d->stop = true;
         d->processEvents(QStateMachinePrivate::QueuedProcessing);
         break;
   }
}

/*!
  \threadsafe

  Posts the given \a event of the given \a priority for processing by this
  state machine.

  This function returns immediately. The event is added to the state machine's
  event queue. Events are processed in the order posted. The state machine
  takes ownership of the event and deletes it once it has been processed.

  You can only post events when the state machine is running.

  \sa postDelayedEvent()
*/
void QStateMachine::postEvent(QEvent *event, EventPriority priority)
{
   Q_D(QStateMachine);
   if (d->state != QStateMachinePrivate::Running) {
      qWarning("QStateMachine::postEvent: cannot post event when the state machine is not running");
      return;
   }
   if (!event) {
      qWarning("QStateMachine::postEvent: cannot post null event");
      return;
   }
#ifdef QSTATEMACHINE_DEBUG
   qDebug() << this << ": posting event" << event;
#endif
   switch (priority) {
      case NormalPriority:
         d->postExternalEvent(event);
         break;
      case HighPriority:
         d->postInternalEvent(event);
         break;
   }
   d->processEvents(QStateMachinePrivate::QueuedProcessing);
}

/*!
  \threadsafe

  Posts the given \a event for processing by this state machine, with the
  given \a delay in milliseconds. Returns an identifier associated with the
  delayed event, or -1 if the event could not be posted.

  This function returns immediately. When the delay has expired, the event
  will be added to the state machine's event queue for processing. The state
  machine takes ownership of the event and deletes it once it has been
  processed.

  You can only post events when the state machine is running.

  \sa cancelDelayedEvent(), postEvent()
*/
int QStateMachine::postDelayedEvent(QEvent *event, int delay)
{
   Q_D(QStateMachine);
   if (d->state != QStateMachinePrivate::Running) {
      qWarning("QStateMachine::postDelayedEvent: cannot post event when the state machine is not running");
      return -1;
   }
   if (!event) {
      qWarning("QStateMachine::postDelayedEvent: cannot post null event");
      return -1;
   }
   if (delay < 0) {
      qWarning("QStateMachine::postDelayedEvent: delay cannot be negative");
      return -1;
   }
#ifdef QSTATEMACHINE_DEBUG
   qDebug() << this << ": posting event" << event << "with delay" << delay;
#endif
   QMutexLocker locker(&d->delayedEventsMutex);
   int tid = startTimer(delay);
   d->delayedEvents[tid] = event;
   return tid;
}

/*!
  \threadsafe

  Cancels the delayed event identified by the given \a id. The id should be a
  value returned by a call to postDelayedEvent(). Returns true if the event
  was successfully cancelled, otherwise returns false.

  \sa postDelayedEvent()
*/
bool QStateMachine::cancelDelayedEvent(int id)
{
   Q_D(QStateMachine);
   if (d->state != QStateMachinePrivate::Running) {
      qWarning("QStateMachine::cancelDelayedEvent: the machine is not running");
      return false;
   }
   QMutexLocker locker(&d->delayedEventsMutex);
   QEvent *e = d->delayedEvents.take(id);
   if (!e) {
      return false;
   }
   killTimer(id);
   delete e;
   return true;
}

/*!
   Returns the maximal consistent set of states (including parallel and final
   states) that this state machine is currently in. If a state \c s is in the
   configuration, it is always the case that the parent of \c s is also in
   c. Note, however, that the machine itself is not an explicit member of the
   configuration.
*/
QSet<QAbstractState *> QStateMachine::configuration() const
{
   Q_D(const QStateMachine);
   return d->configuration;
}

/*!
  \fn QStateMachine::started()

  This signal is emitted when the state machine has entered its initial state
  (QStateMachine::initialState).

  \sa QStateMachine::finished(), QStateMachine::start()
*/

/*!
  \fn QStateMachine::stopped()

  This signal is emitted when the state machine has stopped.

  \sa QStateMachine::stop(), QStateMachine::finished()
*/

/*!
  \reimp
*/
bool QStateMachine::event(QEvent *e)
{
   Q_D(QStateMachine);
   if (e->type() == QEvent::Timer) {
      QTimerEvent *te = static_cast<QTimerEvent *>(e);
      int tid = te->timerId();
      if (d->state != QStateMachinePrivate::Running) {
         // This event has been cancelled already
         QMutexLocker locker(&d->delayedEventsMutex);
         Q_ASSERT(!d->delayedEvents.contains(tid));
         return true;
      }
      d->delayedEventsMutex.lock();
      QEvent *ee = d->delayedEvents.take(tid);
      if (ee != 0) {
         killTimer(tid);
         d->delayedEventsMutex.unlock();
         d->postExternalEvent(ee);
         d->processEvents(QStateMachinePrivate::DirectProcessing);
         return true;
      } else {
         d->delayedEventsMutex.unlock();
      }
   }
   return QState::event(e);
}

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
/*!
  \reimp
*/
bool QStateMachine::eventFilter(QObject *watched, QEvent *event)
{
   Q_D(QStateMachine);
   d->handleFilteredEvent(watched, event);
   return false;
}
#endif

/*!
  \internal

  This function is called when the state machine is about to select
  transitions based on the given \a event.

  The default implementation does nothing.
*/
void QStateMachine::beginSelectTransitions(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \internal

  This function is called when the state machine has finished selecting
  transitions based on the given \a event.

  The default implementation does nothing.
*/
void QStateMachine::endSelectTransitions(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \internal

  This function is called when the state machine is about to do a microstep.

  The default implementation does nothing.
*/
void QStateMachine::beginMicrostep(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \internal

  This function is called when the state machine has finished doing a
  microstep.

  The default implementation does nothing.
*/
void QStateMachine::endMicrostep(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \reimp
    This function will call start() to start the state machine.
*/
void QStateMachine::onEntry(QEvent *event)
{
   start();
   QState::onEntry(event);
}

/*!
  \reimp
    This function will call stop() to stop the state machine and
    subsequently emit the stopped() signal.
*/
void QStateMachine::onExit(QEvent *event)
{
   stop();
   QState::onExit(event);
}

#ifndef QT_NO_ANIMATION

/*!
  Returns whether animations are enabled for this state machine.
*/
bool QStateMachine::isAnimated() const
{
   Q_D(const QStateMachine);
   return d->animated;
}

/*!
  Sets whether animations are \a enabled for this state machine.
*/
void QStateMachine::setAnimated(bool enabled)
{
   Q_D(QStateMachine);
   d->animated = enabled;
}

/*!
    Adds a default \a animation to be considered for any transition.
*/
void QStateMachine::addDefaultAnimation(QAbstractAnimation *animation)
{
   Q_D(QStateMachine);
   d->defaultAnimations.append(animation);
}

/*!
    Returns the list of default animations that will be considered for any transition.
*/
QList<QAbstractAnimation *> QStateMachine::defaultAnimations() const
{
   Q_D(const QStateMachine);
   return d->defaultAnimations;
}

/*!
    Removes \a animation from the list of default animations.
*/
void QStateMachine::removeDefaultAnimation(QAbstractAnimation *animation)
{
   Q_D(QStateMachine);
   d->defaultAnimations.removeAll(animation);
}

#endif // QT_NO_ANIMATION


QSignalEventGenerator::QSignalEventGenerator(QStateMachine *parent)
   : QObject(parent)
{
}


/*!
  \internal

  Constructs a new SignalEvent object with the given \a sender, \a
  signalIndex and \a arguments.
*/
QStateMachine::SignalEvent::SignalEvent(QObject *sender, int signalIndex, const QList<QVariant> &arguments)
   : QEvent(QEvent::StateMachineSignal), m_sender(sender),
     m_signalIndex(signalIndex), m_arguments(arguments)
{
}

/*!
  Destroys this SignalEvent.
*/
QStateMachine::SignalEvent::~SignalEvent()
{
}


/*!
  \internal

  Constructs a new WrappedEvent object with the given \a object
  and \a event.

  The WrappedEvent object takes ownership of \a event.
*/
QStateMachine::WrappedEvent::WrappedEvent(QObject *object, QEvent *event)
   : QEvent(QEvent::StateMachineWrapped), m_object(object), m_event(event)
{
}

/*!
  Destroys this WrappedEvent.
*/
QStateMachine::WrappedEvent::~WrappedEvent()
{
   delete m_event;
}


#endif //QT_NO_STATEMACHINE
