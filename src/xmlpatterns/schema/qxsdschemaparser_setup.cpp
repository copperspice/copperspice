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

#include "qxsdschemaparser_p.h"

#include "qbuiltintypes_p.h"

using namespace QPatternist;

/**
 * @page using_dfa_for_schema Using DFA for validation of correct XML tag occurrence
 *
 * This page describes how to use DFAs for validating that the XML child tags of an
 * XML parent tag occur in the right order.
 *
 * To validate the occurrence of XML tags one need a regular expression that describes
 * which tags can appear how often in what context. For example the regular expression
 * of the global <em>attribute</em> tag in XML Schema is (annotation?, simpleType?).
 * That means the <em>attribute</em> tag can contain an <em>annotation</em> tag followed
 * by a <em>simpleType</em> tag, or just one <em>simpleType</em> tag and even no child
 * tag at all.
 * So the regular expression describes some kind of language and all the various occurrences
 * of the child tags can be seen as words of that language.
 * We can create a DFA now, that accepts all words (and only these words) of that language
 * and whenever we want to check if a sequence of child tags belongs to the language,
 * we test if the sequence passes the DFA successfully.
 *
 * The following example shows how to create the DFA for the regular expression method
 * above.
 *
 * \dotfile GlobalAttribute_diagram.dot
 *
 * At first we need a start state (1), that's the state the DFA is before it
 * starts running. As our regular expression allows that there are no child tags, the
 * start state is an end state as well (marked by the double circle).
 * Now we fetch the first token from the XML file (let's assume it is an <em>annotation</em> tag)
 * and check if there is an edge labled with the tag name leaving the current state of the DFA.
 * If there is no such edge, the input doesn't fullfill the rules of the regular expression,
 * so we throw an error. Otherwise we follow that edge and the DFA is set to the new state (2) the
 * edge points to. Now we fetch the next token from the XML file and do the previous steps again.
 * If there is no further input from the XML file, we check whether the DFA is in an end state and
 * throw an error if not.
 *
 * So the algorithm for checking is quite simple, the whole logic is encoded in the DFA and creating
 * one for a regular expression is sometimes not easy, however the ones for XML Schema are straight
 * forward.
 *
 * <h2>Legend:</h2>
 * \dotfile legend.dot
 * <br>
 *
 * <h2>DFA for <em>all</em> tag</h2>
 * \dotfile All_diagram.dot
 * <br>
 * <h2>DFA for <em>alternative</em> tag</h2>
 * \dotfile Alternative_diagram.dot
 * <br>
 * <h2>DFA for <em>annotation</em> tag</h2>
 * \dotfile Annotation_diagram.dot
 * <br>
 * <h2>DFA for <em>anyAttribute</em> tag</h2>
 * \dotfile AnyAttribute_diagram.dot
 * <br>
 * <h2>DFA for <em>any</em> tag</h2>
 * \dotfile Any_diagram.dot
 * <br>
 * <h2>DFA for <em>assert</em> tag</h2>
 * \dotfile Assert_diagram.dot
 * <br>
 * <h2>DFA for <em>choice</em> tag</h2>
 * \dotfile Choice_diagram.dot
 * <br>
 * <h2>DFA for <em>complexContent</em> tag</h2>
 * \dotfile ComplexContent_diagram.dot
 * <br>
 * <h2>DFA for <em>extension</em> tag inside a <em>complexContent</em> tag</h2>
 * \dotfile ComplexContentExtension_diagram.dot
 * <br>
 * <h2>DFA for <em>restriction</em> tag inside a <em>complexContent</em> tag</h2>
 * \dotfile ComplexContentRestriction_diagram.dot
 * <br>
 * <h2>DFA for <em>defaultOpenContent</em> tag</h2>
 * \dotfile DefaultOpenContent_diagram.dot
 * <br>
 * <h2>DFA for <em>enumeration</em> tag</h2>
 * \dotfile EnumerationFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>field</em> tag</h2>
 * \dotfile Field_diagram.dot
 * <br>
 * <h2>DFA for <em>fractionDigits</em> tag</h2>
 * \dotfile FractionDigitsFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>attribute</em> tag</h2>
 * \dotfile GlobalAttribute_diagram.dot
 * <br>
 * <h2>DFA for <em>complexType</em> tag</h2>
 * \dotfile GlobalComplexType_diagram.dot
 * <br>
 * <h2>DFA for <em>element</em> tag</h2>
 * \dotfile GlobalElement_diagram.dot
 * <br>
 * <h2>DFA for <em>simpleType</em> tag</h2>
 * \dotfile GlobalSimpleType_diagram.dot
 * <br>
 * <h2>DFA for <em>import</em> tag</h2>
 * \dotfile Import_diagram.dot
 * <br>
 * <h2>DFA for <em>include</em> tag</h2>
 * \dotfile Include_diagram.dot
 * <br>
 * <h2>DFA for <em>key</em> tag</h2>
 * \dotfile Key_diagram.dot
 * <br>
 * <h2>DFA for <em>keyref</em> tag</h2>
 * \dotfile KeyRef_diagram.dot
 * <br>
 * <h2>DFA for <em>length</em> tag</h2>
 * \dotfile LengthFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>list</em> tag</h2>
 * \dotfile List_diagram.dot
 * <br>
 * <h2>DFA for <em>all</em> tag</h2>
 * \dotfile LocalAll_diagram.dot
 * <br>
 * <h2>DFA for <em>attribute</em> tag</h2>
 * \dotfile LocalAttribute_diagram.dot
 * <br>
 * <h2>DFA for <em>choice</em> tag</h2>
 * \dotfile LocalChoice_diagram.dot
 * <br>
 * <h2>DFA for <em>complexType</em> tag</h2>
 * \dotfile LocalComplexType_diagram.dot
 * <br>
 * <h2>DFA for <em>element</em> tag</h2>
 * \dotfile LocalElement_diagram.dot
 * <br>
 * <h2>DFA for <em>sequence</em> tag</h2>
 * \dotfile LocalSequence_diagram.dot
 * <br>
 * <h2>DFA for <em>simpleType</em> tag that </h2>
 * \dotfile LocalSimpleType_diagram.dot
 * <br>
 * <h2>DFA for <em>maxExclusive</em> tag</h2>
 * \dotfile MaxExclusiveFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>maxInclusive</em> tag</h2>
 * \dotfile MaxInclusiveFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>maxLength</em> tag</h2>
 * \dotfile MaxLengthFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>minExclusive</em> tag</h2>
 * \dotfile MinExclusiveFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>minInclusive</em> tag</h2>
 * \dotfile MinInclusiveFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>minLength</em> tag</h2>
 * \dotfile MinLengthFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>attributeGroup</em> tag without <em>ref</em> attribute</h2>
 * \dotfile NamedAttributeGroup_diagram.dot
 * <br>
 * <h2>DFA for <em>group</em> tag without <em>ref</em> attribute</h2>
 * \dotfile NamedGroup_diagram.dot
 * <br>
 * <h2>DFA for <em>notation</em> tag</h2>
 * \dotfile Notation_diagram.dot
 * <br>
 * <h2>DFA for <em>override</em> tag</h2>
 * \dotfile Override_diagram.dot
 * <br>
 * <h2>DFA for <em>pattern</em> tag</h2>
 * \dotfile PatternFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>redefine</em> tag</h2>
 * \dotfile Redefine_diagram.dot
 * <br>
 * <h2>DFA for <em>attributeGroup</em> tag with <em>ref</em> attribute</h2>
 * \dotfile ReferredAttributeGroup_diagram.dot
 * <br>
 * <h2>DFA for <em>group</em> tag with <em>ref</em> attribute</h2>
 * \dotfile ReferredGroup_diagram.dot
 * <br>
 * <h2>DFA for <em>schema</em> tag</h2>
 * \dotfile Schema_diagram.dot
 * <br>
 * <h2>DFA for <em>selector</em> tag</h2>
 * \dotfile Selector_diagram.dot
 * <br>
 * <h2>DFA for <em>sequence</em> tag</h2>
 * \dotfile Sequence_diagram.dot
 * <br>
 * <h2>DFA for <em>simpleContent</em> tag</h2>
 * \dotfile SimpleContent_diagram.dot
 * <br>
 * <h2>DFA for <em>extension</em> tag inside a <em>simpleContent</em> tag</h2>
 * \dotfile SimpleContentExtension_diagram.dot
 * <br>
 * <h2>DFA for <em>restriction</em> tag inside a <em>simpleContent</em> tag</h2>
 * \dotfile SimpleContentRestriction_diagram.dot
 * <br>
 * <h2>DFA for <em>restriction</em> tag inside a <em>simpleType</em> tag</h2>
 * \dotfile SimpleRestriction_diagram.dot
 * <br>
 * <h2>DFA for <em>totalDigits</em> tag</h2>
 * \dotfile TotalDigitsFacet_diagram.dot
 * <br>
 * <h2>DFA for <em>union</em> tag</h2>
 * \dotfile Union_diagram.dot
 * <br>
 * <h2>DFA for <em>unique</em> tag</h2>
 * \dotfile Unique_diagram.dot
 * <br>
 * <h2>DFA for <em>whiteSpace</em> tag</h2>
 * \dotfile WhiteSpaceFacet_diagram.dot
 */

