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

#include <qstatemachine.h>

#ifndef QT_NO_STATEMACHINE

#include <qstate.h>

#include <qabstractstate.h>
#include <qabstracttransition.h>
#include <qalgorithms.h>
#include <qdebug.h>
#include <qfinalstate.h>
#include <qhistorystate.h>
#include <qmetaobject.h>
#include <qsignaltransition.h>
#include <qvarlengtharray.h>

#include <qabstractstate_p.h>
#include <qabstracttransition_p.h>
#include <qhistorystate_p.h>
#include <qsignaleventgenerator_p.h>
#include <qstate_p.h>
#include <qstatemachine_p.h>
#include <qthread_p.h>

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
#include <qeventtransition.h>
#include <qeventtransition_p.h>
#endif

#ifndef QT_NO_ANIMATION
#include <qanimationgroup.h>
#include <qpropertyanimation.h>

#include <qvariantanimation_p.h>
#endif

#include <algorithm>

struct CalculationCache {
   struct TransitionInfo {
      QList<QAbstractState *> effectiveTargetStates;
      QSet<QAbstractState *> exitSet;
      QAbstractState *transitionDomain;

      bool effectiveTargetStatesIsKnown: 1;
      bool exitSetIsKnown              : 1;
      bool transitionDomainIsKnown     : 1;

      TransitionInfo()
         : transitionDomain(nullptr), effectiveTargetStatesIsKnown(false),
           exitSetIsKnown(false), transitionDomainIsKnown(false)
      { }
   };

   using TransitionInfoCache = QHash<QAbstractTransition *, TransitionInfo>;
   TransitionInfoCache cache;

   bool effectiveTargetStates(QAbstractTransition *t, QList<QAbstractState *> *targets) const {
      Q_ASSERT(targets);

      TransitionInfoCache::const_iterator cacheIt = cache.find(t);

      if (cacheIt == cache.end() || ! cacheIt->effectiveTargetStatesIsKnown) {
         return false;
      }

      *targets = cacheIt->effectiveTargetStates;
      return true;
   }

   void insert(QAbstractTransition *t, const QList<QAbstractState *> &targets) {
      TransitionInfoCache::iterator cacheIt = cache.find(t);

      TransitionInfo &ti = cacheIt == cache.end()
            ? *cache.insert(t, TransitionInfo()) : *cacheIt;

      Q_ASSERT(!ti.effectiveTargetStatesIsKnown);
      ti.effectiveTargetStates = targets;
      ti.effectiveTargetStatesIsKnown = true;
   }

   bool exitSet(QAbstractTransition *t, QSet<QAbstractState *> *exits) const {
      Q_ASSERT(exits);

      TransitionInfoCache::const_iterator cacheIt = cache.find(t);

      if (cacheIt == cache.end() || ! cacheIt->exitSetIsKnown) {
         return false;
      }

      *exits = cacheIt->exitSet;
      return true;
   }

   void insert(QAbstractTransition *t, const QSet<QAbstractState *> &exits) {
      TransitionInfoCache::iterator cacheIt = cache.find(t);
      TransitionInfo &ti = cacheIt == cache.end()
            ? *cache.insert(t, TransitionInfo()) : *cacheIt;

      Q_ASSERT(!ti.exitSetIsKnown);
      ti.exitSet = exits;
      ti.exitSetIsKnown = true;
   }

   bool transitionDomain(QAbstractTransition *t, QAbstractState **domain) const {
      Q_ASSERT(domain);

      TransitionInfoCache::const_iterator cacheIt = cache.find(t);

      if (cacheIt == cache.end() || !cacheIt->transitionDomainIsKnown) {
         return false;
      }

      *domain = cacheIt->transitionDomain;
      return true;
   }

   void insert(QAbstractTransition *t, QAbstractState *domain) {
      TransitionInfoCache::iterator cacheIt = cache.find(t);
      TransitionInfo &ti = cacheIt == cache.end()
            ? *cache.insert(t, TransitionInfo()) : *cacheIt;

      Q_ASSERT(!ti.transitionDomainIsKnown);
      ti.transitionDomain = domain;
      ti.transitionDomainIsKnown = true;
   }
};

static inline bool isDescendant(const QAbstractState *state1, const QAbstractState *state2)
{
   Q_ASSERT(state1 != nullptr);

   for (QAbstractState *it = state1->parentState(); it != nullptr; it = it->parentState()) {
      if (it == state2) {
         return true;
      }
   }

   return false;
}

static bool containsDecendantOf(const QSet<QAbstractState *> &states, const QAbstractState *node)
{
   for (QAbstractState *s : states) {
      if (isDescendant(s, node)) {
         return true;
      }
   }

   return false;
}

static int descendantDepth(const QAbstractState *state, const QAbstractState *ancestor)
{
   int depth = 0;

   for (const QAbstractState *it = state; it != nullptr; it = it->parentState()) {
      if (it == ancestor) {
         break;
      }

      ++depth;
   }

   return depth;
}
static QVector<QState *> getProperAncestors(const QAbstractState *state, const QAbstractState *upperBound)
{
   Q_ASSERT(state != nullptr);

   QVector<QState *> result;
   result.reserve(16);

   for (QState *it = state->parentState(); it && it != upperBound; it = it->parentState()) {
      result.append(it);
   }

   return result;
}

static QList<QAbstractState *> getEffectiveTargetStates(QAbstractTransition *transition, CalculationCache *cache)
{
   Q_ASSERT(cache);

   QList<QAbstractState *> targetsList;

   if (cache->effectiveTargetStates(transition, &targetsList)) {
      return targetsList;
   }

   QSet<QAbstractState *> targets;

   for (QAbstractState *s : transition->targetStates()) {
      if (QHistoryState *historyState = QStateMachinePrivate::toHistoryState(s)) {
         QList<QAbstractState *> historyConfiguration = QHistoryStatePrivate::get(historyState)->configuration;

         if (! historyConfiguration.isEmpty()) {
            // There is a saved history, so apply that.
            targets.unite(historyConfiguration.toSet());

         } else if (QAbstractTransition *defaultTransition = historyState->defaultTransition()) {
            // No saved history, take all default transition targets.
            targets.unite(defaultTransition->targetStates().toSet());

         } else {
            // we found a history state without a default state. That's not valid!
            QStateMachinePrivate *m = QStateMachinePrivate::get(historyState->machine());
            m->setError(QStateMachine::NoDefaultStateInHistoryStateError, historyState);
         }

      } else {
         targets.insert(s);
      }
   }

   targetsList = targets.toList();
   cache->insert(transition, targetsList);

   return targetsList;
}

QStateMachinePrivate::QStateMachinePrivate()
{
   state       = NotRunning;
   isMachine   = true;
   processing  = false;
   stop        = false;
   processingScheduled = false;

   stopProcessingReason = EventQueueEmpty;
   error = QStateMachine::NoError;
   globalRestorePolicy = QState::DontRestoreProperties;

   m_signalEventGenerator = nullptr;

#ifndef QT_NO_ANIMATION
   animated = true;
#endif
}

QStateMachinePrivate::~QStateMachinePrivate()
{
   qDeleteAll(internalEventQueue);
   qDeleteAll(externalEventQueue);

   for (QHash<int, DelayedEvent>::const_iterator it = delayedEvents.begin(), eit = delayedEvents.end(); it != eit; ++it) {
      delete it.value().event;
   }
}

QStateMachinePrivate *QStateMachinePrivate::get(QStateMachine *q)
{
   if (q != nullptr) {
      return q->d_func();
   }

   return nullptr;
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

   return nullptr;
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

      if ((c == desc) || isDescendant(desc, c)) {
         return i;
      }
   }

   return -1;
}

bool QStateMachinePrivate::transitionStateEntryLessThan(QAbstractTransition *t1, QAbstractTransition *t2)
{
   QState *s1 = t1->sourceState();
   QState *s2 = t2->sourceState();

   if (s1 == s2) {
      QList<QAbstractTransition *> transitions = QStatePrivate::get(s1)->transitions();
      return transitions.indexOf(t1) < transitions.indexOf(t2);

   } else if (isDescendant(s1, s2)) {
      return true;

   } else if (isDescendant(s2, s1)) {
      return false;

   } else {
      Q_ASSERT(s1->machine() != nullptr);
      QStateMachinePrivate *mach = QStateMachinePrivate::get(s1->machine());
      QState *lca = mach->findLCA(QList<QAbstractState *>() << s1 << s2);
      Q_ASSERT(lca != nullptr);

      int s1Depth = descendantDepth(s1, lca);
      int s2Depth = descendantDepth(s2, lca);

      if (s1Depth == s2Depth) {
         return (indexOfDescendant(lca, s1) < indexOfDescendant(lca, s2));
      } else {
         return s1Depth > s2Depth;
      }
   }
}

