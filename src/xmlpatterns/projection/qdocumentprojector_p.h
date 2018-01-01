/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDocumentProjector_P_H
#define QDocumentProjector_P_H

#include <qprojectedexpression_p.h>
#include <qabstractxmlreceiver.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class DocumentProjector : public QAbstractXmlReceiver
{
 public:
   DocumentProjector(const ProjectedExpression::Vector &paths, QAbstractXmlReceiver *const receiver);

   virtual void namespaceBinding(const QXmlName nb);

   virtual void characters(const QString &value);
   void comment(const QString &value) override;

   virtual void startElement(const QXmlName name);
   void endElement() override;

   virtual void attribute(const QXmlName name, const QString &value);

   virtual void processingInstruction(const QXmlName name, const QString &value);

   void item(const Item &item) override;

   void startDocument() override;
   void endDocument() override;

   ProjectedExpression::Vector m_paths;
   const int m_pathCount;

   ProjectedExpression::Action m_action;
   int m_nodesInProcess;

   QAbstractXmlReceiver *const m_receiver;
};
}

QT_END_NAMESPACE

#endif
