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

#include "qpatternistlocale_p.h"

#include "qdocumentcontentvalidator_p.h"

using namespace QPatternist;

DocumentContentValidator::
DocumentContentValidator(QAbstractXmlReceiver *const receiver,
                         const DynamicContext::Ptr &context,
                         const Expression::ConstPtr &expr) : m_receiver(receiver)
   , m_context(context)
   , m_expr(expr)
   , m_elementDepth(0)
{
   Q_ASSERT(receiver);
   Q_ASSERT(m_expr);
   Q_ASSERT(context);
}

void DocumentContentValidator::namespaceBinding(const QXmlName &nb)
{
   m_receiver->namespaceBinding(nb);
}

void DocumentContentValidator::startElement(const QXmlName &name)
{
   ++m_elementDepth;
   m_receiver->startElement(name);
}

void DocumentContentValidator::endElement()
{
   Q_ASSERT(m_elementDepth > 0);
   --m_elementDepth;
   m_receiver->endElement();
}

void DocumentContentValidator::attribute(const QXmlName &name, QStringView value)
{
   if (m_elementDepth == 0) {
      m_context->error(QtXmlPatterns::tr("An attribute node cannot be a "
                  "child of a document node. "
                  "Therefore, the attribute %1 "
                  "is out of place.")
                  .formatArgs(formatKeyword(m_context->namePool(), name)), ReportContext::XPTY0004, m_expr.data());

   } else {
      m_receiver->attribute(name, value);
   }
}

void DocumentContentValidator::comment(const QString &value)
{
   m_receiver->comment(value);
}

void DocumentContentValidator::characters(QStringView value)
{
   m_receiver->characters(value);
}

void DocumentContentValidator::processingInstruction(const QXmlName &name,
      const QString &value)
{
   m_receiver->processingInstruction(name, value);
}

void DocumentContentValidator::item(const Item &outputItem)
{
   /* We can't send outputItem directly to m_receiver since its item() function
    * won't dispatch to this DocumentContentValidator, but to itself. We're not sub-classing here,
    * we're delegating. */

   if (outputItem.isNode()) {
      sendAsNode(outputItem);
   } else {
      m_receiver->item(outputItem);
   }
}

void DocumentContentValidator::startDocument()
{
   m_receiver->startDocument();
}

void DocumentContentValidator::endDocument()
{
   m_receiver->endDocument();
}

void DocumentContentValidator::atomicValue(const QVariant &value)
{
   (void) value;
}

void DocumentContentValidator::startOfSequence()
{
}

void DocumentContentValidator::endOfSequence()
{
}

