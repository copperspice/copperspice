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

#ifndef QSTATEMACHINE_P_H
#define QSTATEMACHINE_P_H

#include <qcoreevent.h>
#include <qhash.h>
#include <qlist.h>
#include <qmultihash.h>
#include <qmutex.h>
#include <qpair.h>
#include <qpointer.h>
#include <qset.h>
#include <qvector.h>

#include <qfreelist_p.h>
#include <qstate_p.h>

class QEvent;

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
class QEventTransition;
#endif

class QSignalEventGenerator;
class QSignalTransition;
class QAbstractState;
class QAbstractTransition;
class QFinalState;
class QHistoryState;
class QState;
class QStateMachine;

#ifndef QT_NO_ANIMATION
class QAbstractAnimation;
#endif

struct CalculationCache;
class Q_CORE_EXPORT QStateMachinePrivate : public QStatePrivate
{
 public:
   enum State {
      NotRunning,
      Starting,
      Running
   };

   enum EventProcessingMode {
      DirectProcessing,
      QueuedProcessing
   };

   enum StopProcessingReason {
      EventQueueEmpty,
      Finished,
      Stopped
   };

   QStateMachinePrivate();
   ~QStateMachinePrivate();

   static QStateMachinePrivate *get(QStateMachine *q);

   QState *findLCA(const QList<QAbstractState *> &states, bool onlyCompound = false) const;
   QState *findLCCA(const QList<QAbstractState *> &states) const;

   static bool transitionStateEntryLessThan(QAbstractTransition *t1, QAbstractTransition *t2);
   static bool stateEntryLessThan(QAbstractState *s1, QAbstractState *s2);
   static bool stateExitLessThan(QAbstractState *s1, QAbstractState *s2);

   QAbstractState *findErrorState(QAbstractState *context);
   void setError(QStateMachine::Error error, QAbstractState *currentContext);

   void _q_start();
   void _q_process();

#ifndef QT_NO_ANIMATION
   void _q_animationFinished();
#endif

   void _q_startDelayedEventTimer(int id, int delay);
   void _q_killDelayedEventTimer(int id, int timerId);

   QState *rootState() const;

   void clearHistory();
   QAbstractTransition *createInitialTransition() const;

   void removeConflictingTransitions(QList<QAbstractTransition *> &enabledTransitions, CalculationCache *cache);
   void microstep(QEvent *event, const QList<QAbstractTransition *> &transitionList, CalculationCache *cache);
   QList<QAbstractTransition *> selectTransitions(QEvent *event, CalculationCache *cache);

   virtual void noMicrostep();
   virtual void processedPendingEvents(bool didChange);
   virtual void beginMacrostep();
   virtual void endMacrostep(bool didChange);
   virtual void exitInterpreter();
   virtual void exitStates(QEvent *event, const QList<QAbstractState *> &statesToExit_sorted,
         const QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates);

   QList<QAbstractState *> computeExitSet(const QList<QAbstractTransition *> &enabledTransitions, CalculationCache *cache);
   QSet<QAbstractState *> computeExitSet_Unordered(const QList<QAbstractTransition *> &enabledTransitions, CalculationCache *cache);
   QSet<QAbstractState *> computeExitSet_Unordered(QAbstractTransition *t, CalculationCache *cache);
   void executeTransitionContent(QEvent *event, const QList<QAbstractTransition *> &transitionList);

#ifdef QT_NO_ANIMATION
   virtual void enterStates(QEvent *event, const QList<QAbstractState *> &exitedStates_sorted,
         const QList<QAbstractState *> &statesToEnter_sorted, const QSet<QAbstractState *> &statesForDefaultEntry,
         QHash<QAbstractState *, QVector<QPropertyAssignment>> &propertyAssignmentsForState);

#else
   virtual void enterStates(QEvent *event, const QList<QAbstractState *> &exitedStates_sorted,
         const QList<QAbstractState *> &statesToEnter_sorted, const QSet<QAbstractState *> &statesForDefaultEntry,
         QHash<QAbstractState *, QVector<QPropertyAssignment>> &propertyAssignmentsForState,
         const QList<QAbstractAnimation *> &selectedAnimations);
#endif