void XsdSchemaParser::setupStateMachines()
{
   NamePool::Ptr namePool(m_namePool);
   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, simpleType?) : attribute
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);

      m_stateMachines.insert(XsdTagScope::GlobalAttribute, machine);
      m_stateMachines.insert(XsdTagScope::LocalAttribute, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, ((simpleType | complexType)?, alternative*, (unique | key | keyref)*)) : element
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(startState, XsdSchemaToken::ComplexType, s2);
      machine.addTransition(startState, XsdSchemaToken::Alternative, s3);
      machine.addTransition(startState, XsdSchemaToken::Unique, s4);
      machine.addTransition(startState, XsdSchemaToken::Key, s4);
      machine.addTransition(startState, XsdSchemaToken::Keyref, s4);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s1, XsdSchemaToken::ComplexType, s2);
      machine.addTransition(s1, XsdSchemaToken::Alternative, s3);
      machine.addTransition(s1, XsdSchemaToken::Unique, s4);
      machine.addTransition(s1, XsdSchemaToken::Key, s4);
      machine.addTransition(s1, XsdSchemaToken::Keyref, s4);

      machine.addTransition(s2, XsdSchemaToken::Alternative, s3);
      machine.addTransition(s2, XsdSchemaToken::Unique, s4);
      machine.addTransition(s2, XsdSchemaToken::Key, s4);
      machine.addTransition(s2, XsdSchemaToken::Keyref, s4);

      machine.addTransition(s3, XsdSchemaToken::Alternative, s3);
      machine.addTransition(s3, XsdSchemaToken::Unique, s4);
      machine.addTransition(s3, XsdSchemaToken::Key, s4);
      machine.addTransition(s3, XsdSchemaToken::Keyref, s4);

      machine.addTransition(s4, XsdSchemaToken::Unique, s4);
      machine.addTransition(s4, XsdSchemaToken::Key, s4);
      machine.addTransition(s4, XsdSchemaToken::Keyref, s4);

      m_stateMachines.insert(XsdTagScope::GlobalElement, machine);
      m_stateMachines.insert(XsdTagScope::LocalElement, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (simpleContent | complexContent | (openContent?, (group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?), assert*))) : complexType
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s5 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s6 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s7 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleContent, s2);
      machine.addTransition(startState, XsdSchemaToken::ComplexContent, s2);
      machine.addTransition(startState, XsdSchemaToken::OpenContent, s3);
      machine.addTransition(startState, XsdSchemaToken::Group, s4);
      machine.addTransition(startState, XsdSchemaToken::All, s4);
      machine.addTransition(startState, XsdSchemaToken::Choice, s4);
      machine.addTransition(startState, XsdSchemaToken::Sequence, s4);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s5);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s5);
      machine.addTransition(startState, XsdSchemaToken::AnyAttribute, s6);
      machine.addTransition(startState, XsdSchemaToken::Assert, s7);

      machine.addTransition(s1, XsdSchemaToken::SimpleContent, s2);
      machine.addTransition(s1, XsdSchemaToken::ComplexContent, s2);
      machine.addTransition(s1, XsdSchemaToken::OpenContent, s3);
      machine.addTransition(s1, XsdSchemaToken::Group, s4);
      machine.addTransition(s1, XsdSchemaToken::All, s4);
      machine.addTransition(s1, XsdSchemaToken::Choice, s4);
      machine.addTransition(s1, XsdSchemaToken::Sequence, s4);
      machine.addTransition(s1, XsdSchemaToken::Attribute, s5);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s5);
      machine.addTransition(s1, XsdSchemaToken::AnyAttribute, s6);
      machine.addTransition(s1, XsdSchemaToken::Assert, s7);

      machine.addTransition(s3, XsdSchemaToken::Group, s4);
      machine.addTransition(s3, XsdSchemaToken::All, s4);
      machine.addTransition(s3, XsdSchemaToken::Choice, s4);
      machine.addTransition(s3, XsdSchemaToken::Sequence, s4);
      machine.addTransition(s3, XsdSchemaToken::Attribute, s5);
      machine.addTransition(s3, XsdSchemaToken::AttributeGroup, s5);
      machine.addTransition(s3, XsdSchemaToken::AnyAttribute, s6);
      machine.addTransition(s3, XsdSchemaToken::Assert, s7);

      machine.addTransition(s4, XsdSchemaToken::Attribute, s5);
      machine.addTransition(s4, XsdSchemaToken::AttributeGroup, s5);
      machine.addTransition(s4, XsdSchemaToken::AnyAttribute, s6);
      machine.addTransition(s4, XsdSchemaToken::Assert, s7);

      machine.addTransition(s5, XsdSchemaToken::Attribute, s5);
      machine.addTransition(s5, XsdSchemaToken::AttributeGroup, s5);
      machine.addTransition(s5, XsdSchemaToken::AnyAttribute, s6);
      machine.addTransition(s5, XsdSchemaToken::Assert, s7);

      machine.addTransition(s6, XsdSchemaToken::Assert, s7);

      machine.addTransition(s7, XsdSchemaToken::Assert, s7);

      m_stateMachines.insert(XsdTagScope::GlobalComplexType, machine);
      m_stateMachines.insert(XsdTagScope::LocalComplexType, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (restriction | extension)) : simpleContent/complexContent
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::InternalState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Restriction, s2);
      machine.addTransition(startState, XsdSchemaToken::Extension, s2);

      machine.addTransition(s1, XsdSchemaToken::Restriction, s2);
      machine.addTransition(s1, XsdSchemaToken::Extension, s2);

      m_stateMachines.insert(XsdTagScope::SimpleContent, machine);
      m_stateMachines.insert(XsdTagScope::ComplexContent, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern | assertion)*)?, ((attribute | attributeGroup)*, anyAttribute?), assert*) : simpleContent restriction
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s5 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s6 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(startState, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(startState, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(startState, XsdSchemaToken::Length, s3);
      machine.addTransition(startState, XsdSchemaToken::MinLength, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(startState, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(startState, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(startState, XsdSchemaToken::Pattern, s3);
      machine.addTransition(startState, XsdSchemaToken::Assertion, s3);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s4);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(startState, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(startState, XsdSchemaToken::Assert, s6);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s1, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s1, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s1, XsdSchemaToken::Length, s3);
      machine.addTransition(s1, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s1, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s1, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s1, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s1, XsdSchemaToken::Assertion, s3);
      machine.addTransition(s1, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s1, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s1, XsdSchemaToken::Assert, s6);

      machine.addTransition(s2, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s2, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s2, XsdSchemaToken::Length, s3);
      machine.addTransition(s2, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s2, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s2, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s2, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s2, XsdSchemaToken::Assertion, s3);
      machine.addTransition(s2, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s2, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s2, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s2, XsdSchemaToken::Assert, s6);

      machine.addTransition(s3, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s3, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s3, XsdSchemaToken::Length, s3);
      machine.addTransition(s3, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s3, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s3, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s3, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s3, XsdSchemaToken::Assertion, s3);
      machine.addTransition(s3, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s3, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s3, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s3, XsdSchemaToken::Assert, s6);

      machine.addTransition(s4, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s4, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s4, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s4, XsdSchemaToken::Assert, s6);

      machine.addTransition(s5, XsdSchemaToken::Assert, s6);

      machine.addTransition(s6, XsdSchemaToken::Assert, s6);

      m_stateMachines.insert(XsdTagScope::SimpleContentRestriction, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, ((attribute | attributeGroup)*, anyAttribute?), assert*) : simple content extension
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s2);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(startState, XsdSchemaToken::AnyAttribute, s3);
      machine.addTransition(startState, XsdSchemaToken::Assert, s4);

      machine.addTransition(s1, XsdSchemaToken::Attribute, s2);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(s1, XsdSchemaToken::AnyAttribute, s3);
      machine.addTransition(s1, XsdSchemaToken::Assert, s4);

      machine.addTransition(s2, XsdSchemaToken::Attribute, s2);
      machine.addTransition(s2, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(s2, XsdSchemaToken::AnyAttribute, s3);
      machine.addTransition(s2, XsdSchemaToken::Assert, s4);

      machine.addTransition(s3, XsdSchemaToken::Assert, s4);

      machine.addTransition(s4, XsdSchemaToken::Assert, s4);

      m_stateMachines.insert(XsdTagScope::SimpleContentExtension, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, openContent?, ((group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?), assert*)) : complex content restriction/complex content extension
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s5 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s6 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::OpenContent, s2);
      machine.addTransition(startState, XsdSchemaToken::Group, s3);
      machine.addTransition(startState, XsdSchemaToken::All, s3);
      machine.addTransition(startState, XsdSchemaToken::Choice, s3);
      machine.addTransition(startState, XsdSchemaToken::Sequence, s3);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s4);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(startState, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(startState, XsdSchemaToken::Assert, s6);

      machine.addTransition(s1, XsdSchemaToken::OpenContent, s2);
      machine.addTransition(s1, XsdSchemaToken::Group, s3);
      machine.addTransition(s1, XsdSchemaToken::All, s3);
      machine.addTransition(s1, XsdSchemaToken::Choice, s3);
      machine.addTransition(s1, XsdSchemaToken::Sequence, s3);
      machine.addTransition(s1, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s1, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s1, XsdSchemaToken::Assert, s6);

      machine.addTransition(s2, XsdSchemaToken::Group, s3);
      machine.addTransition(s2, XsdSchemaToken::All, s3);
      machine.addTransition(s2, XsdSchemaToken::Choice, s3);
      machine.addTransition(s2, XsdSchemaToken::Sequence, s3);
      machine.addTransition(s2, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s2, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s2, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s2, XsdSchemaToken::Assert, s6);

      machine.addTransition(s3, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s3, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s3, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s3, XsdSchemaToken::Assert, s6);

      machine.addTransition(s4, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s4, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s4, XsdSchemaToken::AnyAttribute, s5);
      machine.addTransition(s4, XsdSchemaToken::Assert, s6);

      machine.addTransition(s5, XsdSchemaToken::Assert, s6);

      machine.addTransition(s6, XsdSchemaToken::Assert, s6);

      m_stateMachines.insert(XsdTagScope::ComplexContentRestriction, machine);
      m_stateMachines.insert(XsdTagScope::ComplexContentExtension, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, ((attribute | attributeGroup)*, anyAttribute?)) : named attribute group
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s2);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(startState, XsdSchemaToken::AnyAttribute, s3);

      machine.addTransition(s1, XsdSchemaToken::Attribute, s2);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(s1, XsdSchemaToken::AnyAttribute, s3);

      machine.addTransition(s2, XsdSchemaToken::Attribute, s2);
      machine.addTransition(s2, XsdSchemaToken::AttributeGroup, s2);
      machine.addTransition(s2, XsdSchemaToken::AnyAttribute, s3);

      m_stateMachines.insert(XsdTagScope::NamedAttributeGroup, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (all | choice | sequence)?) : group
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::All, s2);
      machine.addTransition(startState, XsdSchemaToken::Choice, s2);
      machine.addTransition(startState, XsdSchemaToken::Sequence, s2);

      machine.addTransition(s1, XsdSchemaToken::All, s2);
      machine.addTransition(s1, XsdSchemaToken::Choice, s2);
      machine.addTransition(s1, XsdSchemaToken::Sequence, s2);

      m_stateMachines.insert(XsdTagScope::NamedGroup, machine);
      m_stateMachines.insert(XsdTagScope::ReferredGroup, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (element | any)*) : all
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Element, s2);
      machine.addTransition(startState, XsdSchemaToken::Any, s2);

      machine.addTransition(s1, XsdSchemaToken::Element, s2);
      machine.addTransition(s1, XsdSchemaToken::Any, s2);

      machine.addTransition(s2, XsdSchemaToken::Element, s2);
      machine.addTransition(s2, XsdSchemaToken::Any, s2);

      m_stateMachines.insert(XsdTagScope::All, machine);
      m_stateMachines.insert(XsdTagScope::LocalAll, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (element | group | choice | sequence | any)*) : choice sequence
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Element, s2);
      machine.addTransition(startState, XsdSchemaToken::Group, s2);
      machine.addTransition(startState, XsdSchemaToken::Choice, s2);
      machine.addTransition(startState, XsdSchemaToken::Sequence, s2);
      machine.addTransition(startState, XsdSchemaToken::Any, s2);

      machine.addTransition(s1, XsdSchemaToken::Element, s2);
      machine.addTransition(s1, XsdSchemaToken::Group, s2);
      machine.addTransition(s1, XsdSchemaToken::Choice, s2);
      machine.addTransition(s1, XsdSchemaToken::Sequence, s2);
      machine.addTransition(s1, XsdSchemaToken::Any, s2);

      machine.addTransition(s2, XsdSchemaToken::Element, s2);
      machine.addTransition(s2, XsdSchemaToken::Group, s2);
      machine.addTransition(s2, XsdSchemaToken::Choice, s2);
      machine.addTransition(s2, XsdSchemaToken::Sequence, s2);
      machine.addTransition(s2, XsdSchemaToken::Any, s2);

      m_stateMachines.insert(XsdTagScope::Choice, machine);
      m_stateMachines.insert(XsdTagScope::LocalChoice, machine);
      m_stateMachines.insert(XsdTagScope::Sequence, machine);
      m_stateMachines.insert(XsdTagScope::LocalSequence, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?) : any/selector/field/notation/include/import/referred attribute group/anyAttribute/all facets/assert
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);

      m_stateMachines.insert(XsdTagScope::Any, machine);
      m_stateMachines.insert(XsdTagScope::Selector, machine);
      m_stateMachines.insert(XsdTagScope::Field, machine);
      m_stateMachines.insert(XsdTagScope::Notation, machine);
      m_stateMachines.insert(XsdTagScope::Include, machine);
      m_stateMachines.insert(XsdTagScope::Import, machine);
      m_stateMachines.insert(XsdTagScope::ReferredAttributeGroup, machine);
      m_stateMachines.insert(XsdTagScope::AnyAttribute, machine);
      m_stateMachines.insert(XsdTagScope::MinExclusiveFacet, machine);
      m_stateMachines.insert(XsdTagScope::MinInclusiveFacet, machine);
      m_stateMachines.insert(XsdTagScope::MaxExclusiveFacet, machine);
      m_stateMachines.insert(XsdTagScope::MaxInclusiveFacet, machine);
      m_stateMachines.insert(XsdTagScope::TotalDigitsFacet, machine);
      m_stateMachines.insert(XsdTagScope::FractionDigitsFacet, machine);
      m_stateMachines.insert(XsdTagScope::LengthFacet, machine);
      m_stateMachines.insert(XsdTagScope::MinLengthFacet, machine);
      m_stateMachines.insert(XsdTagScope::MaxLengthFacet, machine);
      m_stateMachines.insert(XsdTagScope::EnumerationFacet, machine);
      m_stateMachines.insert(XsdTagScope::WhiteSpaceFacet, machine);
      m_stateMachines.insert(XsdTagScope::PatternFacet, machine);
      m_stateMachines.insert(XsdTagScope::Assert, machine);
      m_stateMachines.insert(XsdTagScope::Assertion, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (selector, field+)) : unique/key/keyref
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::InternalState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::InternalState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Selector, s2);

      machine.addTransition(s1, XsdSchemaToken::Selector, s2);
      machine.addTransition(s2, XsdSchemaToken::Field, s3);
      machine.addTransition(s3, XsdSchemaToken::Field, s3);

      m_stateMachines.insert(XsdTagScope::Unique, machine);
      m_stateMachines.insert(XsdTagScope::Key, machine);
      m_stateMachines.insert(XsdTagScope::KeyRef, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (simpleType | complexType)?) : alternative
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(startState, XsdSchemaToken::ComplexType, s2);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s1, XsdSchemaToken::ComplexType, s2);

      m_stateMachines.insert(XsdTagScope::Alternative, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (appinfo | documentation)* : annotation
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Appinfo, s1);
      machine.addTransition(startState, XsdSchemaToken::Documentation, s1);

      machine.addTransition(s1, XsdSchemaToken::Appinfo, s1);
      machine.addTransition(s1, XsdSchemaToken::Documentation, s1);

      m_stateMachines.insert(XsdTagScope::Annotation, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (restriction | list | union)) : simpleType
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::InternalState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Restriction, s2);
      machine.addTransition(startState, XsdSchemaToken::List, s2);
      machine.addTransition(startState, XsdSchemaToken::Union, s2);

      machine.addTransition(s1, XsdSchemaToken::Restriction, s2);
      machine.addTransition(s1, XsdSchemaToken::List, s2);
      machine.addTransition(s1, XsdSchemaToken::Union, s2);

      m_stateMachines.insert(XsdTagScope::GlobalSimpleType, machine);
      m_stateMachines.insert(XsdTagScope::LocalSimpleType, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern | assertion)*)) : simple type restriction
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(startState, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(startState, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(startState, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(startState, XsdSchemaToken::Length, s3);
      machine.addTransition(startState, XsdSchemaToken::MinLength, s3);
      machine.addTransition(startState, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(startState, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(startState, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(startState, XsdSchemaToken::Pattern, s3);
      machine.addTransition(startState, XsdSchemaToken::Assertion, s3);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s1, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s1, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s1, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s1, XsdSchemaToken::Length, s3);
      machine.addTransition(s1, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s1, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s1, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s1, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s1, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s1, XsdSchemaToken::Assertion, s3);

      machine.addTransition(s2, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s2, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s2, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s2, XsdSchemaToken::Length, s3);
      machine.addTransition(s2, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s2, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s2, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s2, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s2, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s2, XsdSchemaToken::Assertion, s3);

      machine.addTransition(s3, XsdSchemaToken::MinExclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MinInclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxExclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxInclusive, s3);
      machine.addTransition(s3, XsdSchemaToken::TotalDigits, s3);
      machine.addTransition(s3, XsdSchemaToken::FractionDigits, s3);
      machine.addTransition(s3, XsdSchemaToken::Length, s3);
      machine.addTransition(s3, XsdSchemaToken::MinLength, s3);
      machine.addTransition(s3, XsdSchemaToken::MaxLength, s3);
      machine.addTransition(s3, XsdSchemaToken::Enumeration, s3);
      machine.addTransition(s3, XsdSchemaToken::WhiteSpace, s3);
      machine.addTransition(s3, XsdSchemaToken::Pattern, s3);
      machine.addTransition(s3, XsdSchemaToken::Assertion, s3);

      m_stateMachines.insert(XsdTagScope::SimpleRestriction, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, simpleType?) : list
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);

      m_stateMachines.insert(XsdTagScope::List, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, simpleType*) : union
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s2);

      machine.addTransition(s1, XsdSchemaToken::SimpleType, s2);
      machine.addTransition(s2, XsdSchemaToken::SimpleType, s2);

      m_stateMachines.insert(XsdTagScope::Union, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for ((include | import | redefine |i override |  annotation)*, (defaultOpenContent, annotation*)?, (((simpleType | complexType | group | attributeGroup) | element | attribute | notation), annotation*)*) : schema
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s3 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s4 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s5 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Include, s1);
      machine.addTransition(startState, XsdSchemaToken::Import, s1);
      machine.addTransition(startState, XsdSchemaToken::Redefine, s1);
      machine.addTransition(startState, XsdSchemaToken::Override, s1);
      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::DefaultOpenContent, s2);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(startState, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(startState, XsdSchemaToken::Group, s4);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(startState, XsdSchemaToken::Element, s4);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s4);
      machine.addTransition(startState, XsdSchemaToken::Notation, s4);

      machine.addTransition(s1, XsdSchemaToken::Include, s1);
      machine.addTransition(s1, XsdSchemaToken::Import, s1);
      machine.addTransition(s1, XsdSchemaToken::Redefine, s1);
      machine.addTransition(s1, XsdSchemaToken::Override, s1);
      machine.addTransition(s1, XsdSchemaToken::Annotation, s1);
      machine.addTransition(s1, XsdSchemaToken::DefaultOpenContent, s2);
      machine.addTransition(s1, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(s1, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(s1, XsdSchemaToken::Group, s4);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s1, XsdSchemaToken::Element, s4);
      machine.addTransition(s1, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s1, XsdSchemaToken::Notation, s4);

      machine.addTransition(s2, XsdSchemaToken::Annotation, s3);
      machine.addTransition(s2, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(s2, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(s2, XsdSchemaToken::Group, s4);
      machine.addTransition(s2, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s2, XsdSchemaToken::Element, s4);
      machine.addTransition(s2, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s2, XsdSchemaToken::Notation, s4);

      machine.addTransition(s3, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(s3, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(s3, XsdSchemaToken::Group, s4);
      machine.addTransition(s3, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s3, XsdSchemaToken::Element, s4);
      machine.addTransition(s3, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s3, XsdSchemaToken::Notation, s4);

      machine.addTransition(s4, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(s4, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(s4, XsdSchemaToken::Group, s4);
      machine.addTransition(s4, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s4, XsdSchemaToken::Element, s4);
      machine.addTransition(s4, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s4, XsdSchemaToken::Notation, s4);
      machine.addTransition(s4, XsdSchemaToken::Annotation, s5);

      machine.addTransition(s5, XsdSchemaToken::SimpleType, s4);
      machine.addTransition(s5, XsdSchemaToken::ComplexType, s4);
      machine.addTransition(s5, XsdSchemaToken::Group, s4);
      machine.addTransition(s5, XsdSchemaToken::AttributeGroup, s4);
      machine.addTransition(s5, XsdSchemaToken::Element, s4);
      machine.addTransition(s5, XsdSchemaToken::Attribute, s4);
      machine.addTransition(s5, XsdSchemaToken::Notation, s4);
      machine.addTransition(s5, XsdSchemaToken::Annotation, s5);

      m_stateMachines.insert(XsdTagScope::Schema, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation?, any) : defaultOpenContent
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::InternalState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s2 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::Any, s2);

      machine.addTransition(s1, XsdSchemaToken::Any, s2);

      m_stateMachines.insert(XsdTagScope::DefaultOpenContent, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation | (simpleType | complexType | group | attributeGroup))* : redefine
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s1);
      machine.addTransition(startState, XsdSchemaToken::ComplexType, s1);
      machine.addTransition(startState, XsdSchemaToken::Group, s1);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s1);

      machine.addTransition(s1, XsdSchemaToken::Annotation, s1);
      machine.addTransition(s1, XsdSchemaToken::SimpleType, s1);
      machine.addTransition(s1, XsdSchemaToken::ComplexType, s1);
      machine.addTransition(s1, XsdSchemaToken::Group, s1);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s1);

      m_stateMachines.insert(XsdTagScope::Redefine, machine);
   }

   {
      XsdStateMachine<XsdSchemaToken::NodeName> machine(namePool);

      // setup state machine for (annotation | (simpleType | complexType | group | attributeGroup | element | attribute | notation))* : override
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId startState = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::StartEndState);
      const XsdStateMachine<XsdSchemaToken::NodeName>::StateId s1 = machine.addState(
               XsdStateMachine<XsdSchemaToken::NodeName>::EndState);

      machine.addTransition(startState, XsdSchemaToken::Annotation, s1);
      machine.addTransition(startState, XsdSchemaToken::SimpleType, s1);
      machine.addTransition(startState, XsdSchemaToken::ComplexType, s1);
      machine.addTransition(startState, XsdSchemaToken::Group, s1);
      machine.addTransition(startState, XsdSchemaToken::AttributeGroup, s1);
      machine.addTransition(startState, XsdSchemaToken::Element, s1);
      machine.addTransition(startState, XsdSchemaToken::Attribute, s1);
      machine.addTransition(startState, XsdSchemaToken::Notation, s1);

      machine.addTransition(s1, XsdSchemaToken::Annotation, s1);
      machine.addTransition(s1, XsdSchemaToken::SimpleType, s1);
      machine.addTransition(s1, XsdSchemaToken::ComplexType, s1);
      machine.addTransition(s1, XsdSchemaToken::Group, s1);
      machine.addTransition(s1, XsdSchemaToken::AttributeGroup, s1);
      machine.addTransition(s1, XsdSchemaToken::Element, s1);
      machine.addTransition(s1, XsdSchemaToken::Attribute, s1);
      machine.addTransition(s1, XsdSchemaToken::Notation, s1);

      m_stateMachines.insert(XsdTagScope::Override, machine);
   }
}

void XsdSchemaParser::setupBuiltinTypeNames()
{
   NamePool::Ptr namePool(m_namePool);
   m_builtinTypeNames.reserve(48);

   m_builtinTypeNames.insert(BuiltinTypes::xsAnyType->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsAnySimpleType->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUntyped->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsAnyAtomicType->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUntypedAtomic->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDateTime->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDate->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsTime->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDuration->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsYearMonthDuration->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDayTimeDuration->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsFloat->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDouble->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsInteger->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsDecimal->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNonPositiveInteger->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNegativeInteger->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsLong->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsInt->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsShort->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsByte->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNonNegativeInteger->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUnsignedLong->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUnsignedInt->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUnsignedShort->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsUnsignedByte->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsPositiveInteger->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsGYearMonth->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsGYear->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsGMonthDay->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsGDay->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsGMonth->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsBoolean->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsBase64Binary->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsHexBinary->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsAnyURI->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsQName->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsString->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNormalizedString->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsToken->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsLanguage->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNMTOKEN->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsName->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNCName->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsID->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsIDREF->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsENTITY->name(namePool));
   m_builtinTypeNames.insert(BuiltinTypes::xsNOTATION->name(namePool));
}
