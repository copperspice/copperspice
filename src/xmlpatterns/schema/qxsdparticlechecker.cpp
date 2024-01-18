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

#include "qxsdparticlechecker_p.h"

#include "qxsdelement_p.h"
#include "qxsdmodelgroup_p.h"
#include "qxsdschemahelper_p.h"
#include "qxsdstatemachine_p.h"
#include "qxsdstatemachinebuilder_p.h"
#include "qxsdtypechecker_p.h"

#include <QFile>

using namespace QPatternist;

namespace QPatternist {
/**
 * This template specialization is picked up by XsdStateMachine and allows us
 * to print nice edge labels.
 */
template <>
QString XsdStateMachine<XsdTerm::Ptr>::transitionTypeToString(XsdTerm::Ptr term) const
{
   if (!term) {
      return QLatin1String("(empty)");
   }

   if (term->isElement()) {
      return XsdElement::Ptr(term)->displayName(m_namePool);
   } else if (term->isWildcard()) {
      const XsdWildcard::Ptr wildcard(term);
      return QLatin1String("(wildcard)");
   } else {
      return QString();
   }
}
}

/**
 * This method is used by the isUPAConform method to check whether @p term and @p otherTerm
 * are the same resp. match each other.
 */
static bool termMatches(const XsdTerm::Ptr &term, const XsdTerm::Ptr &otherTerm, const NamePool::Ptr &namePool)
{
   if (term->isElement()) {
      const XsdElement::Ptr element(term);

      if (otherTerm->isElement()) {
         // both, the term and the other term are elements

         const XsdElement::Ptr otherElement(otherTerm);

         // if they have the same name they match
         if (element->name(namePool) == otherElement->name(namePool)) {
            return true;
         }

      } else if (otherTerm->isWildcard()) {
         // the term is an element and the other term a wildcard

         const XsdWildcard::Ptr wildcard(otherTerm);

         // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
         QXmlName name = element->name(namePool);
         if (name.namespaceURI() == StandardNamespaces::empty) {
            name.setNamespaceURI(namePool->allocateNamespace(XsdWildcard::absentNamespace()));
         }

         // if the wildcards namespace constraint allows the elements name, they match
         if (XsdSchemaHelper::wildcardAllowsExpandedName(name, wildcard, namePool)) {
            return true;
         }
      }
   } else if (term->isWildcard()) {
      const XsdWildcard::Ptr wildcard(term);

      if (otherTerm->isElement()) {
         // the term is a wildcard and the other term an element

         const XsdElement::Ptr otherElement(otherTerm);

         // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
         QXmlName name = otherElement->name(namePool);
         if (name.namespaceURI() == StandardNamespaces::empty) {
            name.setNamespaceURI(namePool->allocateNamespace(XsdWildcard::absentNamespace()));
         }

         // if the wildcards namespace constraint allows the elements name, they match
         if (XsdSchemaHelper::wildcardAllowsExpandedName(name, wildcard, namePool)) {
            return true;
         }

      } else if (otherTerm->isWildcard()) {
         // both, the term and the other term are wildcards

         const XsdWildcard::Ptr otherWildcard(otherTerm);

         // check if the range of the wildcard overlaps.
         const XsdWildcard::Ptr intersectionWildcard = XsdSchemaHelper::wildcardIntersection(wildcard, otherWildcard);
         if (!intersectionWildcard ||
               (intersectionWildcard &&
                !(intersectionWildcard->namespaceConstraint()->variety() != XsdWildcard::NamespaceConstraint::Not &&
                  intersectionWildcard->namespaceConstraint()->namespaces().isEmpty()))) {
            return true;
         }
      }
   }

   return false;
}

/**
 * This method is used by the subsumes algorithm to check whether the @p derivedTerm is validly derived from the @p baseTerm.
 *
 * @param baseTerm The term of the base component (type or group).
 * @param derivedTerm The term of the derived component (type or group).
 * @param particles A hash to map the passed base and derived term to the particles they belong to.
 * @param context The schema context.
 * @param errorMsg The error message in the case that an error occurs.
 */
