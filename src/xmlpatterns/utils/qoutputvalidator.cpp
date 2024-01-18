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

#include <qoutputvalidator_p.h>

#include <qpatternistlocale_p.h>

using namespace QPatternist;

OutputValidator::OutputValidator(QAbstractXmlReceiver *const receiver,
                                 const DynamicContext::Ptr &context,
                                 const SourceLocationReflection *const r,
                                 const bool isXSLT) : DelegatingSourceLocationReflection(r)
   , m_hasReceivedChildren(false)
   , m_receiver(receiver)
   , m_context(context)
   , m_isXSLT(isXSLT)
{
   Q_ASSERT(receiver);
   Q_ASSERT(context);
}

void OutputValidator::namespaceBinding(const QXmlName &nb)
{
   m_receiver->namespaceBinding(nb);
}

void OutputValidator::startElement(const QXmlName &name)
{
   m_hasReceivedChildren = false;
   m_receiver->startElement(name);
   m_attributes.clear();
}

void OutputValidator::endElement()
{
   m_hasReceivedChildren = true;
   m_receiver->endElement();
}

void OutputValidator::attribute(const QXmlName &name, QStringView value)
{
   if (m_hasReceivedChildren) {
      m_context->error(QtXmlPatterns::tr("It's not possible to add attributes after any other kind of node."),
                  m_isXSLT ? ReportContext::XTDE0410 : ReportContext::XQTY0024, this);

   } else {
      if (!m_isXSLT && m_attributes.contains(name)) {
         m_context->error(QtXmlPatterns::tr("An attribute by name %1 has already been created.").formatArg(formatKeyword(
                  m_context->namePool(), name)), ReportContext::XQDY0025, this);
      } else {
         m_attributes.insert(name);
         m_receiver->attribute(name, value);
      }
   }
}

void OutputValidator::comment(const QString &value)
{
   m_hasReceivedChildren = true;
   m_receiver->comment(value);
}

void OutputValidator::characters(QStringView value)
{
   m_hasReceivedChildren = true;
   m_receiver->characters(value);
}

void OutputValidator::processingInstruction(const QXmlName &name,
      const QString &value)
{
   m_hasReceivedChildren = true;
   m_receiver->processingInstruction(name, value);
}

void OutputValidator::item(const Item &outputItem)
{
   /* We can't send outputItem directly to m_receiver since its item() function
    * won't dispatch to this OutputValidator, but to itself. We're not sub-classing here,
    * we're delegating. */

   if (outputItem.isNode()) {
      sendAsNode(outputItem);
   } else {
      m_hasReceivedChildren = true;
      m_receiver->item(outputItem);
   }
}

void OutputValidator::startDocument()
{
   m_receiver->startDocument();
}

void OutputValidator::endDocument()
{
   m_receiver->endDocument();
}

void OutputValidator::atomicValue(const QVariant &value)
{
   (void) value;

   // TODO
}

void OutputValidator::endOfSequence()
{
}

void OutputValidator::startOfSequence()
{
}