   QList<QAbstractState *> computeEntrySet(const QList<QAbstractTransition *> &enabledTransitions,
         QSet<QAbstractState *> &statesForDefaultEntry, CalculationCache *cache);

   QAbstractState *getTransitionDomain(QAbstractTransition *t,
         const QList<QAbstractState *> &effectiveTargetStates, CalculationCache *cache) const;

   void addDescendantStatesToEnter(QAbstractState *state,
         QSet<QAbstractState *> &statesToEnter, QSet<QAbstractState *> &statesForDefaultEntry);

   void addAncestorStatesToEnter(QAbstractState *s, QAbstractState *ancestor,
         QSet<QAbstractState *> &statesToEnter, QSet<QAbstractState *> &statesForDefaultEntry);

   static QState *toStandardState(QAbstractState *state);
   static const QState *toStandardState(const QAbstractState *state);
   static QFinalState *toFinalState(QAbstractState *state);
   static QHistoryState *toHistoryState(QAbstractState *state);

   bool isInFinalState(QAbstractState *s) const;
   static bool isFinal(const QAbstractState *s);
   static bool isParallel(const QAbstractState *s);
   bool isCompound(const QAbstractState *s) const;
   bool isAtomic(const QAbstractState *s) const;

   void goToState(QAbstractState *targetState);

   void registerTransitions(QAbstractState *state);
   void maybeRegisterTransition(QAbstractTransition *transition);
   void registerTransition(QAbstractTransition *transition);
   void maybeRegisterSignalTransition(QSignalTransition *transition);
   void registerSignalTransition(QSignalTransition *transition);
   void unregisterSignalTransition(QSignalTransition *transition);
   void registerMultiThreadedSignalTransitions();

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   void maybeRegisterEventTransition(QEventTransition *transition);
   void registerEventTransition(QEventTransition *transition);
   void unregisterEventTransition(QEventTransition *transition);
   void handleFilteredEvent(QObject *watched, QEvent *event);
#endif

   void unregisterTransition(QAbstractTransition *transition);
   void unregisterAllTransitions();
   void handleTransitionSignal(QObject *sender, int sender_signalIndex);      // CS , const TeaCupAbstract &data);

   void postInternalEvent(QEvent *e);
   void postExternalEvent(QEvent *e);
   QEvent *dequeueInternalEvent();
   QEvent *dequeueExternalEvent();
   bool isInternalEventQueueEmpty();
   bool isExternalEventQueueEmpty();
   void processEvents(EventProcessingMode processingMode);
   void cancelAllDelayedEvents();

   virtual void emitStateFinished(QState *forState, QFinalState *guiltyState);
   virtual void startupHook();

#ifndef QT_NO_PROPERTIES
   class RestorableId
   {
      QPointer<QObject> guard;
      QObject *obj;
      QString prop;

      // two overloads because friends can't have default arguments
      friend uint qHash(const RestorableId &key, uint seed) {
         return qHash(qMakePair(key.obj, key.prop), seed);
      }

      friend uint qHash(const RestorableId &key) {
         return qHash(key, 0U);
      }

      friend bool operator==(const RestorableId &lhs, const RestorableId &rhs) {
         return lhs.obj == rhs.obj && lhs.prop == rhs.prop;
      }

      friend bool operator!=(const RestorableId &lhs, const RestorableId &rhs) {
         return !operator==(lhs, rhs);
      }

    public:
      explicit RestorableId(QObject *o, QString p)
         : guard(o), obj(o), prop(std::move(p))
      { }

      QObject *object() const {
         return guard;
      }

      QString propertyName() const {
         return prop;
      }
   };

