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

#include "qxsdschemadebugger_p.h"

using namespace QPatternist;

XsdSchemaDebugger::XsdSchemaDebugger(const NamePool::Ptr &namePool)
   : m_namePool(namePool)
{
}

void XsdSchemaDebugger::dumpParticle(const XsdParticle::Ptr &particle, int level)
{
   QString prefix;
   prefix.fill(QLatin1Char(' '), level);

   qDebug("%s min=%s max=%s", csPrintable(prefix), csPrintable(QString::number(particle->minimumOccurs())),
          csPrintable(particle->maximumOccursUnbounded() ? QLatin1String("unbounded") : QString::number(
                        particle->maximumOccurs())));

   if (particle->term()->isElement()) {
      qDebug("%selement (%s)", csPrintable(prefix), csPrintable(XsdElement::Ptr(particle->term())->displayName(m_namePool)));
   } else if (particle->term()->isModelGroup()) {
      const XsdModelGroup::Ptr group(particle->term());
      if (group->compositor() == XsdModelGroup::SequenceCompositor) {
         qDebug("%ssequence", csPrintable(prefix));
      } else if (group->compositor() == XsdModelGroup::AllCompositor) {
         qDebug("%sall", csPrintable(prefix));
      } else if (group->compositor() == XsdModelGroup::ChoiceCompositor) {
         qDebug("%schoice", csPrintable(prefix));
      }

      for (int i = 0; i < group->particles().count(); ++i) {
         dumpParticle(group->particles().at(i), level + 5);
      }
   } else if (particle->term()->isWildcard()) {
      XsdWildcard::Ptr wildcard(particle->term());
      qDebug("%swildcard (process=%d)", csPrintable(prefix), wildcard->processContents());
   }
}

void XsdSchemaDebugger::dumpInheritance(const SchemaType::Ptr &type, int level)
{
   QString prefix;
   prefix.fill(QLatin1Char(' '), level);
   qDebug("%s-->%s", csPrintable(prefix), csPrintable(type->displayName(m_namePool)));
   if (type->wxsSuperType()) {
      dumpInheritance(type->wxsSuperType(), ++level);
   }
}

void XsdSchemaDebugger::dumpWildcard(const XsdWildcard::Ptr &wildcard)
{
   QVector<QString> varietyNames;
   varietyNames.append(QLatin1String("Any"));
   varietyNames.append(QLatin1String("Enumeration"));
   varietyNames.append(QLatin1String("Not"));

   QVector<QString> processContentsNames;
   processContentsNames.append(QLatin1String("Strict"));
   processContentsNames.append(QLatin1String("Lax"));
   processContentsNames.append(QLatin1String("Skip"));

   qDebug("      processContents: %s", csPrintable(processContentsNames.at((int)wildcard->processContents())));
   const XsdWildcard::NamespaceConstraint::Ptr constraint = wildcard->namespaceConstraint();
   qDebug("      variety: %s", csPrintable(varietyNames.at((int)constraint->variety())));
   if (constraint->variety() != XsdWildcard::NamespaceConstraint::Any) {
      qDebug() << "      namespaces:" << constraint->namespaces();
   }
}

void XsdSchemaDebugger::dumpType(const SchemaType::Ptr &type)
{
   if (type->isComplexType()) {
      const XsdComplexType::Ptr complexType(type);

      qDebug("\n+++ Complex Type +++");
      qDebug("Name: %s (abstract: %s)", csPrintable(complexType->displayName(m_namePool)),
             complexType->isAbstract() ? "yes" : "no");

      if (complexType->wxsSuperType()) {
         qDebug("  base type: %s", csPrintable(complexType->wxsSuperType()->displayName(m_namePool)));
      } else {
         qDebug("  base type: (none)");
      }
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
         qDebug("  content type: empty");
      }
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         qDebug("  content type: simple");
      }
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly) {
         qDebug("  content type: element-only");
      }
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed) {
         qDebug("  content type: mixed");
      }
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         if (complexType->contentType()->simpleType()) {
            qDebug("  simple type: %s", csPrintable(complexType->contentType()->simpleType()->displayName(m_namePool)));
         } else {
            qDebug("  simple type: (none)");
         }
      }

      const XsdAttributeUse::List uses = complexType->attributeUses();
      qDebug("   %zd attributes", uses.count());

      for (int i = 0; i < uses.count(); ++i) {
         qDebug("      attr: %s", csPrintable(uses.at(i)->attribute()->displayName(m_namePool)));
      }
      qDebug("   has attribute wildcard: %s", complexType->attributeWildcard() ? "yes" : "no");

      if (complexType->attributeWildcard()) {
         dumpWildcard(complexType->attributeWildcard());
      }

      if (complexType->contentType()->particle()) {
         dumpParticle(complexType->contentType()->particle(), 5);
      }

   } else {
      qDebug("\n+++ Simple Type +++");
      qDebug("Name: %s", csPrintable(type->displayName(m_namePool)));

      if (type->isDefinedBySchema()) {
         const XsdSimpleType::Ptr simpleType(type);
         if (simpleType->primitiveType()) {
            qDebug("  primitive type: %s", csPrintable(simpleType->primitiveType()->displayName(m_namePool)));
         } else {
            qDebug("  primitive type: (none)");
         }
      }

      dumpInheritance(type, 0);
   }
}


void XsdSchemaDebugger::dumpElement(const XsdElement::Ptr &element)
{
   QStringList disallowedSubstGroup;
   if (element->disallowedSubstitutions() & XsdElement::RestrictionConstraint) {
      disallowedSubstGroup << QLatin1String("restriction");
   }
   if (element->disallowedSubstitutions() & XsdElement::ExtensionConstraint) {
      disallowedSubstGroup << QLatin1String("extension");
   }
   if (element->disallowedSubstitutions() & XsdElement::SubstitutionConstraint) {
      disallowedSubstGroup << QLatin1String("substitution");
   }


   qDebug() << "Name:" << element->displayName(m_namePool);
   qDebug() << "IsAbstract:" << (element->isAbstract() ? "yes" : "no");
   qDebug() << "Type:" << element->type()->displayName(m_namePool);
   qDebug() << "DisallowedSubstitutionGroups:" << disallowedSubstGroup.join(QLatin1String("' "));
}

void XsdSchemaDebugger::dumpAttribute(const XsdAttribute::Ptr &attribute)
{
   qDebug() << "Name:" << attribute->displayName(m_namePool);
   qDebug() << "Type:" << attribute->type()->displayName(m_namePool);
}

void XsdSchemaDebugger::dumpSchema(const XsdSchema::Ptr &schema)
{
   qDebug() << "------------------------------ Schema -------------------------------";

   // elements
   {
      qDebug() << "Global Elements:";
      const XsdElement::List elements = schema->elements();
      for (int i = 0; i < elements.count(); ++i) {
         dumpElement(elements.at(i));
      }
   }

   // attributes
   {
      qDebug() << "Global Attributes:";
      const XsdAttribute::List attributes = schema->attributes();
      for (int i = 0; i < attributes.count(); ++i) {
         dumpAttribute(attributes.at(i));
      }
   }

   // types
   {
      qDebug() << "Global Types:";
      const SchemaType::List types = schema->types();
      for (int i = 0; i < types.count(); ++i) {
         dumpType(types.at(i));
      }
   }

   // anonymous types
   {
      qDebug() << "Anonymous Types:";
      const SchemaType::List types = schema->anonymousTypes();
      for (int i = 0; i < types.count(); ++i) {
         dumpType(types.at(i));
      }
   }

   qDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
}
