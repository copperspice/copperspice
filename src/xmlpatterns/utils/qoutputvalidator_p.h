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

#ifndef QOutputValidator_P_H
#define QOutputValidator_P_H

#include <qabstractxmlreceiver.h>
#include <qset.h>

#include <qdynamiccontext_p.h>
#include <qsourcelocationreflection_p.h>

namespace QPatternist {

class OutputValidator : public QAbstractXmlReceiver, public DelegatingSourceLocationReflection
{
 public:
   OutputValidator(QAbstractXmlReceiver *const receiver, const DynamicContext::Ptr &context,
                  const SourceLocationReflection *const r, const bool isXSLT);

   void namespaceBinding(const QXmlName &nb) override;

   void characters(QStringView value) override;
   void comment(const QString &value) override;

   void startElement(const QXmlName &name) override;
   void endElement() override;

   void attribute(const QXmlName &name, QStringView value) override;
   void processingInstruction(const QXmlName &name, const QString &value) override;
   void item(const Item &item) override;

   void startDocument() override;
   void endDocument() override;
   void atomicValue(const QVariant &value) override;
   void endOfSequence() override;
   void startOfSequence() override;

 private:
   bool m_hasReceivedChildren;
   QAbstractXmlReceiver *const m_receiver;
   const DynamicContext::Ptr m_context;

   /**
    * Keeps the current received attributes, in order to check uniqueness.
    */
   QSet<QXmlName> m_attributes;
   const bool m_isXSLT;
};
}

#endif
