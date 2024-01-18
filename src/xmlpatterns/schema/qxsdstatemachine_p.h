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

   /**
    * Describes the type of state.
    */
   enum StateType {
      StartState,     ///< The state the machine will start with.
      StartEndState,  ///< The state the machine will start with, can be end state as well.
      InternalState,  ///< Any state that is not start or end state.
      EndState        ///< Any state where the machine is allowed to stop.
   };

   /**
    * Creates a new state machine object.
    */
   XsdStateMachine();

   /**
    * Creates a new state machine object.
    *
    * The name pool to use for accessing object names.
    */
   XsdStateMachine(const NamePool::Ptr &namePool);

   /**
    * Adds a new state of the given @p type to the state machine.
    *
    * @return The id of the new state.
    */
   StateId addState(StateType type);

   /**
    * Adds a new @p transition to the state machine.
    *
    * @param start The start state.
    * @param transition The transition to come from the start to the end state.
    * @param end The end state.
    */
   void addTransition(StateId start, TransitionType transition, StateId end);

   /**
    * Adds a new epsilon @p transition to the state machine.
    *
    * @param start The start state.
    * @param end The end state.
    */
   void addEpsilonTransition(StateId start, StateId end);

   /**
    * Resets the machine to the start state.
    */
   void reset();

   /**
    * Removes all states and transitions from the state machine.
    */
   void clear();

   /**
    * Continues execution of the machine with the given input @p transition.
    *
    * @return @c true if the transition was successful, @c false otherwise.
    */
   bool proceed(TransitionType transition);

   /**
    * Returns the list of transitions that are reachable from the current
    * state.
    */
   QList<TransitionType> possibleTransitions() const;

   /**
    * Continues execution of the machine with the given @p input.
    *
    * @note To use this method, inputEqualsTransition must be implemented
    *       to find the right transition to use.
    *
    * @return @c true if the transition was successful, @c false otherwise.
    */
   template <typename InputType>
   bool proceed(InputType input);

   /**
    * Returns whether the given @p input matches the given @p transition.
    */
   template <typename InputType>
   bool inputEqualsTransition(InputType input, TransitionType transition) const;

   /**
    * Returns whether the machine is in an allowed end state.
    */
   bool inEndState() const;

   /**
    * Returns the last transition that was taken.
    */
   TransitionType lastTransition() const;

   /**
    * Returns the start state of the machine.
    */
   StateId startState() const;

   /**
    * This method should be redefined by template specialization for every
    * concret TransitionType.
    */
   QString transitionTypeToString(TransitionType type) const;

   /**
    * Outputs the state machine in DOT format to the given
    * output @p device.
    */
   bool outputGraph(QIODevice *device, const QString &graphName) const;

   /**
    * Returns a DFA that is equal to the NFA of the state machine.
    */
   XsdStateMachine<TransitionType> toDFA() const;

   /**
    * Returns the information of all states of the state machine.
    */
   QHash<StateId, StateType> states() const;

   /**
    * Returns the information of all transitions of the state machine.
    *
    * The implementation is inlined in order to workaround a compiler
    * bug on Symbian/winscw.
    */
   QHash<StateId, QHash<TransitionType, QVector<StateId> > > transitions() const {
      return m_transitions;
   }

 private:
   /**
    * Returns the DFA state for the given @p nfaStat from the given @p stateTable.
    * If there is no corresponding DFA state yet, a new one is created.
    */
   StateId dfaStateForNfaState(QSet<StateId> nfaState, QList< QPair< QSet<StateId>, StateId> > &stateTable,
                               XsdStateMachine<TransitionType> &dfa) const;

   /**
    * Returns the set of all states that can be reached from the set of @p input states
    * by the epsilon transition.
    *
    * The implementation is inlined in order to workaround a compiler
    * bug on Symbian/winscw.
    */
   QSet<StateId> epsilonClosure(const QSet<StateId> &input) const {
      // every state can reach itself by epsilon transition, so include the input states
      // in the result as well
      QSet<StateId> result = input;

      // add the input states to the list of to be processed states
      QList<StateId> workStates = input.toList();
      while (!workStates.isEmpty()) { // while there are states to be processed left...

         // dequeue one state from list
         const StateId state = workStates.takeFirst();

         // get the list of states that can be reached by the epsilon transition
         // from the current 'state'
         const QVector<StateId> targetStates = m_epsilonTransitions.value(state);
         for (int i = 0; i < targetStates.count(); ++i) {
            // if we have this target state not in our result set yet...
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

   /**
    * Returns the set of all states that can be reached from the set of given @p states
    * by the given @p input.
    *
    * The implementation is inlined in order to workaround a compiler
    * bug on Symbian/winscw.
    */
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