static bool derivedTermValid(const XsdTerm::Ptr &baseTerm, const XsdTerm::Ptr &derivedTerm,
                             const QHash<XsdTerm::Ptr, XsdParticle::Ptr> &particles, const XsdSchemaContext::Ptr &context, QString &errorMsg)
{
   const NamePool::Ptr namePool(context->namePool());

   // find the particles where the base and derived term belongs to
   const XsdParticle::Ptr baseParticle = particles.value(baseTerm);
   const XsdParticle::Ptr derivedParticle = particles.value(derivedTerm);

   // check that an empty particle can not be derived from a non-empty particle
   if (derivedParticle && baseParticle) {
      if (XsdSchemaHelper::isParticleEmptiable(derivedParticle) && !XsdSchemaHelper::isParticleEmptiable(baseParticle)) {
         errorMsg = QtXmlPatterns::tr("Empty particle cannot be derived from non-empty particle.");
         return false;
      }
   }

   if (baseTerm->isElement()) {
      const XsdElement::Ptr element(baseTerm);

      if (derivedTerm->isElement()) {
         // if both terms are elements

         const XsdElement::Ptr derivedElement(derivedTerm);

         // check names are equal
         if (element->name(namePool) != derivedElement->name(namePool)) {
            errorMsg = QtXmlPatterns::tr("Derived particle is missing element %1.").formatArg(formatKeyword(element->displayName(
                          namePool)));
            return false;
         }

         // check value constraints are equal (if available)
         if (element->valueConstraint() && element->valueConstraint()->variety() == XsdElement::ValueConstraint::Fixed) {
            if (!derivedElement->valueConstraint()) {
               errorMsg = QtXmlPatterns::tr("Derived element %1 is missing value constraint as defined in base particle.").formatArg(
                             formatKeyword(derivedElement->displayName(namePool)));
               return false;
            }

            if (derivedElement->valueConstraint()->variety() != XsdElement::ValueConstraint::Fixed) {
               errorMsg = QtXmlPatterns::tr("Derived element %1 has weaker value constraint than base particle.").formatArg(formatKeyword(
                             derivedElement->displayName(namePool)));
               return false;
            }

            const QSourceLocation dummyLocation(QUrl(QLatin1String("http://dummy.org")), 1, 1);
            const XsdTypeChecker checker(context, QVector<QXmlName>(), dummyLocation);
            if (!checker.valuesAreEqual(element->valueConstraint()->value(), derivedElement->valueConstraint()->value(),
                                        derivedElement->type())) {
               errorMsg =
                  QtXmlPatterns::tr("Fixed value constraint of element %1 differs from value constraint in base particle.").formatArg(
                     formatKeyword(derivedElement->displayName(namePool)));
               return false;
            }
         }

         // check that a derived element can not be nillable if the base element is not nillable
         if (!element->isNillable() && derivedElement->isNillable()) {
            errorMsg = QtXmlPatterns::tr("Derived element %1 cannot be nillable as base element is not nillable.").formatArg(
                          formatKeyword(derivedElement->displayName(namePool)));
            return false;
         }

         // check that the constraints of the derived element are more strict then the constraints of the base element
         const XsdElement::BlockingConstraints baseConstraints = element->disallowedSubstitutions();
         const XsdElement::BlockingConstraints derivedConstraints = derivedElement->disallowedSubstitutions();
         if (((baseConstraints & XsdElement::RestrictionConstraint) &&
               !(derivedConstraints & XsdElement::RestrictionConstraint)) ||
               ((baseConstraints & XsdElement::ExtensionConstraint) && !(derivedConstraints & XsdElement::ExtensionConstraint)) ||
               ((baseConstraints & XsdElement::SubstitutionConstraint) &&
                !(derivedConstraints & XsdElement::SubstitutionConstraint))) {
            errorMsg =
               QtXmlPatterns::tr("Block constraints of derived element %1 must not be more weaker than in the base element.").formatArg(
                  formatKeyword(derivedElement->displayName(namePool)));
            return false;
         }

         // if the type of both elements is the same we can stop testing here
         if (element->type()->name(namePool) == derivedElement->type()->name(namePool)) {
            return true;
         }

         // check that the type of the derived element can validly derived from the type of the base element
         if (derivedElement->type()->isSimpleType()) {
            if (!XsdSchemaHelper::isSimpleDerivationOk(derivedElement->type(), element->type(),
                  SchemaType::DerivationConstraints())) {
               errorMsg = QtXmlPatterns::tr("Simple type of derived element %1 cannot be validly derived from base element.").formatArg(
                             formatKeyword(derivedElement->displayName(namePool)));
               return false;
            }
         } else if (derivedElement->type()->isComplexType()) {
            if (!XsdSchemaHelper::isComplexDerivationOk(derivedElement->type(), element->type(),
                  SchemaType::DerivationConstraints())) {
               errorMsg = QtXmlPatterns::tr("Complex type of derived element %1 cannot be validly derived from base element.").formatArg(
                             formatKeyword(derivedElement->displayName(namePool)));
               return false;
            }
         }

         // if both, derived and base element, have a complex type that contains a particle itself, apply the subsumes algorithm
         // recursive on their particles
         if (element->type()->isComplexType() && derivedElement->type()->isComplexType()) {
            if (element->type()->isDefinedBySchema() && derivedElement->type()->isDefinedBySchema()) {
               const XsdComplexType::Ptr baseType(element->type());
               const XsdComplexType::Ptr derivedType(derivedElement->type());
               if ((baseType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                     baseType->contentType()->variety() == XsdComplexType::ContentType::Mixed) &&
                     (derivedType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                      derivedType->contentType()->variety() == XsdComplexType::ContentType::Mixed)) {

                  return XsdParticleChecker::subsumes(baseType->contentType()->particle(), derivedType->contentType()->particle(),
                                                      context, errorMsg);
               }
            }
         }

         return true;
      } else if (derivedTerm->isWildcard()) {
         // derive a wildcard from an element is not allowed
         errorMsg = QtXmlPatterns::tr("Element %1 is missing in derived particle.").formatArg(formatKeyword(element->displayName(
                       namePool)));
         return false;
      }
   } else if (baseTerm->isWildcard()) {
      const XsdWildcard::Ptr wildcard(baseTerm);

      if (derivedTerm->isElement()) {
         // the base term is a wildcard and derived term an element

         const XsdElement::Ptr derivedElement(derivedTerm);

         // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
         QXmlName name = derivedElement->name(namePool);
         if (name.namespaceURI() == StandardNamespaces::empty) {
            name.setNamespaceURI(namePool->allocateNamespace(XsdWildcard::absentNamespace()));
         }

         // check that name of the element is allowed by the wildcards namespace constraint
         if (!XsdSchemaHelper::wildcardAllowsExpandedName(name, wildcard, namePool)) {
            errorMsg = QtXmlPatterns::tr("Element %1 does not match namespace constraint of wildcard in base particle.").formatArg(
                          formatKeyword(derivedElement->displayName(namePool)));
            return false;
         }

      } else if (derivedTerm->isWildcard()) {
         // both, derived and base term are wildcards

         const XsdWildcard::Ptr derivedWildcard(derivedTerm);

         // check that the derived wildcard is a valid subset of the base wildcard
         if (!XsdSchemaHelper::isWildcardSubset(derivedWildcard, wildcard)) {
            errorMsg = QtXmlPatterns::tr("Wildcard in derived particle is not a valid subset of wildcard in base particle.");
            return false;
         }

         if (!XsdSchemaHelper::checkWildcardProcessContents(wildcard, derivedWildcard)) {
            errorMsg =
               QtXmlPatterns::tr("processContent of wildcard in derived particle is weaker than wildcard in base particle.");
            return false;
         }
      }

      return true;
   }

   return false;
}