bool QStateMachinePrivate::stateEntryLessThan(QAbstractState *s1, QAbstractState *s2)
{
   if (s1->parent() == s2->parent()) {
      return s1->parent()->children().indexOf(s1) < s2->parent()->children().indexOf(s2);

   } else if (isDescendant(s1, s2)) {
      return false;

   } else if (isDescendant(s2, s1)) {
      return true;

   } else {
      Q_ASSERT(s1->machine() != nullptr);

      QStateMachinePrivate *mach = QStateMachinePrivate::get(s1->machine());
      QState *lca = mach->findLCA(QList<QAbstractState *>() << s1 << s2);

      Q_ASSERT(lca != nullptr);

      return (indexOfDescendant(lca, s1) < indexOfDescendant(lca, s2));
   }
}

bool QStateMachinePrivate::stateExitLessThan(QAbstractState *s1, QAbstractState *s2)
{
   if (s1->parent() == s2->parent()) {
      return s2->parent()->children().indexOf(s2) < s1->parent()->children().indexOf(s1);

   } else if (isDescendant(s1, s2)) {
      return true;

   } else if (isDescendant(s2, s1)) {
      return false;

   } else {
      Q_ASSERT(s1->machine() != nullptr);

      QStateMachinePrivate *mach = QStateMachinePrivate::get(s1->machine());
      QState *lca = mach->findLCA(QList<QAbstractState *>() << s1 << s2);

      Q_ASSERT(lca != nullptr);

      return (indexOfDescendant(lca, s2) < indexOfDescendant(lca, s1));
   }
}

QState *QStateMachinePrivate::findLCA(const QList<QAbstractState *> &states, bool onlyCompound) const
{
   if (states.isEmpty()) {
      return nullptr;
   }

   QVector<QState *> ancestors = getProperAncestors(states.at(0), rootState()->parentState());

   for (int i = 0; i < ancestors.size(); ++i) {
      QState *anc = ancestors.at(i);

      if (onlyCompound && ! isCompound(anc)) {
         continue;
      }

      bool ok = true;

      for (int j = states.size() - 1; (j > 0) && ok; --j) {
         const QAbstractState *s = states.at(j);

         if (! isDescendant(s, anc)) {
            ok = false;
         }
      }

      if (ok) {
         return anc;
      }
   }

   return nullptr;
}

QState *QStateMachinePrivate::findLCCA(const QList<QAbstractState *> &states) const
{
   return findLCA(states, true);
}

QList<QAbstractTransition *> QStateMachinePrivate::selectTransitions(QEvent *event, CalculationCache *cache)
{
   Q_ASSERT(cache);
   Q_Q(const QStateMachine);

   QVarLengthArray<QAbstractState *> configuration_sorted;

   for (QAbstractState *s : configuration) {
      if (isAtomic(s)) {
         configuration_sorted.append(s);
      }
   }

   std::sort(configuration_sorted.begin(), configuration_sorted.end(), stateEntryLessThan);

   QList<QAbstractTransition *> enabledTransitions;
   const_cast<QStateMachine *>(q)->beginSelectTransitions(event);

   for (QAbstractState *state : configuration_sorted) {
      QVector<QState *> lst = getProperAncestors(state, nullptr);

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

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
               qDebug() << q << ": selecting transition" << t;
#endif
               enabledTransitions.append(t);
               found = true;
               break;
            }
         }
      }

      if (! enabledTransitions.isEmpty()) {
         removeConflictingTransitions(enabledTransitions, cache);
      }
   }

   const_cast<QStateMachine *>(q)->endSelectTransitions(event);

   return enabledTransitions;
}

void QStateMachinePrivate::removeConflictingTransitions(QList<QAbstractTransition *> &enabledTransitions, CalculationCache *cache)
{
   Q_ASSERT(cache);

   if (enabledTransitions.size() < 2) {
      return; // There is no transition to conflict with
   }

   QList<QAbstractTransition *> filteredTransitions;

   std::sort(enabledTransitions.begin(), enabledTransitions.end(), transitionStateEntryLessThan);

   for (QAbstractTransition *t1 : enabledTransitions) {
      bool t1Preempted = false;

      const QSet<QAbstractState *> exitSetT1 = computeExitSet_Unordered(t1, cache);
      QList<QAbstractTransition *>::iterator t2It = filteredTransitions.begin();

      while (t2It != filteredTransitions.end()) {
         QAbstractTransition *t2 = *t2It;

         if (t1 == t2) {
            // Special case, someone added the same transition object to a state twice. In this
            // case, t2 (which is already in the list) "preempts" t1.
            t1Preempted = true;
            break;
         }

         QSet<QAbstractState *> exitSetT2 = computeExitSet_Unordered(t2, cache);

         if (! exitSetT1.intersects(exitSetT2)) {
            // no conflict
            ++t2It;

         } else {
            // have a conflict, check which transition can be removed
            if (isDescendant(t1->sourceState(), t2->sourceState())) {
               // t1 preempts t2, so we can remove t2
               t2It = filteredTransitions.erase(t2It);

            } else {
               // t2 preempts t1, so there is no use in looking further and we don't need to add
               // t1 to the list.
               t1Preempted = true;
               break;
            }
         }
      }

      if (! t1Preempted) {
         filteredTransitions.append(t1);
      }
   }

   enabledTransitions = filteredTransitions;
}
void QStateMachinePrivate::microstep(QEvent *event, const QList<QAbstractTransition *> &enabledTransitions,
      CalculationCache *cache)
{
   Q_ASSERT(cache);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": begin microstep( enabledTransitions:" << enabledTransitions << ')';
   qDebug() << q_func() << ": configuration before exiting states:" << configuration;
#endif

   QList<QAbstractState *> exitedStates = computeExitSet(enabledTransitions, cache);
   QHash<RestorableId, QVariant> pendingRestorables = computePendingRestorables(exitedStates);

   QSet<QAbstractState *> statesForDefaultEntry;
   QList<QAbstractState *> enteredStates = computeEntrySet(enabledTransitions, statesForDefaultEntry, cache);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": computed exit set:"  << exitedStates;
   qDebug() << q_func() << ": computed entry set:" << enteredStates;
#endif

   QHash<QAbstractState *, QVector<QPropertyAssignment>> assignmentsForEnteredStates =
         computePropertyAssignments(enteredStates, pendingRestorables);

   if (! pendingRestorables.isEmpty()) {
      // Add "implicit" assignments for restored properties to the first (outermost) entered state

      Q_ASSERT(!enteredStates.isEmpty());
      QAbstractState *s = enteredStates.first();
      assignmentsForEnteredStates[s] << restorablesToPropertyList(pendingRestorables);
   }

   exitStates(event, exitedStates, assignmentsForEnteredStates);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": configuration after exiting states:" << configuration;
#endif

   executeTransitionContent(event, enabledTransitions);

#ifndef QT_NO_ANIMATION
   QList<QAbstractAnimation *> selectedAnimations = selectAnimations(enabledTransitions);
   enterStates(event, exitedStates, enteredStates, statesForDefaultEntry, assignmentsForEnteredStates, selectedAnimations);

#else
   enterStates(event, exitedStates, enteredStates, statesForDefaultEntry, assignmentsForEnteredStates);

#endif

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": configuration after entering states:" << configuration;
   qDebug() << q_func() << ": end microstep";
#endif
}

QList<QAbstractState *> QStateMachinePrivate::computeExitSet(const QList<QAbstractTransition *> &enabledTransitions,
      CalculationCache *cache)
{
   Q_ASSERT(cache);

   QList<QAbstractState *> statesToExit_sorted = computeExitSet_Unordered(enabledTransitions, cache).toList();
   std::sort(statesToExit_sorted.begin(), statesToExit_sorted.end(), stateExitLessThan);

   return statesToExit_sorted;
}

QSet<QAbstractState *> QStateMachinePrivate::computeExitSet_Unordered(const QList<QAbstractTransition *> &enabledTransitions,
      CalculationCache *cache)
{
   Q_ASSERT(cache);

   QSet<QAbstractState *> statesToExit;

   for (QAbstractTransition *t : enabledTransitions) {
      statesToExit.unite(computeExitSet_Unordered(t, cache));
   }

   return statesToExit;
}

