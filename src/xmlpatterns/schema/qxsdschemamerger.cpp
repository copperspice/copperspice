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

#include "qxsdschemamerger_p.h"

using namespace QPatternist;

XsdSchemaMerger::XsdSchemaMerger(const XsdSchema::Ptr &schema, const XsdSchema::Ptr &otherSchema)
{
   merge(schema, otherSchema);
}

XsdSchema::Ptr XsdSchemaMerger::mergedSchema() const
{
   return m_mergedSchema;
}

void XsdSchemaMerger::merge(const XsdSchema::Ptr &schema, const XsdSchema::Ptr &otherSchema)
{
   m_mergedSchema = XsdSchema::Ptr(new XsdSchema(otherSchema->namePool()));

   // first fill the merged schema with the values from schema
   if (schema) {
      const XsdElement::List elements = schema->elements();
      for (int i = 0; i < elements.count(); ++i) {
         m_mergedSchema->addElement(elements.at(i));
      }

      const XsdAttribute::List attributes = schema->attributes();
      for (int i = 0; i < attributes.count(); ++i) {
         m_mergedSchema->addAttribute(attributes.at(i));
      }

      const SchemaType::List types = schema->types();
      for (int i = 0; i < types.count(); ++i) {
         m_mergedSchema->addType(types.at(i));
      }

      const SchemaType::List anonymousTypes = schema->anonymousTypes();
      for (int i = 0; i < anonymousTypes.count(); ++i) {
         m_mergedSchema->addAnonymousType(anonymousTypes.at(i));
      }

      const XsdModelGroup::List elementGroups = schema->elementGroups();
      for (int i = 0; i < elementGroups.count(); ++i) {
         m_mergedSchema->addElementGroup(elementGroups.at(i));
      }

      const XsdAttributeGroup::List attributeGroups = schema->attributeGroups();
      for (int i = 0; i < attributeGroups.count(); ++i) {
         m_mergedSchema->addAttributeGroup(attributeGroups.at(i));
      }

      const XsdNotation::List notations = schema->notations();
      for (int i = 0; i < notations.count(); ++i) {
         m_mergedSchema->addNotation(notations.at(i));
      }

      const XsdIdentityConstraint::List identityConstraints = schema->identityConstraints();
      for (int i = 0; i < identityConstraints.count(); ++i) {
         m_mergedSchema->addIdentityConstraint(identityConstraints.at(i));
      }
   }

   // then merge in the values from the otherSchema
   {
      const XsdElement::List elements = otherSchema->elements();
      for (int i = 0; i < elements.count(); ++i) {
         if (!m_mergedSchema->element(elements.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addElement(elements.at(i));
         }
      }

      const XsdAttribute::List attributes = otherSchema->attributes();
      for (int i = 0; i < attributes.count(); ++i) {
         if (!m_mergedSchema->attribute(attributes.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addAttribute(attributes.at(i));
         }
      }

      const SchemaType::List types = otherSchema->types();
      for (int i = 0; i < types.count(); ++i) {
         if (!m_mergedSchema->type(types.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addType(types.at(i));
         }
      }

      const SchemaType::List anonymousTypes = otherSchema->anonymousTypes();
      for (int i = 0; i < anonymousTypes.count(); ++i) {
         // add anonymous type as they are
         m_mergedSchema->addAnonymousType(anonymousTypes.at(i));
      }

      const XsdModelGroup::List elementGroups = otherSchema->elementGroups();
      for (int i = 0; i < elementGroups.count(); ++i) {
         if (!m_mergedSchema->elementGroup(elementGroups.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addElementGroup(elementGroups.at(i));
         }
      }

      const XsdAttributeGroup::List attributeGroups = otherSchema->attributeGroups();
      for (int i = 0; i < attributeGroups.count(); ++i) {
         if (!m_mergedSchema->attributeGroup(attributeGroups.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addAttributeGroup(attributeGroups.at(i));
         }
      }

      const XsdNotation::List notations = otherSchema->notations();
      for (int i = 0; i < notations.count(); ++i) {
         if (!m_mergedSchema->notation(notations.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addNotation(notations.at(i));
         }
      }

      const XsdIdentityConstraint::List identityConstraints = otherSchema->identityConstraints();
      for (int i = 0; i < identityConstraints.count(); ++i) {
         if (!m_mergedSchema->identityConstraint(identityConstraints.at(i)->name(otherSchema->namePool()))) {
            m_mergedSchema->addIdentityConstraint(identityConstraints.at(i));
         }
      }
   }
}