typedef QHash<QXmlName, XsdElement::Ptr> ElementHash;

/**
 * Internal helper method that checks if the given @p particle contains an element with the
 * same name and type twice.
 */
static bool hasDuplicatedElementsInternal(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool,
      ElementHash &hash, XsdElement::Ptr &conflictingElement)
{
   const XsdTerm::Ptr term = particle->term();
   if (term->isElement()) {
      const XsdElement::Ptr mainElement(term);
      XsdElement::WeakList substGroups = mainElement->substitutionGroups();
      if (substGroups.isEmpty()) {
         substGroups << mainElement.data();
      }

      for (int i = 0; i < substGroups.count(); ++i) {
         const XsdElement::Ptr element(substGroups.at(i));
         if (hash.contains(element->name(namePool))) {
            if (element->type()->name(namePool) != hash.value(element->name(namePool))->type()->name(namePool)) {
               conflictingElement = element;
               return true;
            }
         } else {
            hash.insert(element->name(namePool), element);
         }
      }
   } else if (term->isModelGroup()) {
      const XsdModelGroup::Ptr group(term);
      const XsdParticle::List particles = group->particles();
      for (int i = 0; i < particles.count(); ++i) {
         if (hasDuplicatedElementsInternal(particles.at(i), namePool, hash, conflictingElement)) {
            return true;
         }
      }
   }

   return false;
}

bool XsdParticleChecker::hasDuplicatedElements(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool,
      XsdElement::Ptr &conflictingElement)
{
   ElementHash hash;
   return hasDuplicatedElementsInternal(particle, namePool, hash, conflictingElement);
}

