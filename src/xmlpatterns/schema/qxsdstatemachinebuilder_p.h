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

   XsdStateMachineBuilder(XsdStateMachine<XsdTerm::Ptr> *machine, const NamePool::Ptr &namePool, Mode mode = CheckingMode);

   XsdStateMachine<XsdTerm::Ptr>::StateId reset();

   XsdStateMachine<XsdTerm::Ptr>::StateId addStartState(XsdStateMachine<XsdTerm::Ptr>::StateId state);

   XsdStateMachine<XsdTerm::Ptr>::StateId buildParticle(const XsdParticle::Ptr &particle,
         XsdStateMachine<XsdTerm::Ptr>::StateId endState);

   XsdStateMachine<XsdTerm::Ptr>::StateId buildTerm(const XsdTerm::Ptr &term,
         XsdStateMachine<XsdTerm::Ptr>::StateId endState);

   static QHash<XsdTerm::Ptr, XsdParticle::Ptr> particleLookupMap(const XsdParticle::Ptr &particle);

 private:
   XsdStateMachine<XsdTerm::Ptr> *m_stateMachine;
   NamePool::Ptr m_namePool;
   Mode m_mode;
};

}

#endif
