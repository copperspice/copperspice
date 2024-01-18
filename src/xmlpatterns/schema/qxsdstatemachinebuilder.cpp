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

#include "qxsdstatemachinebuilder_p.h"

#include "qxsdelement_p.h"
#include "qxsdmodelgroup_p.h"
#include "qxsdschemahelper_p.h"

using namespace QPatternist;

/*
 * This methods takes a list of objects and returns a list of list
 * of all combinations the objects can be ordered.
 *
 * e.g. input = [ 1, 2, 3 ]
 *      output = [
 *                  [ 1, 2, 3 ],
 *                  [ 1, 3, 2 ],
 *                  [ 2, 1, 3 ],
 *                  [ 2, 3, 1 ],
 *                  [ 3, 1, 2 ],
 *                  [ 3, 2, 1 ]
 *               ]
 *
 * The method is used to create all possible combinations for the particles
 * in an <all> model group.
 */
template <typename T>
QList< QList<T> > allCombinations(const QList<T> &input)
{
   if (input.count() == 1) {
      return (QList< QList<T> >() << input);
   }

   QList< QList<T> > result;
   for (int i = 0; i < input.count(); ++i) {
      QList<T> subList = input;
      T value = subList.takeAt(i);

      QList< QList<T> > subLists = allCombinations(subList);
      for (int j = 0; j < subLists.count(); ++j) {
         subLists[j].prepend(value);
      }
      result << subLists;
   }

   return result;
}

XsdStateMachineBuilder::XsdStateMachineBuilder(XsdStateMachine<XsdTerm::Ptr> *machine, const NamePool::Ptr &namePool,
      Mode mode)
   : m_stateMachine(machine), m_namePool(namePool), m_mode(mode)
{
}

XsdStateMachine<XsdTerm::Ptr>::StateId XsdStateMachineBuilder::reset()
{
   Q_ASSERT(m_stateMachine);

   m_stateMachine->clear();

   return m_stateMachine->addState(XsdStateMachine<XsdTerm::Ptr>::EndState);
}

XsdStateMachine<XsdTerm::Ptr>::StateId XsdStateMachineBuilder::addStartState(XsdStateMachine<XsdTerm::Ptr>::StateId
      state)
{
   const XsdStateMachine<XsdTerm::Ptr>::StateId startState = m_stateMachine->addState(
            XsdStateMachine<XsdTerm::Ptr>::StartState);
   m_stateMachine->addEpsilonTransition(startState, state);

   return startState;
}

/*
 * Create the FSA according to Algorithm Tp(S) from http://www.ltg.ed.ac.uk/~ht/XML_Europe_2003.html
 */
XsdStateMachine<XsdTerm::Ptr>::StateId XsdStateMachineBuilder::buildParticle(const XsdParticle::Ptr &particle,
      XsdStateMachine<XsdTerm::Ptr>::StateId endState)
{
   XsdStateMachine<XsdTerm::Ptr>::StateId currentStartState = endState;
   XsdStateMachine<XsdTerm::Ptr>::StateId currentEndState = endState;

   // 2
   if (particle->maximumOccursUnbounded()) {
      const XsdStateMachine<XsdTerm::Ptr>::StateId t = m_stateMachine->addState(XsdStateMachine<XsdTerm::Ptr>::InternalState);
      const XsdStateMachine<XsdTerm::Ptr>::StateId n = buildTerm(particle->term(), t);

      m_stateMachine->addEpsilonTransition(t, n);
      m_stateMachine->addEpsilonTransition(n, endState);

      currentEndState = t;
      currentStartState = t;
   } else { // 3
      int count = (particle->maximumOccurs() - particle->minimumOccurs());
      if (count > 100) {
         count = 100;
      }

      for (int i = 0; i < count; ++i) {
         currentStartState = buildTerm(particle->term(), currentEndState);
         m_stateMachine->addEpsilonTransition(currentStartState, endState);
         currentEndState = currentStartState;
      }
   }

   int minOccurs = particle->minimumOccurs();
   if (minOccurs > 100) {
      minOccurs = 100;
   }

   for (int i = 0; i < minOccurs; ++i) {
      currentStartState = buildTerm(particle->term(), currentEndState);
      currentEndState = currentStartState;
   }

   return currentStartState;
}

/*
 * Create the FSA according to Algorithm Tt(S) from http://www.ltg.ed.ac.uk/~ht/XML_Europe_2003.html
 */