bool XsdParticleChecker::isUPAConform(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool)
{

   /**
    * In case we encounter an <xsd:all> element, don't construct a state machine, but use the approach
    * described at http://www.w3.org/TR/xmlschema-1/#non-ambig
    * Reason: For n elements inside the <xsd:all>, represented in the NDA, the state machine
    * constructs n! states in the DFA, which does not scale.
    */
   if (particle->term()->isModelGroup()) {
      const XsdModelGroup::Ptr group(particle->term());
      if (group->compositor() == XsdModelGroup::AllCompositor) {
         return isUPAConformXsdAll(particle, namePool);
      }
   }

   /**
    * The algorithm is implemented like described in http://www.ltg.ed.ac.uk/~ht/XML_Europe_2003.html#S2.2
    */

   // create a state machine for the given particle
   XsdStateMachine<XsdTerm::Ptr> stateMachine(namePool);

   XsdStateMachineBuilder builder(&stateMachine, namePool);
   const XsdStateMachine<XsdTerm::Ptr>::StateId endState = builder.reset();
   const XsdStateMachine<XsdTerm::Ptr>::StateId startState = builder.buildParticle(particle, endState);
   builder.addStartState(startState);

   /*
       static int counter = 0;
       {
           QFile file(QString("/tmp/file_upa%1.dot").formatArg(counter));
           file.open(QIODevice::WriteOnly);
           stateMachine.outputGraph(&file, "Base");
           file.close();
       }
       ::system(QString("dot -Tpng /tmp/file_upa%1.dot -o/tmp/file_upa%1.png").formatArg(counter).toLatin1().data());
   */
   const XsdStateMachine<XsdTerm::Ptr> dfa = stateMachine.toDFA();
   /*
       {
           QFile file(QString("/tmp/file_upa%1dfa.dot").formatArg(counter));
           file.open(QIODevice::WriteOnly);
           dfa.outputGraph(&file, "Base");
           file.close();
       }
       ::system(QString("dot -Tpng /tmp/file_upa%1dfa.dot -o/tmp/file_upa%1dfa.png").formatArg(counter).toLatin1().data());
   */
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateType> states = dfa.states();
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > >
   transitions = dfa.transitions();

   // the basic idea of that algorithm is to iterate over all states of that machine and check that no two edges
   // that match on the same term leave a state, so for a given term it should always be obvious which edge to take
   QHashIterator<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateType> stateIt(states);
   while (stateIt.hasNext()) { // iterate over all states
      stateIt.next();

      // fetch all transitions the current state allows
      const QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > currentTransitions = transitions.value(
               stateIt.key());
      QHashIterator<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > transitionIt(currentTransitions);
      while (transitionIt.hasNext()) { // iterate over all transitions
         transitionIt.next();

         if (transitionIt.value().size() > 1) {
            // we have one state with two edges leaving it, that means
            // the XsdTerm::Ptr exists twice, that is an error
            return false;
         }

         QHashIterator<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > innerTransitionIt(currentTransitions);
         while (innerTransitionIt.hasNext()) { // iterate over all transitions again, as we have to compare all transitions with all
            innerTransitionIt.next();

            if (transitionIt.key() == innerTransitionIt.key()) { // do no compare with ourself
               continue;
            }

            // use the helper method termMatches to check if both term matches
            if (termMatches(transitionIt.key(), innerTransitionIt.key(), namePool)) {
               return false;
            }
         }
      }
   }

   return true;
}

bool XsdParticleChecker::isUPAConformXsdAll(const XsdParticle::Ptr &particle, const NamePool::Ptr &namePool)
{
   /**
    * see http://www.w3.org/TR/xmlschema-1/#non-ambig
    */
   const XsdModelGroup::Ptr group(particle->term());
   const XsdParticle::List particles = group->particles();
   const int count = particles.count();
   for (int left = 0; left < count; ++left) {
      for (int right = left + 1; right < count; ++right) {
         if (termMatches(particles.at(left)->term(), particles.at(right)->term(), namePool)) {
            return false;
         }
      }
   }
   return true;
}

