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

#ifndef QXsdStateMachineBuilder_P_H
#define QXsdStateMachineBuilder_P_H

#include <qexplicitlyshareddatapointer.h>
#include <qlist.h>

#include <qxsdparticle_p.h>
#include <qxsdstatemachine_p.h>
#include <qxsdterm_p.h>

namespace QPatternist {

class XsdStateMachineBuilder : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<XsdStateMachineBuilder> Ptr;

   enum Mode {
      CheckingMode,
      ValidatingMode
   };

   /**
    * Creates a new state machine builder.
    *
    * @param machine The state machine it should work on.
    * @param namePool The name pool used by all schema components.
    * @param mode The mode the machine shall be build for.
    */
   XsdStateMachineBuilder(XsdStateMachine<XsdTerm::Ptr> *machine, const NamePool::Ptr &namePool, Mode mode = CheckingMode);

   /**
    * Resets the state machine.
    *
    * @returns The initial end state.
    */
   XsdStateMachine<XsdTerm::Ptr>::StateId reset();

   /**
    * Prepends a start state to the given @p state.
    * That is needed to allow the conversion of the state machine from a FSA to a DFA.
    */
   XsdStateMachine<XsdTerm::Ptr>::StateId addStartState(XsdStateMachine<XsdTerm::Ptr>::StateId state);

   /**
    * Creates the state machine for the given @p particle that should have the
    * given @p endState.
    *
    * @returns The new start state.
    */
   XsdStateMachine<XsdTerm::Ptr>::StateId buildParticle(const XsdParticle::Ptr &particle,
         XsdStateMachine<XsdTerm::Ptr>::StateId endState);

   /**
    * Creates the state machine for the given @p term that should have the
    * given @p endState.
    *
    * @returns The new start state.
    */
   XsdStateMachine<XsdTerm::Ptr>::StateId buildTerm(const XsdTerm::Ptr &term,
         XsdStateMachine<XsdTerm::Ptr>::StateId endState);

   /**
    * Returns a hash that maps each term that appears inside @p particle, to the particle it belongs.
    *
    * @note These information are used by XsdParticleChecker to check particle inheritance.
    */
   static QHash<XsdTerm::Ptr, XsdParticle::Ptr> particleLookupMap(const XsdParticle::Ptr &particle);

 private:
   XsdStateMachine<XsdTerm::Ptr> *m_stateMachine;
   NamePool::Ptr m_namePool;
   Mode m_mode;
};

}

#endif