XsdStateMachine<XsdTerm::Ptr>::StateId XsdStateMachineBuilder::buildTerm(const XsdTerm::Ptr &term,
      XsdStateMachine<XsdTerm::Ptr>::StateId endState)
{
   if (term->isWildcard()) { // 1
      const XsdStateMachine<XsdTerm::Ptr>::StateId b = m_stateMachine->addState(XsdStateMachine<XsdTerm::Ptr>::InternalState);
      m_stateMachine->addTransition(b, term, endState);
      return b;
   } else if (term->isElement()) { // 2
      const XsdStateMachine<XsdTerm::Ptr>::StateId b = m_stateMachine->addState(XsdStateMachine<XsdTerm::Ptr>::InternalState);
      m_stateMachine->addTransition(b, term, endState);

      const XsdElement::Ptr element(term);
      if (m_mode == CheckingMode) {
         const XsdElement::WeakList substGroups = element->substitutionGroups();
         for (int i = 0; i < substGroups.count(); ++i) {
            m_stateMachine->addTransition(b, XsdElement::Ptr(substGroups.at(i)), endState);
         }
      } else if (m_mode == ValidatingMode) {
         const XsdElement::WeakList substGroups = element->substitutionGroups();
         for (int i = 0; i < substGroups.count(); ++i) {
            if (XsdSchemaHelper::substitutionGroupOkTransitive(element, XsdElement::Ptr(substGroups.at(i)), m_namePool)) {
               m_stateMachine->addTransition(b, XsdElement::Ptr(substGroups.at(i)), endState);
            }
         }
      }

      return b;
   } else if (term->isModelGroup()) {
      const XsdModelGroup::Ptr group(term);

      if (group->compositor() == XsdModelGroup::ChoiceCompositor) { // 3
         const XsdStateMachine<XsdTerm::Ptr>::StateId b = m_stateMachine->addState(XsdStateMachine<XsdTerm::Ptr>::InternalState);

         for (int i = 0; i < group->particles().count(); ++i) {
            const XsdParticle::Ptr particle(group->particles().at(i));
            if (particle->maximumOccurs() != 0) {
               const XsdStateMachine<XsdTerm::Ptr>::StateId state = buildParticle(particle, endState);
               m_stateMachine->addEpsilonTransition(b, state);
            }
         }

         return b;
      } else if (group->compositor() == XsdModelGroup::SequenceCompositor) { // 4
         XsdStateMachine<XsdTerm::Ptr>::StateId currentStartState = endState;
         XsdStateMachine<XsdTerm::Ptr>::StateId currentEndState = endState;

         for (int i = (group->particles().count() - 1); i >= 0; --i) { // iterate reverse
            const XsdParticle::Ptr particle(group->particles().at(i));
            if (particle->maximumOccurs() != 0) {
               currentStartState = buildParticle(particle, currentEndState);
               currentEndState = currentStartState;
            }
         }

         return currentStartState;
      } else if (group->compositor() == XsdModelGroup::AllCompositor) {
         const XsdStateMachine<XsdTerm::Ptr>::StateId newStartState = m_stateMachine->addState(
                  XsdStateMachine<XsdTerm::Ptr>::InternalState);

         const QList<XsdParticle::List> list = allCombinations(group->particles());

         for (int i = 0; i < list.count(); ++i) {
            XsdStateMachine<XsdTerm::Ptr>::StateId currentStartState = endState;
            XsdStateMachine<XsdTerm::Ptr>::StateId currentEndState = endState;

            const XsdParticle::List particles = list.at(i);
            for (int j = (particles.count() - 1); j >= 0; --j) { // iterate reverse
               const XsdParticle::Ptr particle(particles.at(j));
               if (particle->maximumOccurs() != 0) {
                  currentStartState = buildParticle(particle, currentEndState);
                  currentEndState = currentStartState;
               }
            }
            m_stateMachine->addEpsilonTransition(newStartState, currentStartState);
         }

         if (list.isEmpty()) {
            return endState;
         } else {
            return newStartState;
         }
      }
   }

   Q_ASSERT(false);
   return 0;
}

static void internalParticleLookupMap(const XsdParticle::Ptr &particle, QHash<XsdTerm::Ptr, XsdParticle::Ptr> &hash)
{
   hash.insert(particle->term(), particle);

   if (particle->term()->isModelGroup()) {
      const XsdModelGroup::Ptr group(particle->term());
      const XsdParticle::List particles = group->particles();
      for (int i = 0; i < particles.count(); ++i) {
         internalParticleLookupMap(particles.at(i), hash);
      }
   }
}

QHash<XsdTerm::Ptr, XsdParticle::Ptr> XsdStateMachineBuilder::particleLookupMap(const XsdParticle::Ptr &particle)
{
   QHash<XsdTerm::Ptr, XsdParticle::Ptr> result;
   internalParticleLookupMap(particle, result);

   return result;
}