bool XsdParticleChecker::subsumes(const XsdParticle::Ptr &particle, const XsdParticle::Ptr &derivedParticle,
                                  const XsdSchemaContext::Ptr &context, QString &errorMsg)
{
   /**
    * The algorithm is implemented like described in http://www.ltg.ed.ac.uk/~ht/XML_Europe_2003.html#S2.3
    */

   const NamePool::Ptr namePool(context->namePool());

   XsdStateMachine<XsdTerm::Ptr> baseStateMachine(namePool);
   XsdStateMachine<XsdTerm::Ptr> derivedStateMachine(namePool);

   // build up state machines for both particles
   {
      XsdStateMachineBuilder builder(&baseStateMachine, namePool);
      const XsdStateMachine<XsdTerm::Ptr>::StateId endState = builder.reset();
      const XsdStateMachine<XsdTerm::Ptr>::StateId startState = builder.buildParticle(particle, endState);
      builder.addStartState(startState);

      baseStateMachine = baseStateMachine.toDFA();
   }
   {
      XsdStateMachineBuilder builder(&derivedStateMachine, namePool);
      const XsdStateMachine<XsdTerm::Ptr>::StateId endState = builder.reset();
      const XsdStateMachine<XsdTerm::Ptr>::StateId startState = builder.buildParticle(derivedParticle, endState);
      builder.addStartState(startState);

      derivedStateMachine = derivedStateMachine.toDFA();
   }

   QHash<XsdTerm::Ptr, XsdParticle::Ptr> particlesHash = XsdStateMachineBuilder::particleLookupMap(particle);
   particlesHash.unite(XsdStateMachineBuilder::particleLookupMap(derivedParticle));

   /*
       static int counter = 0;
       {
           QFile file(QString("/tmp/file_base%1.dot").formatArg(counter));
           file.open(QIODevice::WriteOnly);
           baseStateMachine.outputGraph(&file, QLatin1String("Base"));
           file.close();
       }
       {
           QFile file(QString("/tmp/file_derived%1.dot").formatArg(counter));
           file.open(QIODevice::WriteOnly);
           derivedStateMachine.outputGraph(&file, QLatin1String("Base"));
           file.close();
       }
       ::system(QString("dot -Tpng /tmp/file_base%1.dot -o/tmp/file_base%1.png").formatArg(counter).toLatin1().data());
       ::system(QString("dot -Tpng /tmp/file_derived%1.dot -o/tmp/file_derived%1.png").formatArg(counter).toLatin1().data());
   */

   const XsdStateMachine<XsdTerm::Ptr>::StateId baseStartState = baseStateMachine.startState();
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateType> baseStates =
      baseStateMachine.states();
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > >
   baseTransitions = baseStateMachine.transitions();

   const XsdStateMachine<XsdTerm::Ptr>::StateId derivedStartState = derivedStateMachine.startState();
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateType> derivedStates =
      derivedStateMachine.states();
   const QHash<XsdStateMachine<XsdTerm::Ptr>::StateId, QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > >
   derivedTransitions = derivedStateMachine.transitions();

   // @see http://www.ltg.ed.ac.uk/~ht/XML_Europe_2003.html#S2.3.1

   // define working set
   QList<QPair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId> > workSet;
   QList<QPair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId> > processedSet;

   // 1) fill working set initially with start states
   workSet.append(qMakePair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId>(baseStartState,
                  derivedStartState));
   processedSet.append(qMakePair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId>
                       (baseStartState, derivedStartState));

   while (!workSet.isEmpty()) { // while there are state sets to process

      // 3) dequeue on state set
      const QPair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId> set = workSet.takeFirst();

      const QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > derivedTrans = derivedTransitions.value(
               set.second);
      QHashIterator<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > derivedIt(derivedTrans);

      const QHash<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > baseTrans = baseTransitions.value(
               set.first);

      while (derivedIt.hasNext()) {
         derivedIt.next();

         bool found = false;
         QHashIterator<XsdTerm::Ptr, QVector<XsdStateMachine<XsdTerm::Ptr>::StateId> > baseIt(baseTrans);
         while (baseIt.hasNext()) {
            baseIt.next();
            if (derivedTermValid(baseIt.key(), derivedIt.key(), particlesHash, context, errorMsg)) {
               const QPair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId> endSet =
                  qMakePair<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateId>(baseIt.value().first(),
                        derivedIt.value().first());
               if (!processedSet.contains(endSet) && !workSet.contains(endSet)) {
                  workSet.append(endSet);
                  processedSet.append(endSet);
               }

               found = true;
            }
         }

         if (!found) {
            return false;
         }
      }
   }

   // 5)
   QHashIterator<XsdStateMachine<XsdTerm::Ptr>::StateId, XsdStateMachine<XsdTerm::Ptr>::StateType> it(derivedStates);
   while (it.hasNext()) {
      it.next();

      if (it.value() == XsdStateMachine<XsdTerm::Ptr>::EndState ||
            it.value() == XsdStateMachine<XsdTerm::Ptr>::StartEndState) {
         for (int i = 0; i < processedSet.count(); ++i) {
            if (processedSet.at(i).second == it.key() &&
                  (baseStates.value(processedSet.at(i).first) != XsdStateMachine<XsdTerm::Ptr>::EndState &&
                   baseStates.value(processedSet.at(i).first) != XsdStateMachine<XsdTerm::Ptr>::StartEndState)) {
               errorMsg = QtXmlPatterns::tr("Derived particle allows content that is not allowed in the base particle.");
               return false;
            }
         }
      }
   }

   return true;
}