QSet<QAbstractState *> QStateMachinePrivate::computeExitSet_Unordered(QAbstractTransition *t, CalculationCache *cache)
{
   Q_ASSERT(cache);

   QSet<QAbstractState *> statesToExit;

   if (cache->exitSet(t, &statesToExit)) {
      return statesToExit;
   }

   QList<QAbstractState *> effectiveTargetStates = getEffectiveTargetStates(t, cache);
   QAbstractState *domain = getTransitionDomain(t, effectiveTargetStates, cache);

   if (domain == nullptr && ! t->targetStates().isEmpty()) {
      // So we didn't find the least common ancestor for the source and target states of the
      // transition. If there were not target states, that would be fine: then the transition
      // will fire any events or signals, but not exit the state.
      //
      // However, there are target states, so it's either a node without a parent (or parent's
      // parent, etc), or the state belongs to a different state machine. Either way, this
      // makes the state machine invalid.

      if (error == QStateMachine::NoError) {
         setError(QStateMachine::NoCommonAncestorForTransitionError, t->sourceState());
      }

      QList<QAbstractState *> lst = pendingErrorStates.toList();
      lst.prepend(t->sourceState());

      domain = findLCCA(lst);
      Q_ASSERT(domain != nullptr);
   }

   for (QAbstractState *s : configuration) {
      if (isDescendant(s, domain)) {
         statesToExit.insert(s);
      }
   }

   cache->insert(t, statesToExit);
   return statesToExit;
}

void QStateMachinePrivate::exitStates(QEvent *event, const QList<QAbstractState *> &statesToExit_sorted,
      const QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates)
{

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
                  if (isAtomic(s0) && isDescendant(s0, s)) {
                     QHistoryStatePrivate::get(h)->configuration.append(s0);
                  }

               } else if (s0->parentState() == s) {
                  QHistoryStatePrivate::get(h)->configuration.append(s0);
               }
            }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
            qDebug() << q_func() << ": recorded"
                  << ((QHistoryStatePrivate::get(h)->historyType == QHistoryState::DeepHistory)
                  ? "deep" : "shallow")
                  << "history for" << s << "in" << h << ':' << QHistoryStatePrivate::get(h)->configuration;
#endif
         }
      }
   }

   for (int i = 0; i < statesToExit_sorted.size(); ++i) {
      QAbstractState *s = statesToExit_sorted.at(i);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
      qDebug() << q_func() << ": exiting" << s;
#endif

      QAbstractStatePrivate::get(s)->callOnExit(event);

#ifndef QT_NO_ANIMATION
      terminateActiveAnimations(s, assignmentsForEnteredStates);
#else
      (void) assignmentsForEnteredStates;
#endif

      configuration.remove(s);
      QAbstractStatePrivate::get(s)->emitExited();
   }
}

void QStateMachinePrivate::executeTransitionContent(QEvent *event,
      const QList<QAbstractTransition *> &enabledTransitions)
{
   for (int i = 0; i < enabledTransitions.size(); ++i) {
      QAbstractTransition *t = enabledTransitions.at(i);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
      qDebug() << q_func() << ": triggering" << t;
#endif

      QAbstractTransitionPrivate::get(t)->callOnTransition(event);
      QAbstractTransitionPrivate::get(t)->emitTriggered();
   }
}

QList<QAbstractState *> QStateMachinePrivate::computeEntrySet(const QList<QAbstractTransition *> &enabledTransitions,
      QSet<QAbstractState *> &statesForDefaultEntry, CalculationCache *cache)
{
   Q_ASSERT(cache);

   QSet<QAbstractState *> statesToEnter;

   if (pendingErrorStates.isEmpty()) {
      for (QAbstractTransition *t : enabledTransitions) {
         for (QAbstractState *s : t->targetStates()) {
            addDescendantStatesToEnter(s, statesToEnter, statesForDefaultEntry);
         }

         QList<QAbstractState *> effectiveTargetStates = getEffectiveTargetStates(t, cache);
         QAbstractState *ancestor = getTransitionDomain(t, effectiveTargetStates, cache);

         for (QAbstractState *s : effectiveTargetStates) {
            addAncestorStatesToEnter(s, ancestor, statesToEnter, statesForDefaultEntry);
         }
      }
   }

   // Did an error occur while selecting transitions? Then we enter the error state.
   if (! pendingErrorStates.isEmpty()) {
      statesToEnter.clear();
      statesToEnter = pendingErrorStates;
      statesForDefaultEntry = pendingErrorStatesForDefaultEntry;
      pendingErrorStates.clear();
      pendingErrorStatesForDefaultEntry.clear();
   }

   QList<QAbstractState *> statesToEnter_sorted = statesToEnter.toList();
   std::sort(statesToEnter_sorted.begin(), statesToEnter_sorted.end(), stateEntryLessThan);

   return statesToEnter_sorted;
}

QAbstractState *QStateMachinePrivate::getTransitionDomain(QAbstractTransition *t,
      const QList<QAbstractState *> &effectiveTargetStates, CalculationCache *cache) const
{
   Q_ASSERT(cache);

   if (effectiveTargetStates.isEmpty()) {
      return nullptr;
   }

   QAbstractState *domain = nullptr;

   if (cache->transitionDomain(t, &domain)) {
      return domain;
   }

   if (t->transitionType() == QAbstractTransition::InternalTransition) {
      if (QState *tSource = t->sourceState()) {
         if (isCompound(tSource)) {
            bool allDescendants = true;

            for (QAbstractState *s : effectiveTargetStates) {
               if (!isDescendant(s, tSource)) {
                  allDescendants = false;
                  break;
               }
            }

            if (allDescendants) {
               return tSource;
            }
         }
      }
   }

   QList<QAbstractState *> states(effectiveTargetStates);

   if (QAbstractState *src = t->sourceState()) {
      states.prepend(src);
   }

   domain = findLCCA(states);
   cache->insert(t, domain);
   return domain;
}
void QStateMachinePrivate::enterStates(QEvent *event, const QList<QAbstractState *> &exitedStates_sorted,
      const QList<QAbstractState *> &statesToEnter_sorted,
      const QSet<QAbstractState *> &statesForDefaultEntry,
      QHash<QAbstractState *, QVector<QPropertyAssignment>> &propertyAssignmentsForState
#ifndef QT_NO_ANIMATION
      , const QList<QAbstractAnimation *> &selectedAnimations
#endif
)
{
#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   Q_Q(QStateMachine);
#endif

   for (int i = 0; i < statesToEnter_sorted.size(); ++i) {
      QAbstractState *s = statesToEnter_sorted.at(i);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
      qDebug() << q << ": entering" << s;
#endif

      configuration.insert(s);
      registerTransitions(s);

#ifndef QT_NO_ANIMATION
      initializeAnimations(s, selectedAnimations, exitedStates_sorted, propertyAssignmentsForState);
#endif

      // Immediately set the properties that are not animated.
      {
         QVector<QPropertyAssignment> assignments = propertyAssignmentsForState.value(s);

         for (int i = 0; i < assignments.size(); ++i) {
            const QPropertyAssignment &assn = assignments.at(i);

            if (globalRestorePolicy == QState::RestoreProperties) {
               if (assn.explicitlySet) {
                  if (!hasRestorable(s, assn.object, assn.propertyName)) {
                     QVariant value = savedValueForRestorable(exitedStates_sorted, assn.object, assn.propertyName);
                     unregisterRestorables(exitedStates_sorted, assn.object, assn.propertyName);
                     registerRestorable(s, assn.object, assn.propertyName, value);
                  }

               } else {
                  // The property is being restored, hence no need to
                  // save the current value. Discard any saved values in
                  // exited states, since those are now stale.
                  unregisterRestorables(exitedStates_sorted, assn.object, assn.propertyName);
               }
            }

            assn.write();
         }
      }

      QAbstractStatePrivate::get(s)->callOnEntry(event);
      QAbstractStatePrivate::get(s)->emitEntered();

      // FIXME: refer to the "initial transitions" comment in addDescendantStatesToEnter first, then implement:
      //      if (statesForDefaultEntry.contains(s)) {
      //         // ### executeContent(s.initial.transition.children())
      //      }

      (void) statesForDefaultEntry;

      if (QHistoryState *h = toHistoryState(s)) {
         QAbstractTransitionPrivate::get(h->defaultTransition())->callOnTransition(event);
      }

      // Emit propertiesAssigned signal if the state has no animated properties.
      {
         QState *ss = toStandardState(s);

         if (ss
#ifndef QT_NO_ANIMATION
               && ! animationsForState.contains(s)
#endif
         ) {
            QStatePrivate::get(ss)->emitPropertiesAssigned();
         }
      }

      if (isFinal(s)) {
         QState *parent = s->parentState();

         if (parent) {

            if (parent != rootState()) {
               QFinalState *finalState = qobject_cast<QFinalState *>(s);
               Q_ASSERT(finalState);
               emitStateFinished(parent, finalState);
            }

            QState *grandparent = parent->parentState();

            if (grandparent && isParallel(grandparent)) {
               bool allChildStatesFinal = true;
               QList<QAbstractState *> childStates = QStatePrivate::get(grandparent)->childStates();

               for (int j = 0; j < childStates.size(); ++j) {
                  QAbstractState *cs = childStates.at(j);

                  if (! isInFinalState(cs)) {
                     allChildStatesFinal = false;
                     break;
                  }
               }

               if (allChildStatesFinal && (grandparent != rootState())) {
                  QFinalState *finalState = qobject_cast<QFinalState *>(s);
                  Q_ASSERT(finalState);
                  emitStateFinished(grandparent, finalState);
               }

            }
         }
      }
   }

   {
      QSet<QAbstractState *>::const_iterator it;

      for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
         if (isFinal(*it)) {
            QState *parent = (*it)->parentState();

            if (((parent == rootState()) && (rootState()->childMode() == QState::ExclusiveStates))
                  || ((parent->parentState() == rootState())
                  && (rootState()->childMode() == QState::ParallelStates) && isInFinalState(rootState()))) {
               processing = false;
               stopProcessingReason = Finished;
               break;
            }
         }
      }
   }
}

