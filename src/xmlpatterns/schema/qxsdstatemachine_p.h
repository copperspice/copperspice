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

#ifndef QXsdStateMachine_P_H
#define QXsdStateMachine_P_H

#include <qnamepool_p.h>
#include <QHash>
#include <QSet>
#include <QTextStream>

class QIODevice;

namespace QPatternist {

template <typename TransitionType>
class XsdStateMachine
{
 public:
   typedef qint32 StateId;

   enum StateType {
      StartState,     // The state the machine will start with.
      StartEndState,  // The state the machine will start with, can be end state as well.
      InternalState,  // Any state that is not start or end state.
      EndState        // Any state where the machine is allowed to stop.
   };

   XsdStateMachine();

   XsdStateMachine(const NamePool::Ptr &namePool);

   StateId addState(StateType type);

   void addTransition(StateId start, TransitionType transition, StateId end);
   void addEpsilonTransition(StateId start, StateId end);

   void reset();
   void clear();

   bool proceed(TransitionType transition);

   QList<TransitionType> possibleTransitions() const;

   template <typename InputType>
   bool proceed(InputType input);

   template <typename InputType>
   bool inputEqualsTransition(InputType input, TransitionType transition) const;

   bool inEndState() const;
   TransitionType lastTransition() const;
   StateId startState() const;

   QString transitionTypeToString(TransitionType type) const;

   bool outputGraph(QIODevice *device, const QString &graphName) const;

   XsdStateMachine<TransitionType> toDFA() const;

   QHash<StateId, StateType> states() const;

   QHash<StateId, QHash<TransitionType, QVector<StateId> > > transitions() const {
      return m_transitions;
   }

 private:
   StateId dfaStateForNfaState(QSet<StateId> nfaState, QList< QPair< QSet<StateId>, StateId> > &stateTable,
         XsdStateMachine<TransitionType> &dfa) const;

   // implementation was inlined to workaround a compiler bug on Symbian/winscw.
   QSet<StateId> epsilonClosure(const QSet<StateId> &input) const {
      // every state can reach itself by epsilon transition, so include the input states in the result as well
      QSet<StateId> result = input;

      // add the input states to the list of to be processed states
      QList<StateId> workStates = input.toList();

      while (!workStates.isEmpty()) {
         // while there are states to be processed

         // dequeue one state from list
         const StateId state = workStates.takeFirst();

         // get the list of states that can be reached by the epsilon transition
         // from the current 'state'

         const QVector<StateId> targetStates = m_epsilonTransitions.value(state);
         for (int i = 0; i < targetStates.count(); ++i) {
            // if we have this target state not in our result set yet

            if (!result.contains(targetStates.at(i))) {
               // ... add it to the result set
               result.insert(targetStates.at(i));

               // add the target state to the list of to be processed states as well,
               // as we want to have the epsilon transitions not only for the first
               // level of following states
               workStates.append(targetStates.at(i));
            }
         }
      }

      return result;
   }

   QSet<StateId> move(const QSet<StateId> &states, TransitionType input) const {
      QSet<StateId> result;

      QSetIterator<StateId> it(states);
      while (it.hasNext()) { // iterate over all given states
         const StateId state = it.next();

         // get the transition table for the current state
         const QHash<TransitionType, QVector<StateId> > transitions = m_transitions.value(state);

         // get the target states for the given input
         const QVector<StateId> targetStates = transitions.value(input);

         // add all target states to the result
         for (int i = 0; i < targetStates.size(); ++i) {
            result.insert(targetStates.at(i));
         }
      }

      return result;
   }

   NamePool::Ptr                                             m_namePool;
   QHash<StateId, StateType>                                 m_states;
   QHash<StateId, QHash<TransitionType, QVector<StateId> > > m_transitions;
   QHash<StateId, QVector<StateId> >                         m_epsilonTransitions;
   StateId                                                   m_currentState;
   qint32                                                    m_counter;
   TransitionType                                            m_lastTransition;
};

#include "qxsdstatemachine.cpp"

}

#endif
