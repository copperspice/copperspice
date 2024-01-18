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

#ifndef QSTATEMACHINE_H
#define QSTATEMACHINE_H

#include <qcoreevent.h>
#include <qlist.h>
#include <qobject.h>
#include <qstate.h>
#include <qset.h>
#include <qvariant.h>

#ifndef QT_NO_STATEMACHINE

class QStateMachinePrivate;
class QAbstractAnimation;

class Q_CORE_EXPORT QStateMachine : public QState
{
   CORE_CS_OBJECT(QStateMachine)

   CORE_CS_PROPERTY_READ(errorString, errorString)

   CORE_CS_PROPERTY_READ(globalRestorePolicy, globalRestorePolicy)
   CORE_CS_PROPERTY_WRITE(globalRestorePolicy, setGlobalRestorePolicy)

   CORE_CS_PROPERTY_READ(running, isRunning)
   CORE_CS_PROPERTY_WRITE(running, setRunning)
   CORE_CS_PROPERTY_NOTIFY(running, runningChanged)

#ifndef QT_NO_ANIMATION
   CORE_CS_PROPERTY_READ(animated, isAnimated)
   CORE_CS_PROPERTY_WRITE(animated, setAnimated)
#endif

 public:

   class Q_CORE_EXPORT SignalEvent : public QEvent
   {
    public:
      SignalEvent(QObject *sender, int signalIndex, const QList<QVariant> &arguments);
      ~SignalEvent();

      QObject *sender() const {
         return m_sender;
      }

      int signalIndex() const {
         return m_signalIndex;
      }

      QList<QVariant> arguments() const {
         return m_arguments;
      }

    private:
      QObject *m_sender;
      int m_signalIndex;
      QList<QVariant> m_arguments;

      friend class QSignalTransition;
   };

   class Q_CORE_EXPORT WrappedEvent : public QEvent
   {
    public:
      WrappedEvent(QObject *object, QEvent *event);
      ~WrappedEvent();

      QObject *object() const {
         return m_object;
      }

      QEvent *event() const {
         return m_event;
      }

    private:
      QObject *m_object;
      QEvent *m_event;
   };

   enum EventPriority {
      NormalPriority,
      HighPriority
   };

   enum Error {
      NoError,
      NoInitialStateError,
      NoDefaultStateInHistoryStateError,
      NoCommonAncestorForTransitionError
   };

   explicit QStateMachine(QObject *parent = nullptr);
   explicit QStateMachine(QState::ChildMode childMode, QObject *parent = nullptr);

   QStateMachine(const QStateMachine &) = delete;
   QStateMachine &operator=(const QStateMachine &) = delete;

   ~QStateMachine();

   void addState(QAbstractState *state);
   void removeState(QAbstractState *state);

   Error error() const;
   QString errorString() const;
   void clearError();

   bool isRunning() const;

#ifndef QT_NO_ANIMATION
   bool isAnimated() const;
   void setAnimated(bool enabled);

   void addDefaultAnimation(QAbstractAnimation *animation);
   QList<QAbstractAnimation *> defaultAnimations() const;
   void removeDefaultAnimation(QAbstractAnimation *animation);
#endif

   QState::RestorePolicy globalRestorePolicy() const;
   void setGlobalRestorePolicy(QState::RestorePolicy restorePolicy);

   void postEvent(QEvent *event, EventPriority priority = NormalPriority);
   int postDelayedEvent(QEvent *event, int delay);
   bool cancelDelayedEvent(int id);

   QSet<QAbstractState *> configuration() const;

#ifndef QT_NO_STATEMACHINE_EVENTFILTER
   bool eventFilter(QObject *watched, QEvent *event) override;
#endif

   CORE_CS_SIGNAL_1(Public, void started())
   CORE_CS_SIGNAL_2(started)

   CORE_CS_SIGNAL_1(Public, void stopped())
   CORE_CS_SIGNAL_2(stopped)

   CORE_CS_SIGNAL_1(Public, void runningChanged(bool running))
   CORE_CS_SIGNAL_2(runningChanged, running)

   CORE_CS_SLOT_1(Public, void start())
   CORE_CS_SLOT_2(start)

   CORE_CS_SLOT_1(Public, void stop())
   CORE_CS_SLOT_2(stop)

   CORE_CS_SLOT_1(Public, void setRunning(bool running))
   CORE_CS_SLOT_2(setRunning)

 protected:
   void onEntry(QEvent *event) override;
   void onExit(QEvent *event) override;

   virtual void beginSelectTransitions(QEvent *event);
   virtual void endSelectTransitions(QEvent *event);

   virtual void beginMicrostep(QEvent *event);
   virtual void endMicrostep(QEvent *event);

   bool event(QEvent *event) override;

   QStateMachine(QStateMachinePrivate &dd, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QStateMachine)

   CORE_CS_SLOT_1(Private, void _q_start())
   CORE_CS_SLOT_2(_q_start)

   CORE_CS_SLOT_1(Private, void _q_process())
   CORE_CS_SLOT_2(_q_process)

#ifndef QT_NO_ANIMATION
   CORE_CS_SLOT_1(Private, void _q_animationFinished())
   CORE_CS_SLOT_2(_q_animationFinished)
#endif

   CORE_CS_SLOT_1(Private, void _q_startDelayedEventTimer(int, int))
   CORE_CS_SLOT_2(_q_startDelayedEventTimer)

   CORE_CS_SLOT_1(Private, void _q_killDelayedEventTimer(int, int))
   CORE_CS_SLOT_2(_q_killDelayedEventTimer)
};

#endif //QT_NO_STATEMACHINE

#endif