void QStateMachinePrivate::addDescendantStatesToEnter(QAbstractState *state,
      QSet<QAbstractState *> &statesToEnter, QSet<QAbstractState *> &statesForDefaultEntry)
{
   if (QHistoryState *h = toHistoryState(state)) {
      QList<QAbstractState *> historyConfiguration = QHistoryStatePrivate::get(h)->configuration;

      if (! historyConfiguration.isEmpty()) {
         for (QAbstractState *s : historyConfiguration) {
            addDescendantStatesToEnter(s, statesToEnter, statesForDefaultEntry);
         }

         for (QAbstractState *s : historyConfiguration) {
            addAncestorStatesToEnter(s, state->parentState(), statesToEnter, statesForDefaultEntry);
         }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
         qDebug() << q_func() << ": restoring"
               << ((QHistoryStatePrivate::get(h)->historyType == QHistoryState::DeepHistory) ? "deep" : "shallow")
               << "history from" << state << ':' << historyConfiguration;
#endif

      } else {
         QList<QAbstractState *> defaultHistoryContent;

         if (QAbstractTransition *t = QHistoryStatePrivate::get(h)->defaultTransition) {
            defaultHistoryContent = t->targetStates();
         }

         if (defaultHistoryContent.isEmpty()) {
            setError(QStateMachine::NoDefaultStateInHistoryStateError, h);

         } else {
            for (QAbstractState *s : defaultHistoryContent) {
               addDescendantStatesToEnter(s, statesToEnter, statesForDefaultEntry);
            }

            for (QAbstractState *s : defaultHistoryContent) {
               addAncestorStatesToEnter(s, state->parentState(), statesToEnter, statesForDefaultEntry);
            }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
            qDebug() << q_func() << ": initial history targets for" << state << ':' << defaultHistoryContent;
#endif
         }
      }

   } else {
      if (state == rootState()) {
         // Error has already been set by exitStates().
         Q_ASSERT(error != QStateMachine::NoError);
         return;
      }

      statesToEnter.insert(state);

      if (isCompound(state)) {
         statesForDefaultEntry.insert(state);

         if (QAbstractState *initial = toStandardState(state)->initialState()) {
            Q_ASSERT(initial->machine() == q_func());

            // FIXME: does not support initial transitions (which is a problem for parallel states).
            // The way it simulates this for other states, is by having a single initial state.
            // See also the FIXME in enterStates.
            statesForDefaultEntry.insert(initial);

            addDescendantStatesToEnter(initial, statesToEnter, statesForDefaultEntry);
            addAncestorStatesToEnter(initial, state, statesToEnter, statesForDefaultEntry);

         } else {
            setError(QStateMachine::NoInitialStateError, state);
            return;
         }

      } else if (isParallel(state)) {
         QState *grp = toStandardState(state);

         for (QAbstractState *child : QStatePrivate::get(grp)->childStates()) {
            if (! containsDecendantOf(statesToEnter, child)) {
               addDescendantStatesToEnter(child, statesToEnter, statesForDefaultEntry);
            }
         }
      }
   }
}

void QStateMachinePrivate::addAncestorStatesToEnter(QAbstractState *s, QAbstractState *ancestor,
      QSet<QAbstractState *> &statesToEnter, QSet<QAbstractState *> &statesForDefaultEntry)
{
   for (QState *anc : getProperAncestors(s, ancestor)) {
      if (! anc->parentState()) {
         continue;
      }

      statesToEnter.insert(anc);

      if (isParallel(anc)) {
         for (QAbstractState *child : QStatePrivate::get(anc)->childStates()) {
            if (! containsDecendantOf(statesToEnter, child)) {
               addDescendantStatesToEnter(child, statesToEnter, statesForDefaultEntry);
            }
         }
      }
   }
}

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

   if (! group) {
      return false;
   }

   bool isMachine = QStatePrivate::get(group)->isMachine;

   // do not treat the machine as compound if it's a sub-state of this machine
   if (isMachine && (group != rootState())) {
      return false;
   }

   return (! isParallel(group) && ! QStatePrivate::get(group)->childStates().isEmpty());
}

bool QStateMachinePrivate::isAtomic(const QAbstractState *s) const
{
   const QState *ss = toStandardState(s);

   // Treat the machine as atomic if it's a sub-state of this machine

   return (ss && QStatePrivate::get(ss)->childStates().isEmpty()) || isFinal(s)
         || (ss && QStatePrivate::get(ss)->isMachine && (ss != rootState()));
}

QState *QStateMachinePrivate::toStandardState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::StandardState)) {
      return static_cast<QState *>(state);
   }

   return nullptr;
}

const QState *QStateMachinePrivate::toStandardState(const QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::StandardState)) {
      return static_cast<const QState *>(state);
   }

   return nullptr;
}

QFinalState *QStateMachinePrivate::toFinalState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::FinalState)) {
      return static_cast<QFinalState *>(state);
   }

   return nullptr;
}

