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

#include "qxsdschemaparsercontext_p.h"

using namespace QPatternist;

XsdSchemaParserContext::XsdSchemaParserContext(const NamePool::Ptr &namePool, const XsdSchemaContext::Ptr &context)
   : m_namePool(namePool), m_schema(new XsdSchema(m_namePool)), m_checker(new XsdSchemaChecker(context, this)),
     m_resolver(new XsdSchemaResolver(context, this)), m_elementDescriptions(setupElementDescriptions())
{
}

NamePool::Ptr XsdSchemaParserContext::namePool() const
{
   return m_namePool;
}

XsdSchemaResolver::Ptr XsdSchemaParserContext::resolver() const
{
   return m_resolver;
}

XsdSchemaChecker::Ptr XsdSchemaParserContext::checker() const
{
   return m_checker;
}

XsdSchema::Ptr XsdSchemaParserContext::schema() const
{
   return m_schema;
}

ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash XsdSchemaParserContext::elementDescriptions() const
{
   return m_elementDescriptions;
}

QXmlName XsdSchemaParserContext::createAnonymousName(const QString &targetNamespace) const
{
   m_anonymousNameCounter.ref();

   const QString name = QString::fromLatin1("__AnonymousClass_%1").formatArg(m_anonymousNameCounter.load());

   return m_namePool->allocateQName(targetNamespace, name);
}

ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash  XsdSchemaParserContext::setupElementDescriptions()
{
   enum {
      ReservedForElements = 60
   };

   ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash elementDescriptions;
   elementDescriptions.reserve(ReservedForElements);

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Schema];
      description.optionalAttributes.reserve(10);
      //description.tagToken = XsdSchemaToken::Schema;
      description.optionalAttributes.insert(XsdSchemaToken::AttributeFormDefault);
      description.optionalAttributes.insert(XsdSchemaToken::BlockDefault);
      description.optionalAttributes.insert(XsdSchemaToken::DefaultAttributes);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
      description.optionalAttributes.insert(XsdSchemaToken::ElementFormDefault);
      description.optionalAttributes.insert(XsdSchemaToken::FinalDefault);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::TargetNamespace);
      description.optionalAttributes.insert(XsdSchemaToken::Version);
      description.optionalAttributes.insert(XsdSchemaToken::XmlLanguage);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Include];
      //description.tagToken = XsdSchemaToken::Include;
      description.requiredAttributes.insert(XsdSchemaToken::SchemaLocation);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Import];
      //description.tagToken = XsdSchemaToken::Import;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Namespace);
      description.optionalAttributes.insert(XsdSchemaToken::SchemaLocation);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Redefine];
      //description.tagToken = XsdSchemaToken::Redefine;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::SchemaLocation);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Override];
      //description.tagToken = XsdSchemaToken::Override;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::SchemaLocation);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Annotation];
      //description.tagToken = XsdSchemaToken::Annotation;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::AppInfo];
      //description.tagToken = XsdSchemaToken::Appinfo;
      description.optionalAttributes.insert(XsdSchemaToken::Source);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Documentation];
      //description.tagToken = XsdSchemaToken::Documentation;
      description.optionalAttributes.insert(XsdSchemaToken::Source);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::GlobalSimpleType];
      //description.tagToken = XsdSchemaToken::SimpleType;
      description.optionalAttributes.insert(XsdSchemaToken::Final);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalSimpleType];
      //description.tagToken = XsdSchemaToken::SimpleType;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::SimpleRestriction];
      //description.tagToken = XsdSchemaToken::Restriction;
      description.optionalAttributes.insert(XsdSchemaToken::Base);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::List];
      //description.tagToken = XsdSchemaToken::List;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::ItemType);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Union];
      //description.tagToken = XsdSchemaToken::Union;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MemberTypes);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::MinExclusiveFacet];
      //description.tagToken = XsdSchemaToken::MinExclusive;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::MinInclusiveFacet];
      //description.tagToken = XsdSchemaToken::MinInclusive;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::MaxExclusiveFacet];
      //description.tagToken = XsdSchemaToken::MaxExclusive;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::MaxInclusiveFacet];
      //description.tagToken = XsdSchemaToken::MaxInclusive;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::TotalDigitsFacet];
      //description.tagToken = XsdSchemaToken::TotalDigits;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::FractionDigitsFacet];
      //description.tagToken = XsdSchemaToken::FractionDigits;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LengthFacet];
      //description.tagToken = XsdSchemaToken::Length;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::MinLengthFacet];
      //description.tagToken = XsdSchemaToken::MinLength;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::MaxLengthFacet];
      //description.tagToken = XsdSchemaToken::MaxLength;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::EnumerationFacet];
      //description.tagToken = XsdSchemaToken::Enumeration;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::WhiteSpaceFacet];
      //description.tagToken = XsdSchemaToken::WhiteSpace;
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::PatternFacet];
      //description.tagToken = XsdSchemaToken::Pattern;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Value);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::GlobalComplexType];
      description.optionalAttributes.reserve(7);
      //description.tagToken = XsdSchemaToken::ComplexType;
      description.optionalAttributes.insert(XsdSchemaToken::Abstract);
      description.optionalAttributes.insert(XsdSchemaToken::Block);
      description.optionalAttributes.insert(XsdSchemaToken::DefaultAttributesApply);
      description.optionalAttributes.insert(XsdSchemaToken::Final);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Mixed);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalComplexType];
      //description.tagToken = XsdSchemaToken::ComplexType;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Mixed);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::SimpleContent];
      //description.tagToken = XsdSchemaToken::SimpleContent;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::SimpleContentRestriction];
      //description.tagToken = XsdSchemaToken::Restriction;
      description.requiredAttributes.insert(XsdSchemaToken::Base);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::SimpleContentExtension];
      //description.tagToken = XsdSchemaToken::Extension;
      description.requiredAttributes.insert(XsdSchemaToken::Base);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::ComplexContent];
      //description.tagToken = XsdSchemaToken::ComplexContent;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Mixed);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::ComplexContentRestriction];
      //description.tagToken = XsdSchemaToken::Restriction;
      description.requiredAttributes.insert(XsdSchemaToken::Base);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::ComplexContentExtension];
      //description.tagToken = XsdSchemaToken::Extension;
      description.requiredAttributes.insert(XsdSchemaToken::Base);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::NamedGroup];
      //description.tagToken = XsdSchemaToken::Group;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::ReferredGroup];
      description.optionalAttributes.reserve(4);
      //description.tagToken = XsdSchemaToken::Group;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
      description.requiredAttributes.insert(XsdSchemaToken::Ref);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::All];
      //description.tagToken = XsdSchemaToken::All;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalAll];
      //description.tagToken = XsdSchemaToken::All;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Choice];
      //description.tagToken = XsdSchemaToken::Choice;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalChoice];
      //description.tagToken = XsdSchemaToken::Choice;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Sequence];
      //description.tagToken = XsdSchemaToken::Sequence;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalSequence];
      //description.tagToken = XsdSchemaToken::Sequence;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::GlobalAttribute];
      description.optionalAttributes.reserve(5);
      //description.tagToken = XsdSchemaToken::Attribute;
      description.optionalAttributes.insert(XsdSchemaToken::Default);
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
      description.optionalAttributes.insert(XsdSchemaToken::Type);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalAttribute];
      description.optionalAttributes.reserve(8);
      //description.tagToken = XsdSchemaToken::Attribute;
      description.optionalAttributes.insert(XsdSchemaToken::Default);
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Form);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Name);
      description.optionalAttributes.insert(XsdSchemaToken::Ref);
      description.optionalAttributes.insert(XsdSchemaToken::Type);
      description.optionalAttributes.insert(XsdSchemaToken::Use);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::NamedAttributeGroup];
      //description.tagToken = XsdSchemaToken::AttributeGroup;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::ReferredAttributeGroup];
      //description.tagToken = XsdSchemaToken::AttributeGroup;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Ref);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::LocalElement];
      description.optionalAttributes.reserve(11);
      //description.tagToken = XsdSchemaToken::Element;
      description.optionalAttributes.insert(XsdSchemaToken::Block);
      description.optionalAttributes.insert(XsdSchemaToken::Default);
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Form);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::Name);
      description.optionalAttributes.insert(XsdSchemaToken::Nillable);
      description.optionalAttributes.insert(XsdSchemaToken::Ref);
      description.optionalAttributes.insert(XsdSchemaToken::Type);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::GlobalElement];
      description.optionalAttributes.reserve(10);
      //description.tagToken = XsdSchemaToken::Element;
      description.optionalAttributes.insert(XsdSchemaToken::Abstract);
      description.optionalAttributes.insert(XsdSchemaToken::Block);
      description.optionalAttributes.insert(XsdSchemaToken::Default);
      description.optionalAttributes.insert(XsdSchemaToken::Final);
      description.optionalAttributes.insert(XsdSchemaToken::Fixed);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
      description.optionalAttributes.insert(XsdSchemaToken::Nillable);
      description.optionalAttributes.insert(XsdSchemaToken::SubstitutionGroup);
      description.optionalAttributes.insert(XsdSchemaToken::Type);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Unique];
      //description.tagToken = XsdSchemaToken::Unique;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Key];
      //description.tagToken = XsdSchemaToken::Key;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::KeyRef];
      //description.tagToken = XsdSchemaToken::Keyref;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
      description.requiredAttributes.insert(XsdSchemaToken::Refer);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Selector];
      //description.tagToken = XsdSchemaToken::Selector;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Xpath);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Field];
      //description.tagToken = XsdSchemaToken::Field;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Xpath);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Notation];
      description.optionalAttributes.reserve(4);
      //description.tagToken = XsdSchemaToken::Notation;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Name);
      description.optionalAttributes.insert(XsdSchemaToken::Public);
      description.optionalAttributes.insert(XsdSchemaToken::System);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Any];
      description.optionalAttributes.reserve(7);
      //description.tagToken = XsdSchemaToken::Any;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::MaxOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::MinOccurs);
      description.optionalAttributes.insert(XsdSchemaToken::Namespace);
      description.optionalAttributes.insert(XsdSchemaToken::NotNamespace);
      description.optionalAttributes.insert(XsdSchemaToken::NotQName);
      description.optionalAttributes.insert(XsdSchemaToken::ProcessContents);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::AnyAttribute];
      description.optionalAttributes.reserve(5);
      //description.tagToken = XsdSchemaToken::AnyAttribute;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Namespace);
      description.optionalAttributes.insert(XsdSchemaToken::NotNamespace);
      description.optionalAttributes.insert(XsdSchemaToken::NotQName);
      description.optionalAttributes.insert(XsdSchemaToken::ProcessContents);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Alternative];
      //description.tagToken = XsdSchemaToken::Alternative;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Test);
      description.optionalAttributes.insert(XsdSchemaToken::Type);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::OpenContent];
      //description.tagToken = XsdSchemaToken::OpenContent;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Mode);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description =
         elementDescriptions[XsdTagScope::DefaultOpenContent];
      //description.tagToken = XsdSchemaToken::DefaultOpenContent;
      description.optionalAttributes.insert(XsdSchemaToken::AppliesToEmpty);
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.optionalAttributes.insert(XsdSchemaToken::Mode);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Assert];
      //description.tagToken = XsdSchemaToken::Assert;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Test);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
   }

   {
      ElementDescription<XsdSchemaToken, XsdTagScope::Type> &description = elementDescriptions[XsdTagScope::Assertion];
      //description.tagToken = XsdSchemaToken::Assertion;
      description.optionalAttributes.insert(XsdSchemaToken::Id);
      description.requiredAttributes.insert(XsdSchemaToken::Test);
      description.optionalAttributes.insert(XsdSchemaToken::XPathDefaultNamespace);
   }

   Q_ASSERT_X(elementDescriptions.count() == ReservedForElements, Q_FUNC_INFO,
              csPrintable(QString::fromLatin1("Expected is %1, actual is %2.").formatArg(ReservedForElements).formatArg(
                            elementDescriptions.count())));

   return elementDescriptions;
}
