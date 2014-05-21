/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_OutputValidator_P_H
#define Patternist_OutputValidator_P_SH

#include <QSet>
#include "qdynamiccontext_p.h"
#include "qabstractxmlreceiver.h"
#include "qsourcelocationreflection_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
   
    class OutputValidator : public QAbstractXmlReceiver, public DelegatingSourceLocationReflection
    {
    public:
        OutputValidator(QAbstractXmlReceiver *const receiver,
                        const DynamicContext::Ptr &context,
                        const SourceLocationReflection *const r,
                        const bool isXSLT);

        virtual void namespaceBinding(const QXmlName &nb);

        virtual void characters(const QStringRef &value);
        virtual void comment(const QString &value);

        virtual void startElement(const QXmlName &name);

        virtual void endElement();

        virtual void attribute(const QXmlName &name, const QStringRef &value);

        virtual void processingInstruction(const QXmlName &name, const QString &value);

        virtual void item(const Item &item);

        virtual void startDocument();
        virtual void endDocument();
        virtual void atomicValue(const QVariant &value);
        virtual void endOfSequence();
        virtual void startOfSequence();

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

QT_END_NAMESPACE

#endif
