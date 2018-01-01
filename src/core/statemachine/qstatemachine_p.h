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

#ifndef QSTATEMACHINE_P_H
#define QSTATEMACHINE_P_H

#include <qstate_p.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qhash.h>
#include <QtCore/qmultihash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qpair.h>
#include <QtCore/qset.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

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

class Q_CORE_EXPORT QStateMachinePrivate : public QStatePrivate
{
   Q_DECLARE_PUBLIC(QStateMachine)

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

   QState *findLCA(const QList<QAbstractState *> &states) const;

   static bool stateEntryLessThan(QAbstractState *s1, QAbstractState *s2);
   static bool stateExitLessThan(QAbstractState *s1, QAbstractState *s2);

   QAbstractState *findErrorState(QAbstractState *context);
   void setError(QStateMachine::Error error, QAbstractState *currentContext);

   void _q_start();
   void _q_process();

#ifndef QT_NO_ANIMATION
   void _q_animationFinished();
#endif

   QState *rootState() const;

   QState *startState();
   void removeStartState();

   void clearHistory();

   void microstep(QEvent *event, const QList<QAbstractTransition *> &transitionList);
   bool isPreempted(const QAbstractState *s, const QSet<QAbstractTransition *> &transitions) const;
   QSet<QAbstractTransition *> selectTransitions(QEvent *event) const;
   QList<QAbstractState *> exitStates(QEvent *event, const QList<QAbstractTransition *> &transitionList);
   void executeTransitionContent(QEvent *event, const QList<QAbstractTransition *> &transitionList);
   QList<QAbstractState *> enterStates(QEvent *event, const QList<QAbstractTransition *> &enabledTransitions);
   void addStatesToEnter(QAbstractState *s, QState *root,
                         QSet<QAbstractState *> &statesToEnter,
                         QSet<QAbstractState *> &statesForDefaultEntry);

   void applyProperties(const QList<QAbstractTransition *> &transitionList,
                        const QList<QAbstractState *> &exitedStates,
                        const QList<QAbstractState *> &enteredStates);

   static QState *toStandardState(QAbstractState *state);
   static const QState *toStandardState(const QAbstractState *state);
   static QFinalState *toFinalState(QAbstractState *state);
   static QHistoryState *toHistoryState(QAbstractState *state);

   bool isInFinalState(QAbstractState *s) const;
   static bool isFinal(const QAbstractState *s);
   static bool isParallel(const QAbstractState *s);
   bool isCompound(const QAbstractState *s) const;
   bool isAtomic(const QAbstractState *s) const;
   static bool isDescendantOf(const QAbstractState *s, const QAbstractState *other);
   static QList<QState *> properAncestors(const QAbstractState *s, const QState *upperBound);

   void goToState(QAbstractState *targetState);

   void registerTransitions(QAbstractState *state);
   void registerSignalTransition(QSignalTransition *transition);
   void unregisterSignalTransition(QSignalTransition *transition);

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   void registerEventTransition(QEventTransition *transition);
   void unregisterEventTransition(QEventTransition *transition);
   void handleFilteredEvent(QObject *watched, QEvent *event);
#endif

   void unregisterTransition(QAbstractTransition *transition);
   void unregisterAllTransitions();
   void handleTransitionSignal(QObject *sender, int sender_signalIndex);      // , const TeaCupAbstract &data);

   void postInternalEvent(QEvent *e);
   void postExternalEvent(QEvent *e);
   QEvent *dequeueInternalEvent();
   QEvent *dequeueExternalEvent();
   bool isInternalEventQueueEmpty();
   bool isExternalEventQueueEmpty();
   void processEvents(EventProcessingMode processingMode);
   void cancelAllDelayedEvents();

#ifndef QT_NO_PROPERTIES
   typedef QPair<QObject *, QByteArray> RestorableId;
   QHash<RestorableId, QVariant> registeredRestorables;
   void registerRestorable(QObject *object, const QByteArray &propertyName);
   void unregisterRestorable(QObject *object, const QByteArray &propertyName);
   bool hasRestorable(QObject *object, const QByteArray &propertyName) const;
   QVariant restorableValue(QObject *object, const QByteArray &propertyName) const;
   QList<QPropertyAssignment> restorablesToPropertyList(const QHash<RestorableId, QVariant> &restorables) const;
#endif

   State state;
   QState *_startState;
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
   QStateMachine::RestorePolicy globalRestorePolicy;

   QString errorString;
   QSet<QAbstractState *> pendingErrorStates;
   QSet<QAbstractState *> pendingErrorStatesForDefaultEntry;

#ifndef QT_NO_ANIMATION
   bool animated;

   QPair<QList<QAbstractAnimation *>, QList<QAbstractAnimation *> >
   initializeAnimation(QAbstractAnimation *abstractAnimation, const QPropertyAssignment &prop);

   QHash<QAbstractState *, QList<QAbstractAnimation *> > animationsForState;
   QHash<QAbstractAnimation *, QPropertyAssignment> propertyForAnimation;
   QHash<QAbstractAnimation *, QAbstractState *> stateForAnimation;
   QSet<QAbstractAnimation *> resetAnimationEndValues;

   QList<QAbstractAnimation *> defaultAnimations;
   QMultiHash<QAbstractState *, QAbstractAnimation *> defaultAnimationsForSource;
   QMultiHash<QAbstractState *, QAbstractAnimation *> defaultAnimationsForTarget;

#endif

   QSignalEventGenerator *m_signalEventGenerator;

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   QHash<QObject *, QHash<QEvent::Type, int> > qobjectEvents;
#endif

   QHash<int, QEvent *> delayedEvents;
   QMutex delayedEventsMutex;

   typedef QEvent *(*f_cloneEvent)(QEvent *);
   struct Handler {
      f_cloneEvent cloneEvent;
   };

   static const Handler *handler;
};

Q_CORE_EXPORT const QStateMachinePrivate::Handler *qcoreStateMachineHandler();

QT_END_NAMESPACE

#endif