   QHash<QAbstractState *, QHash<RestorableId, QVariant>> registeredRestorablesForState;
   bool hasRestorable(QAbstractState *state, QObject *object, const QString &propertyName) const;

   QVariant savedValueForRestorable(const QList<QAbstractState *> &exitedStates_sorted,
         QObject *object, const QString &propertyName) const;

   void registerRestorable(QAbstractState *state, QObject *object, const QString &propertyName, const QVariant &value);
   void unregisterRestorables(const QList<QAbstractState *> &states, QObject *object, const QString &propertyName);

   QVector<QPropertyAssignment> restorablesToPropertyList(const QHash<RestorableId, QVariant> &restorables) const;
   QHash<RestorableId, QVariant> computePendingRestorables(const QList<QAbstractState *> &statesToExit_sorted) const;

   QHash<QAbstractState *, QVector<QPropertyAssignment>>
   computePropertyAssignments(const QList<QAbstractState *> &statesToEnter_sorted,
         QHash<RestorableId, QVariant> &pendingRestorables) const;
#endif

   State state;

   bool processing;
   bool processingScheduled;
   bool stop;

   StopProcessingReason stopProcessingReason;

   QSet<QAbstractState *> configuration;
   QList<QEvent *> internalEventQueue;
   QList<QEvent *> externalEventQueue;
   QMutex internalEventMutex;
   QMutex externalEventMutex;

   QStateMachine::Error error;
   QState::RestorePolicy globalRestorePolicy;

   QString errorString;
   QSet<QAbstractState *> pendingErrorStates;
   QSet<QAbstractState *> pendingErrorStatesForDefaultEntry;

#ifndef QT_NO_ANIMATION
   bool animated;

   QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *>> initializeAnimation(QAbstractAnimation *abstractAnimation,
         const QPropertyAssignment &prop);

   QHash<QAbstractState *, QList<QAbstractAnimation *>> animationsForState;
   QHash<QAbstractAnimation *, QPropertyAssignment> propertyForAnimation;
   QHash<QAbstractAnimation *, QAbstractState *> stateForAnimation;
   QSet<QAbstractAnimation *> resetAnimationEndValues;

   QList<QAbstractAnimation *> defaultAnimations;
   QMultiHash<QAbstractState *, QAbstractAnimation *> defaultAnimationsForSource;
   QMultiHash<QAbstractState *, QAbstractAnimation *> defaultAnimationsForTarget;

   QList<QAbstractAnimation *> selectAnimations(const QList<QAbstractTransition *> &transitionList) const;

   void terminateActiveAnimations(QAbstractState *state,
         const QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates);

   void initializeAnimations(QAbstractState *state, const QList<QAbstractAnimation *> &selectedAnimations,
         const QList<QAbstractState *> &exitedStates_sorted,
         QHash<QAbstractState *, QVector<QPropertyAssignment>> &assignmentsForEnteredStates);
#endif

   QSignalEventGenerator *m_signalEventGenerator;

   QHash<const QObject *, QVector<int>> connections;
   QMutex connectionsMutex;

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   QHash<QObject *, QHash<QEvent::Type, int>> qobjectEvents;
#endif

   QFreeList<void> delayedEventIdFreeList;

   struct DelayedEvent {
      QEvent *event;
      int timerId;

      DelayedEvent(QEvent *e, int tid)
         : event(e), timerId(tid)
      { }

      DelayedEvent()
         : event(nullptr), timerId(0)
      { }
   };

   QHash<int, DelayedEvent> delayedEvents;
   QHash<int, int> timerIdToDelayedEventId;
   QMutex delayedEventsMutex;

   using f_cloneEvent = QEvent *(*)(QEvent *);

   struct Handler {
      f_cloneEvent cloneEvent;
   };

   static const Handler *handler;

 private:
   Q_DECLARE_PUBLIC(QStateMachine)
};

Q_CORE_EXPORT const QStateMachinePrivate::Handler *qcoreStateMachineHandler();

#endif