QHistoryState *QStateMachinePrivate::toHistoryState(QAbstractState *state)
{
   if (state && (QAbstractStatePrivate::get(state)->stateType == QAbstractStatePrivate::HistoryState)) {
      return static_cast<QHistoryState *>(state);
   }

   return nullptr;
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

bool QStateMachinePrivate::hasRestorable(QAbstractState *state, QObject *object,
      const QString &propertyName) const
{
   RestorableId id(object, propertyName);
   return registeredRestorablesForState.value(state).contains(id);
}

QVariant QStateMachinePrivate::savedValueForRestorable(const QList<QAbstractState *> &exitedStates_sorted,
      QObject *object, const QString &propertyName) const
{
#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": savedValueForRestorable(" << exitedStates_sorted << object << propertyName << ')';
#endif

   for (int i = exitedStates_sorted.size() - 1; i >= 0; --i) {
      QAbstractState *s = exitedStates_sorted.at(i);
      QHash<RestorableId, QVariant> restorables = registeredRestorablesForState.value(s);
      QHash<RestorableId, QVariant>::const_iterator it = restorables.constFind(RestorableId(object, propertyName));

      if (it != restorables.constEnd()) {

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ":   using" << it.value() << "from" << s;
#endif

         return it.value();
      }
   }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ":   falling back to current value";
#endif

   return object->property(propertyName);
}

void QStateMachinePrivate::registerRestorable(QAbstractState *state, QObject *object, const QString &propertyName,
      const QVariant &value)
{
#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": registerRestorable(" << state << object << propertyName << value << ')';
#endif

   RestorableId id(object, propertyName);
   QHash<RestorableId, QVariant> &restorables = registeredRestorablesForState[state];

   if (! restorables.contains(id)) {
      restorables.insert(id, value);
   }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   else {
      qDebug() << q_func() << ":   (already registered)";
   }

#endif
}

void QStateMachinePrivate::unregisterRestorables(const QList<QAbstractState *> &states, QObject *object,
      const QString &propertyName)
{
#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q_func() << ": unregisterRestorables(" << states << object << propertyName << ')';
#endif

   RestorableId id(object, propertyName);

   for (int i = 0; i < states.size(); ++i) {
      QAbstractState *s = states.at(i);
      QHash<QAbstractState *, QHash<RestorableId, QVariant>>::iterator it;
      it = registeredRestorablesForState.find(s);

      if (it == registeredRestorablesForState.end()) {
         continue;
      }

      QHash<RestorableId, QVariant> &restorables = it.value();
      QHash<RestorableId, QVariant>::iterator it2;
      it2 = restorables.find(id);

      if (it2 == restorables.end()) {
         continue;
      }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
      qDebug() << q_func() << ":   unregistered for" << s;
#endif
      restorables.erase(it2);

      if (restorables.isEmpty()) {
         registeredRestorablesForState.erase(it);
      }
   }
}

QVector<QPropertyAssignment> QStateMachinePrivate::restorablesToPropertyList(const QHash<RestorableId, QVariant> &restorables) const
{
   QVector<QPropertyAssignment> result;
   QHash<RestorableId, QVariant>::const_iterator it;

   for (it = restorables.constBegin(); it != restorables.constEnd(); ++it) {
      const RestorableId &id = it.key();

      if (! id.object()) {
         // Property object was deleted
         continue;
      }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
      qDebug() << q_func() << ": restoring" << id.object() << id.propertyName() << "to" << it.value();
#endif

      result.append(QPropertyAssignment(id.object(), id.propertyName(), it.value(), false));
   }

   return result;
}

QHash<QStateMachinePrivate::RestorableId, QVariant> QStateMachinePrivate::computePendingRestorables(
      const QList<QAbstractState *> &statesToExit_sorted) const
{
   QHash<QStateMachinePrivate::RestorableId, QVariant> restorables;

   for (int i = statesToExit_sorted.size() - 1; i >= 0; --i) {
      QAbstractState *s = statesToExit_sorted.at(i);
      QHash<QStateMachinePrivate::RestorableId, QVariant> rs = registeredRestorablesForState.value(s);
      QHash<QStateMachinePrivate::RestorableId, QVariant>::const_iterator it;

      for (it = rs.constBegin(); it != rs.constEnd(); ++it) {
         if (! restorables.contains(it.key())) {
            restorables.insert(it.key(), it.value());
         }
      }
   }

   return restorables;
}
QHash<QAbstractState *, QVector<QPropertyAssignment>> QStateMachinePrivate::computePropertyAssignments(
      const QList<QAbstractState *> &statesToEnter_sorted, QHash<RestorableId, QVariant> &pendingRestorables) const
{
   QHash<QAbstractState *, QVector<QPropertyAssignment>> assignmentsForState;

   for (int i = 0; i < statesToEnter_sorted.size(); ++i) {
      QState *s = toStandardState(statesToEnter_sorted.at(i));

      if (! s) {
         continue;
      }

      QVector<QPropertyAssignment> &assignments = QStatePrivate::get(s)->propertyAssignments;

      for (int j = 0; j < assignments.size(); ++j) {
         const QPropertyAssignment &assn = assignments.at(j);

         if (assn.objectDeleted()) {
            assignments.removeAt(j--);
         } else {
            pendingRestorables.remove(RestorableId(assn.object, assn.propertyName));
            assignmentsForState[s].append(assn);
         }
      }
   }

   return assignmentsForState;
}

QAbstractState *QStateMachinePrivate::findErrorState(QAbstractState *context)
{
   // Find error state recursively in parent hierarchy if not set explicitly for context state
   QAbstractState *errorState = nullptr;

   if (context != nullptr) {
      QState *s = toStandardState(context);

      if (s != nullptr) {
         errorState = s->errorState();
      }

      if (errorState == nullptr) {
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
         Q_ASSERT(currentContext != nullptr);

         errorString = QStateMachine::tr("Missing initial state in compound state '%1'")
               .formatArg(currentContext->objectName());
         break;

      case QStateMachine::NoDefaultStateInHistoryStateError:
         Q_ASSERT(currentContext != nullptr);

         errorString = QStateMachine::tr("Missing default state in history state '%1'")
               .formatArg(currentContext->objectName());
         break;

      case QStateMachine::NoCommonAncestorForTransitionError:
         Q_ASSERT(currentContext != nullptr);

         errorString = QStateMachine::tr("No common ancestor for targets and source of transition from state '%1'")
               .formatArg(currentContext->objectName());
         break;

      default:
         errorString = QStateMachine::tr("Unknown error");
   };

   pendingErrorStates.clear();

   pendingErrorStatesForDefaultEntry.clear();

   QAbstractState *currentErrorState = findErrorState(currentContext);

   // Avoid infinite loop if the error state itself has an error
   if (currentContext == currentErrorState) {
      currentErrorState = nullptr;
   }

   Q_ASSERT(currentErrorState != rootState());

   if (currentErrorState != nullptr) {
      pendingErrorStates.insert(currentErrorState);
      addDescendantStatesToEnter(currentErrorState, pendingErrorStates, pendingErrorStatesForDefaultEntry);
      addAncestorStatesToEnter(currentErrorState, rootState(), pendingErrorStates, pendingErrorStatesForDefaultEntry);

      for (QAbstractState *s : configuration) {
         pendingErrorStates.remove(s);
      }

   } else {
      qWarning("QStateMachine::setError() Unrecoverable error detected while running state machine, %s",
            csPrintable(errorString));

      q->stop();
   }
}

#ifndef QT_NO_ANIMATION

QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *>> QStateMachinePrivate::initializeAnimation(
      QAbstractAnimation *abstractAnimation, const QPropertyAssignment &prop)
{
   QList<QAbstractAnimation *> handledAnimations;
   QList<QAbstractAnimation *> localResetEndValues;
   QAnimationGroup *group = qobject_cast<QAnimationGroup *>(abstractAnimation);

   if (group) {
      for (int i = 0; i < group->animationCount(); ++i) {
         QAbstractAnimation *animationChild = group->animationAt(i);
         QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *>> ret;
         ret = initializeAnimation(animationChild, prop);
         handledAnimations << ret.first;
         localResetEndValues << ret.second;
      }

   } else {
      QPropertyAnimation *animation = qobject_cast<QPropertyAnimation *>(abstractAnimation);

      if (animation != nullptr && prop.object == animation->targetObject()
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
   Q_ASSERT(anim != nullptr);
   QObject::disconnect(anim, &QAbstractAnimation::finished, q, &QStateMachine::_q_animationFinished);

   if (resetAnimationEndValues.contains(anim)) {
      qobject_cast<QVariantAnimation *>(anim)->setEndValue(QVariant()); // ### generalize
      resetAnimationEndValues.remove(anim);
   }

   QAbstractState *state = stateForAnimation.take(anim);
   Q_ASSERT(state != nullptr);

   // Set the final property value.
   QPropertyAssignment assn = propertyForAnimation.take(anim);
   assn.write();

   if (! assn.explicitlySet) {
      unregisterRestorables(QList<QAbstractState *>() << state, assn.object, assn.propertyName);
   }

   QHash<QAbstractState *, QList<QAbstractAnimation *>>::iterator it;
   it = animationsForState.find(state);

   Q_ASSERT(it != animationsForState.end());

   QList<QAbstractAnimation *> &animations = it.value();
   animations.removeOne(anim);

   if (animations.isEmpty()) {
      animationsForState.erase(it);
      QStatePrivate::get(toStandardState(state))->emitPropertiesAssigned();
   }
}

QList<QAbstractAnimation *> QStateMachinePrivate::selectAnimations(const QList<QAbstractTransition *> &transitionList) const
{
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

   return selectedAnimations;
}

void QStateMachinePrivate::terminateActiveAnimations(QAbstractState *state,
      const QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates)
{
   Q_Q(QStateMachine);
   QList<QAbstractAnimation *> animations = animationsForState.take(state);

   for (int i = 0; i < animations.size(); ++i) {
      QAbstractAnimation *anim = animations.at(i);
      QObject::disconnect(anim, &QAbstractAnimation::finished, q, &QStateMachine::_q_animationFinished);
      stateForAnimation.remove(anim);

      // Stop the (top-level) animation, stopping nested animation has weird behavior
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
      Q_ASSERT(assn.object != nullptr);
      // If there is no property assignment that sets this property,
      // set the property to its target value.
      bool found = false;
      QHash<QAbstractState *, QVector<QPropertyAssignment>>::const_iterator it;

      for (it = assignmentsForEnteredStates.constBegin(); it != assignmentsForEnteredStates.constEnd(); ++it) {
         const QVector<QPropertyAssignment> &assignments = it.value();

         for (int j = 0; j < assignments.size(); ++j) {
            if (assignments.at(j).hasTarget(assn.object, assn.propertyName)) {
               found = true;
               break;
            }
         }
      }

      if (! found) {
         assn.write();

         if (! assn.explicitlySet) {
            unregisterRestorables(QList<QAbstractState *>() << state, assn.object, assn.propertyName);
         }
      }
   }
}

void QStateMachinePrivate::initializeAnimations(QAbstractState *state, const QList<QAbstractAnimation *> &selectedAnimations,
      const QList<QAbstractState *> &exitedStates_sorted,
      QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates)
{
   Q_Q(QStateMachine);

   if (! assignmentsForEnteredStates.contains(state)) {
      return;
   }

   QVector<QPropertyAssignment> &assignments = assignmentsForEnteredStates[state];

   for (int i = 0; i < selectedAnimations.size(); ++i) {
      QAbstractAnimation *anim = selectedAnimations.at(i);
      QVector<QPropertyAssignment>::iterator it;

      for (it = assignments.begin(); it != assignments.end(); ) {
         QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *>> ret;
         const QPropertyAssignment &assn = *it;
         ret = initializeAnimation(anim, assn);

         QList<QAbstractAnimation *> handlers = ret.first;

         if (! handlers.isEmpty()) {
            for (int j = 0; j < handlers.size(); ++j) {
               QAbstractAnimation *a = handlers.at(j);
               propertyForAnimation.insert(a, assn);
               stateForAnimation.insert(a, state);
               animationsForState[state].append(a);

               // ### connect to just the top-level animation?
               QObject::connect(a, &QAbstractAnimation::finished, q, &QStateMachine::_q_animationFinished, Qt::UniqueConnection);
            }

            if ((globalRestorePolicy == QState::RestoreProperties)
                  && !hasRestorable(state, assn.object, assn.propertyName)) {
               QVariant value = savedValueForRestorable(exitedStates_sorted, assn.object, assn.propertyName);
               unregisterRestorables(exitedStates_sorted, assn.object, assn.propertyName);
               registerRestorable(state, assn.object, assn.propertyName, value);
            }

            it = assignments.erase(it);

         } else {
            ++it;
         }

         for (int j = 0; j < ret.second.size(); ++j) {
            resetAnimationEndValues.insert(ret.second.at(j));
         }
      }

      // require that at least one animation is valid
      QList<QVariantAnimation *> variantAnims = anim->findChildren<QVariantAnimation *>();

      if (QVariantAnimation *va = qobject_cast<QVariantAnimation * >(anim)) {
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

      if (assignments.isEmpty()) {
         assignmentsForEnteredStates.remove(state);
         break;
      }
   }
}

#endif // ! QT_NO_ANIMATION

QAbstractTransition *QStateMachinePrivate::createInitialTransition() const
{
   class InitialTransition : public QAbstractTransition
   {
    public:
      InitialTransition(const QList<QAbstractState *> &targets)
         : QAbstractTransition() {
         setTargetStates(targets);
      }

    protected:
      bool eventTest(QEvent *) override {
         return true;
      }

      void onTransition(QEvent *) override {
      }
   };

   QState *root = rootState();
   Q_ASSERT(root != nullptr);
   QList<QAbstractState *> targets;

   switch (root->childMode()) {
      case QState::ExclusiveStates:
         targets.append(root->initialState());
         break;

      case QState::ParallelStates:
         targets = QStatePrivate::get(root)->childStates();
         break;
   }

   return new InitialTransition(targets);
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

void QStateMachinePrivate::registerMultiThreadedSignalTransitions()
{
   Q_Q(QStateMachine);
   QList<QSignalTransition *> transitions = rootState()->findChildren<QSignalTransition *>();

   for (int i = 0; i < transitions.size(); ++i) {
      QSignalTransition *t = transitions.at(i);

      if ((t->machine() == q) && t->senderObject() && (t->senderObject()->thread() != q->thread())) {
         registerSignalTransition(t);
      }
   }
}

void QStateMachinePrivate::_q_start()
{
   Q_Q(QStateMachine);
   Q_ASSERT(state == Starting);

   for (QAbstractState *state : configuration) {
      QAbstractStatePrivate *abstractStatePrivate = QAbstractStatePrivate::get(state);
      abstractStatePrivate->active = false;
      emit state->activeChanged(false);
   }

   configuration.clear();

   qDeleteAll(internalEventQueue);
   internalEventQueue.clear();
   qDeleteAll(externalEventQueue);

   externalEventQueue.clear();
   clearHistory();

   registerMultiThreadedSignalTransitions();

   startupHook();

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q << ": starting";
#endif

   state = Running;
   processingScheduled = true; // we call _q_process() below

   QList<QAbstractTransition *> transitions;
   CalculationCache calculationCache;
   QAbstractTransition *initialTransition = createInitialTransition();
   transitions.append(initialTransition);

   QEvent nullEvent(QEvent::None);
   executeTransitionContent(&nullEvent, transitions);
   QList<QAbstractState *> exitedStates = QList<QAbstractState *>();
   QSet<QAbstractState *> statesForDefaultEntry;
   QList<QAbstractState *> enteredStates = computeEntrySet(transitions, statesForDefaultEntry, &calculationCache);
   QHash<RestorableId, QVariant> pendingRestorables;

   QHash<QAbstractState *, QVector<QPropertyAssignment>> assignmentsForEnteredStates =
         computePropertyAssignments(enteredStates, pendingRestorables);

#ifndef QT_NO_ANIMATION
   QList<QAbstractAnimation *> selectedAnimations = selectAnimations(transitions);
#endif

   // enterStates() will set stopProcessingReason to Finished if a final state is entered
   stopProcessingReason = EventQueueEmpty;

   enterStates(&nullEvent, exitedStates, enteredStates, statesForDefaultEntry, assignmentsForEnteredStates
#ifndef QT_NO_ANIMATION
         , selectedAnimations
#endif
   );

   delete initialTransition;

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q << ": initial configuration:" << configuration;
#endif

   emit q->started();
   emit q->runningChanged(true);

   if (stopProcessingReason == Finished) {
      // The state machine immediately reached a final state.
      processingScheduled = false;
      state = NotRunning;
      unregisterAllTransitions();
      emitFinished();
      emit q->runningChanged(false);
      exitInterpreter();
   } else {
      _q_process();
   }
}

void QStateMachinePrivate::_q_process()
{
   Q_Q(QStateMachine);
   Q_ASSERT(state == Running);
   Q_ASSERT(!processing);

   processing = true;
   processingScheduled = false;
   beginMacrostep();

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q << ": starting the event processing loop";
#endif

   bool didChange = false;

   while (processing) {
      if (stop) {
         processing = false;
         break;
      }

      QList<QAbstractTransition *> enabledTransitions;
      CalculationCache calculationCache;
      QEvent *e = new QEvent(QEvent::None);
      enabledTransitions = selectTransitions(e, &calculationCache);

      if (enabledTransitions.isEmpty()) {
         delete e;
         e = nullptr;
      }

      while (enabledTransitions.isEmpty() && ((e = dequeueInternalEvent()) != nullptr)) {

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
         qDebug() << q << ": dequeued internal event" << e << "of type" << e->type();
#endif
         enabledTransitions = selectTransitions(e, &calculationCache);

         if (enabledTransitions.isEmpty()) {
            delete e;
            e = nullptr;
         }
      }

      while (enabledTransitions.isEmpty() && ((e = dequeueExternalEvent()) != nullptr)) {

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
         qDebug() << q << ": dequeued external event" << e << "of type" << e->type();
#endif
         enabledTransitions = selectTransitions(e, &calculationCache);

         if (enabledTransitions.isEmpty()) {
            delete e;
            e = nullptr;
         }
      }

      if (enabledTransitions.isEmpty()) {
         if (isInternalEventQueueEmpty()) {
            processing = false;
            stopProcessingReason = EventQueueEmpty;
            noMicrostep();
         }

      } else {
         didChange = true;

         q->beginMicrostep(e);
         microstep(e, enabledTransitions, &calculationCache);
         q->endMicrostep(e);
      }

      delete e;
   }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q << ": finished the event processing loop";
#endif

   if (stop) {
      stop = false;
      stopProcessingReason = Stopped;
   }

   switch (stopProcessingReason) {
      case EventQueueEmpty:
         processedPendingEvents(didChange);
         break;

      case Finished:
         state = NotRunning;
         cancelAllDelayedEvents();
         unregisterAllTransitions();

         emitFinished();
         emit q->runningChanged(false);
         break;

      case Stopped:
         state = NotRunning;
         cancelAllDelayedEvents();
         unregisterAllTransitions();

         emit q->stopped();
         emit q->runningChanged(false);
         break;
   }

   endMacrostep(didChange);

   if (stopProcessingReason == Finished) {
      exitInterpreter();
   }
}

void QStateMachinePrivate::_q_startDelayedEventTimer(int id, int delay)
{
   Q_Q(QStateMachine);

   QMutexLocker locker(&delayedEventsMutex);
   QHash<int, DelayedEvent>::iterator it = delayedEvents.find(id);

   if (it != delayedEvents.end()) {
      DelayedEvent &e = it.value();
      Q_ASSERT(!e.timerId);
      e.timerId = q->startTimer(delay);

      if (! e.timerId) {
         qWarning("QStateMachine::_q_startDelayedEventTimer() Failed to start timer (id=%d, delay=%d)", id, delay);
         delete e.event;
         delayedEvents.erase(it);
         delayedEventIdFreeList.release(id);
      } else {
         timerIdToDelayedEventId.insert(e.timerId, id);
      }

   } else {
      // it has been cancelled already
      delayedEventIdFreeList.release(id);
   }
}

void QStateMachinePrivate::_q_killDelayedEventTimer(int id, int timerId)
{
   Q_Q(QStateMachine);

   q->killTimer(timerId);
   QMutexLocker locker(&delayedEventsMutex);
   delayedEventIdFreeList.release(id);
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

void QStateMachine::_q_startDelayedEventTimer(int id, int delay)
{
   Q_D(QStateMachine);
   d->_q_startDelayedEventTimer(id, delay);
}

void QStateMachine::_q_killDelayedEventTimer(int id, int timerId)
{
   Q_D(QStateMachine);
   d->_q_killDelayedEventTimer(id, timerId);
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
      return nullptr;
   }

   return internalEventQueue.takeFirst();
}

QEvent *QStateMachinePrivate::dequeueExternalEvent()
{
   QMutexLocker locker(&externalEventMutex);

   if (externalEventQueue.isEmpty()) {
      return nullptr;
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
         }

         // processing must be done in the machine thread
         [[fallthrough]];

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
   QHash<int, DelayedEvent>::const_iterator it;

   for (it = delayedEvents.constBegin(); it != delayedEvents.constEnd(); ++it) {

      const DelayedEvent &e = it.value();

      if (e.timerId) {
         timerIdToDelayedEventId.remove(e.timerId);
         q->killTimer(e.timerId);
         delayedEventIdFreeList.release(it.key());

      } else {
         // Cancellation will be detected in pending _q_startDelayedEventTimer() call
      }

      delete e.event;
   }

   delayedEvents.clear();
}

void QStateMachinePrivate::noMicrostep()
{
}

void QStateMachinePrivate::processedPendingEvents(bool)
{
}

void QStateMachinePrivate::beginMacrostep()
{
}

void QStateMachinePrivate::endMacrostep(bool)
{
}

void QStateMachinePrivate::exitInterpreter()
{
}

void QStateMachinePrivate::emitStateFinished(QState *forState, QFinalState *guiltyState)
{
   (void) guiltyState;
   Q_ASSERT(guiltyState);

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   Q_Q(QStateMachine);
   qDebug() << q << ": emitting finished signal for" << forState;
#endif

   QStatePrivate::get(forState)->emitFinished();
}

void QStateMachinePrivate::startupHook()
{
}

namespace cs_internal_stateMachine {

class GoToStateTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(GoToStateTransition)

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

using namespace cs_internal_stateMachine;

void QStateMachinePrivate::goToState(QAbstractState *targetState)
{
   if (! targetState) {
      qWarning("QStateMachine::goToState() Unable to transition to an invalid state (nullptr)");
      return;
   }

   if (configuration.contains(targetState)) {
      return;
   }

   Q_ASSERT(state == Running);
   QState *sourceState = nullptr;

   QSet<QAbstractState *>::const_iterator it;

   for (it = configuration.constBegin(); it != configuration.constEnd(); ++it) {
      sourceState = toStandardState(*it);

      if (sourceState != nullptr) {
         break;
      }
   }

   Q_ASSERT(sourceState != nullptr);

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
      registerTransition(t);
   }
}

void QStateMachinePrivate::maybeRegisterTransition(QAbstractTransition *transition)
{
   if (QSignalTransition *st = dynamic_cast<QSignalTransition * >(transition)) {
      maybeRegisterSignalTransition(st);
   }

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   else if (QEventTransition *et = dynamic_cast<QEventTransition * >(transition)) {
      maybeRegisterEventTransition(et);
   }

#endif
}

void QStateMachinePrivate::registerTransition(QAbstractTransition *transition)
{
   if (QSignalTransition *st = dynamic_cast<QSignalTransition * >(transition)) {
      registerSignalTransition(st);
   }

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   else if (QEventTransition *oet = dynamic_cast<QEventTransition * >(transition)) {
      registerEventTransition(oet);
   }

#endif
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

void QStateMachinePrivate::maybeRegisterSignalTransition(QSignalTransition *transition)
{
   Q_Q(QStateMachine);

   if ((state == Running) && (configuration.contains(transition->sourceState())
               || (transition->senderObject() && (transition->senderObject()->thread() != q->thread())))) {
      registerSignalTransition(transition);
   }
}

void QStateMachinePrivate::registerSignalTransition(QSignalTransition *transition)
{
   Q_Q(QStateMachine);

   // signal
   const QObject *sender = transition->senderObject();

   if (! sender) {
      return;
   }

   std::unique_ptr<CsSignal::Internal::BentoAbstract> signalBento = transition->get_signalBento()->clone();

   // slot
   if (! m_signalEventGenerator) {
      m_signalEventGenerator = new QSignalEventGenerator(q);
   }

   std::unique_ptr<CSBento<void (QSignalEventGenerator::*)()>> slotBento =
         std::make_unique<CSBento<void (QSignalEventGenerator::*)()>>(&QSignalEventGenerator::execute);

   // emerald (on hold, statemachine passed data is missing, change this form CsSignal to QObject)
   CsSignal::connect(*sender, std::move(signalBento), *m_signalEventGenerator, std::move(slotBento),
         CsSignal::ConnectionKind::AutoConnection, true);
}

void QStateMachinePrivate::unregisterSignalTransition(QSignalTransition *transition)
{
   // signal
   const QObject *sender = transition->senderObject();

   if (! sender) {
      return;
   }

   Q_ASSERT(m_signalEventGenerator != nullptr);

   /*
      CsSignal::Internal::BentoAbstract *signalBento = transition->get_signalBento();

      // emerald (on hold, statemachine passed data missing)
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

   // emerald (on hold, statemachine passed data missing)
   QStateMachinePrivate::get(machine)->handleTransitionSignal(sender, sender_signalIndex);
}

#ifndef QT_NO_STATEMACHINE_EVENTFILTER

void QStateMachinePrivate::maybeRegisterEventTransition(QEventTransition *transition)
{
   if ((state == Running) && configuration.contains(transition->sourceState())) {
      registerEventTransition(transition);
   }
}

void QStateMachinePrivate::registerEventTransition(QEventTransition *transition)
{
   Q_Q(QStateMachine);

   if (QEventTransitionPrivate::get(transition)->registered) {
      return;
   }

   if (transition->eventType() >= QEvent::User) {
      qWarning("QStateMachine::registerEventTransition() Event transitions are not supported for custom types");
      return;
   }

   QObject *object = QEventTransitionPrivate::get(transition)->object;

   if (! object) {
      return;
   }

   QList<QPointer<QObject>> &eventFilters = CSInternalEvents::get_m_EventFilters(object);

   if (! eventFilters.contains(q)) {
      object->installEventFilter(q);
   }

   ++qobjectEvents[object][transition->eventType()];
   QEventTransitionPrivate::get(transition)->registered = true;

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << q << ": added event transition from" << transition->sourceState()
         << ": ( object =" << object << ", event =" << transition->eventType()
         << ", targets =" << transition->targetStates() << ')';
#endif

}

void QStateMachinePrivate::unregisterEventTransition(QEventTransition *transition)
{
   Q_Q(QStateMachine);

   if (! QEventTransitionPrivate::get(transition)->registered) {
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

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug("Sending Signal Event");
#endif

   postInternalEvent(new QStateMachine::SignalEvent(sender, signalIndex, vArgs));
   processEvents(DirectProcessing);
}

QStateMachine::QStateMachine(QObject *parent)
   : QState(*new QStateMachinePrivate, nullptr)
{
   // unable to pass the parent to the QState constructor, expects a QState
   // calling setParent works as expected, regardless of the data type
   setParent(parent);
}

QStateMachine::QStateMachine(QState::ChildMode childMode, QObject *parent)
   : QState(*new QStateMachinePrivate, nullptr)
{
   Q_D(QStateMachine);

   d->childMode = childMode;
   setParent(parent);           // See comment in constructor above
}

QStateMachine::QStateMachine(QStateMachinePrivate &dd, QObject *parent)
   : QState(dd, nullptr)
{
   setParent(parent);
}

QStateMachine::~QStateMachine()
{
}

QStateMachine::Error QStateMachine::error() const
{
   Q_D(const QStateMachine);
   return d->error;
}

QString QStateMachine::errorString() const
{
   Q_D(const QStateMachine);
   return d->errorString;
}

void QStateMachine::clearError()
{
   Q_D(QStateMachine);
   d->errorString.clear();
   d->error = NoError;
}

QState::RestorePolicy QStateMachine::globalRestorePolicy() const
{
   Q_D(const QStateMachine);
   return d->globalRestorePolicy;
}

void QStateMachine::setGlobalRestorePolicy(QState::RestorePolicy restorePolicy)
{
   Q_D(QStateMachine);
   d->globalRestorePolicy = restorePolicy;
}

void QStateMachine::addState(QAbstractState *state)
{
   if (!state) {
      qWarning("QStateMachine::addState() Unable to add invalid state (nullptr)");
      return;
   }

   if (QAbstractStatePrivate::get(state)->machine() == this) {
      qWarning("QStateMachine::addState() State has already been added to this state machine");
      return;
   }

   state->setParent(this);
}

void QStateMachine::removeState(QAbstractState *state)
{
   if (!state) {
      qWarning("QStateMachine::removeState() Unable to remove invalid state (nullptr)");
      return;
   }

   if (QAbstractStatePrivate::get(state)->machine() != this) {
      qWarning("QStateMachine::removeState() Unable to remove state from a different state machine");
      return;
   }

   state->setParent(nullptr);
}

bool QStateMachine::isRunning() const
{
   Q_D(const QStateMachine);
   return (d->state == QStateMachinePrivate::Running);
}

void QStateMachine::start()
{
   Q_D(QStateMachine);

   if ((childMode() == QState::ExclusiveStates) && (initialState() == nullptr)) {
      qWarning("QStateMachine::start() No initial state set for state machine");
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
         qWarning("QStateMachine::start() State machine is already running");
         break;
   }
}

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

void QStateMachine::setRunning(bool running)
{
   if (running) {
      start();
   } else {
      stop();
   }
}

void QStateMachine::postEvent(QEvent *event, EventPriority priority)
{
   Q_D(QStateMachine);

   switch (d->state) {

      case QStateMachinePrivate::Running:
      case QStateMachinePrivate::Starting:
         break;

      default:
         qWarning("QStateMachine::postEvent() Unable to post event when the state machine is not running");
         return;
   }

   if (! event) {
      qWarning("QStateMachine::postEvent() Unable to post invalid event (nullptr)");
      return;
   }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
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

int QStateMachine::postDelayedEvent(QEvent *event, int delay)
{
   Q_D(QStateMachine);

   if (d->state != QStateMachinePrivate::Running) {
      qWarning("QStateMachine::postDelayedEvent() Unable to post event when the state machine is not running");
      return -1;
   }

   if (!event) {
      qWarning("QStateMachine::postDelayedEvent() Unable to post invalid event (nullptr)");
      return -1;
   }

   if (delay < 0) {
      qWarning("QStateMachine::postDelayedEvent() Delay can not be negative");
      return -1;
   }

#if defined(CS_SHOW_DEBUG_CORE_STATEMACHINE)
   qDebug() << this << ": posting event" << event << "with delay" << delay;
#endif

   QMutexLocker locker(&d->delayedEventsMutex);

   int id = d->delayedEventIdFreeList.next();
   bool inMachineThread = (QThread::currentThread() == thread());
   int timerId = inMachineThread ? startTimer(delay) : 0;

   if (inMachineThread && !timerId) {
      qWarning("QStateMachine::postDelayedEvent() Failed to start timer with interval %d", delay);
      d->delayedEventIdFreeList.release(id);
      return -1;
   }

   QStateMachinePrivate::DelayedEvent delayedEvent(event, timerId);
   d->delayedEvents.insert(id, delayedEvent);

   if (timerId) {
      d->timerIdToDelayedEventId.insert(timerId, id);
   } else {
      Q_ASSERT(!inMachineThread);
      QMetaObject::invokeMethod(this, "_q_startDelayedEventTimer",
            Qt::QueuedConnection, Q_ARG(int, id), Q_ARG(int, delay));
   }

   return id;
}

bool QStateMachine::cancelDelayedEvent(int id)
{
   Q_D(QStateMachine);

   if (d->state != QStateMachinePrivate::Running) {
      qWarning("QStateMachine::cancelDelayedEvent() State machine is not running");
      return false;
   }

   QMutexLocker locker(&d->delayedEventsMutex);
   QStateMachinePrivate::DelayedEvent e = d->delayedEvents.take(id);

   if (! e.event) {
      return false;
   }

   if (e.timerId) {
      d->timerIdToDelayedEventId.remove(e.timerId);
      bool inMachineThread = (QThread::currentThread() == thread());

      if (inMachineThread) {
         killTimer(e.timerId);
         d->delayedEventIdFreeList.release(id);
      } else {
         QMetaObject::invokeMethod(this, "_q_killDelayedEventTimer",
               Qt::QueuedConnection, Q_ARG(int, id), Q_ARG(int, e.timerId));
      }

   } else {
      // Cancellation will be detected in pending _q_startDelayedEventTimer() call
   }

   delete e.event;

   return true;
}

QSet<QAbstractState *> QStateMachine::configuration() const
{
   Q_D(const QStateMachine);
   return d->configuration;
}

bool QStateMachine::event(QEvent *e)
{
   Q_D(QStateMachine);

   if (e->type() == QEvent::Timer) {
      QTimerEvent *te = static_cast<QTimerEvent *>(e);
      int tid = te->timerId();

      if (d->state != QStateMachinePrivate::Running) {
         // This event has been cancelled already
         QMutexLocker locker(&d->delayedEventsMutex);
         Q_ASSERT(!d->timerIdToDelayedEventId.contains(tid));
         return true;
      }

      d->delayedEventsMutex.lock();
      int id = d->timerIdToDelayedEventId.take(tid);

      QStateMachinePrivate::DelayedEvent ee = d->delayedEvents.take(id);

      if (ee.event != nullptr) {
         Q_ASSERT(ee.timerId == tid);

         killTimer(tid);

         d->delayedEventIdFreeList.release(id);
         d->delayedEventsMutex.unlock();
         d->postExternalEvent(ee.event);
         d->processEvents(QStateMachinePrivate::DirectProcessing);

         return true;

      } else {
         d->delayedEventsMutex.unlock();
      }
   }

   return QState::event(e);
}

#ifndef QT_NO_STATEMACHINE_EVENTFILTER

bool QStateMachine::eventFilter(QObject *watched, QEvent *event)
{
   Q_D(QStateMachine);
   d->handleFilteredEvent(watched, event);

   return false;
}
#endif

void QStateMachine::beginSelectTransitions(QEvent *event)
{
   (void) event;
}

void QStateMachine::endSelectTransitions(QEvent *event)
{
   (void) event;
}
void QStateMachine::beginMicrostep(QEvent *event)
{
   (void) event;
}

void QStateMachine::endMicrostep(QEvent *event)
{
   (void) event;
}

void QStateMachine::onEntry(QEvent *event)
{
   start();
   QState::onEntry(event);
}

void QStateMachine::onExit(QEvent *event)
{
   stop();
   QState::onExit(event);
}

#ifndef QT_NO_ANIMATION
bool QStateMachine::isAnimated() const
{
   Q_D(const QStateMachine);
   return d->animated;
}

void QStateMachine::setAnimated(bool enabled)
{
   Q_D(QStateMachine);
   d->animated = enabled;
}

void QStateMachine::addDefaultAnimation(QAbstractAnimation *animation)
{
   Q_D(QStateMachine);
   d->defaultAnimations.append(animation);
}

QList<QAbstractAnimation *> QStateMachine::defaultAnimations() const
{
   Q_D(const QStateMachine);
   return d->defaultAnimations;
}

void QStateMachine::removeDefaultAnimation(QAbstractAnimation *animation)
{
   Q_D(QStateMachine);
   d->defaultAnimations.removeAll(animation);
}
#endif

QSignalEventGenerator::QSignalEventGenerator(QStateMachine *parent)
   : QObject(parent)
{
}

QStateMachine::SignalEvent::SignalEvent(QObject *sender, int signalIndex, const QList<QVariant> &arguments)
   : QEvent(QEvent::StateMachineSignal), m_sender(sender),
     m_signalIndex(signalIndex), m_arguments(arguments)
{
}

QStateMachine::SignalEvent::~SignalEvent()
{
}

QStateMachine::WrappedEvent::WrappedEvent(QObject *object, QEvent *event)
   : QEvent(QEvent::StateMachineWrapped), m_object(object), m_event(event)
{
}

QStateMachine::WrappedEvent::~WrappedEvent()
{
   delete m_event;
}

#endif //QT_NO_STATEMACHINE
