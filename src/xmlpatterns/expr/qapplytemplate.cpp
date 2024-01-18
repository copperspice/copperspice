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

#include <QDebug>

#include "qaxisstep_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qitemmappingiterator_p.h"
#include "qsequencemappingiterator_p.h"
#include "qpatternistlocale_p.h"

#include "qapplytemplate_p.h"

using namespace QPatternist;

ApplyTemplate::ApplyTemplate(const TemplateMode::Ptr &mode,
                             const WithParam::Hash &withParams,
                             const TemplateMode::Ptr &defaultMode) : TemplateInvoker(withParams)
   , m_mode(mode)
   , m_defaultMode(defaultMode)
{
   Q_ASSERT_X(m_mode || m_defaultMode, Q_FUNC_INFO,
              "Either a mode, or the default mode must be supplied.");
}

Item ApplyTemplate::mapToItem(const QXmlNodeModelIndex &node,
                              const DynamicContext::Ptr &) const
{
   return Item(node);
}

Item::Iterator::Ptr ApplyTemplate::mapToSequence(const Item &item,
      const DynamicContext::Ptr &context) const
{
   (void) item;
   return evaluateSequence(context);
}

TemplateMode::Ptr ApplyTemplate::effectiveMode(const DynamicContext::Ptr &context) const
{
   if (m_mode) {
      return m_mode;
   } else {
      const TemplateMode::Ptr currentMode(context->currentTemplateMode());

      if (currentMode) {
         return currentMode;
      } else {
         return m_defaultMode;
      }
   }
}

Template::Ptr ApplyTemplate::findTemplate(const DynamicContext::Ptr &context,
      const TemplateMode::Ptr &templateMode) const
{
   const int count = templateMode->templatePatterns.count();
   Template::Ptr result;
   /* It's redundant to initialize these values, but it suppresses false
    * positives with GCC. */
   PatternPriority priority = 0;
   TemplatePattern::ID id = -1;

   /* Possible optimization: detecting ambiguous rule matches could be forked off to a
    * low prioirity thread. */
   for (int i = 0; i < count; ++i) {
      const TemplatePattern::Ptr &candidate = templateMode->templatePatterns.at(i);
      if (candidate->matchPattern()->evaluateEBV(context)) {
         if (result) {
            if (   candidate->id() != id
                   && candidate->priority() == priority
                   && candidate->templateTarget()->importPrecedence ==
                   result->importPrecedence) {
               context->error(QtXmlPatterns::tr("Ambiguous rule match."),
                              ReportContext::XTRE0540, this);
            } else {
               break;
            }
         } else {
            result = candidate->templateTarget();
            priority = candidate->priority();
            id = candidate->id();
         }
      }
   }

   return result;
}

Item::Iterator::Ptr ApplyTemplate::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const TemplateMode::Ptr templateMode(effectiveMode(context));
   const Template::Ptr &templateMatch = findTemplate(context, templateMode);

   if (templateMatch) {
      return templateMatch->body->evaluateSequence(templateMatch->createContext(this, context, false));
   } else {
      /* None of our templates matched. Proceed with a built-in. */
      const Item current(context->contextItem());
      // TODO it can be an atomic value?
      const QXmlNodeModelIndex::NodeKind kind(current.asNode().kind());

      if (kind == QXmlNodeModelIndex::Element || kind == QXmlNodeModelIndex::Document) {
         pDebug() << "No template match, using builtin template for element() | document-node()";

         const Item::Iterator::Ptr focusIterator(makeItemMappingIterator<Item>(ConstPtr(this),
                                                 current.asNode().iterate(QXmlNodeModelIndex::AxisChild),
                                                 context));

         const DynamicContext::Ptr focus(context->createFocus());
         focus->setFocusIterator(focusIterator);
         return makeSequenceMappingIterator<Item>(ConstPtr(this), focusIterator, focus);
      }
      return CommonValues::emptyIterator;
   }
}

Expression::Ptr ApplyTemplate::compress(const StaticContext::Ptr &context)
{
   /* If we have a mode, we will never need the default mode. */
   if (m_mode) {
      m_defaultMode.reset();
   }

   return TemplateInvoker::compress(context);
}

SequenceType::Ptr ApplyTemplate::staticType() const
{
   return CommonSequenceTypes::ZeroOrMoreItems;
}

ExpressionVisitorResult::Ptr ApplyTemplate::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::Properties ApplyTemplate::properties() const
{
   return RequiresFocus | DisableElimination;
}

bool ApplyTemplate::configureRecursion(const CallTargetDescription::Ptr &sign)
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   (void) sign;

   return false;
}

Expression::Ptr ApplyTemplate::body() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return Expression::Ptr();
}

CallTargetDescription::Ptr ApplyTemplate::callTargetDescription() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return CallTargetDescription::Ptr();
}
