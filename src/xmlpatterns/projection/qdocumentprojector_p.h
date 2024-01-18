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

#ifndef QDocumentProjector_P_H
#define QDocumentProjector_P_H

#include <qabstractxmlreceiver.h>

#include <qprojectedexpression_p.h>

namespace QPatternist {

class DocumentProjector : public QAbstractXmlReceiver
{
 public:
   DocumentProjector(const ProjectedExpression::Vector &paths, QAbstractXmlReceiver *const receiver);

   void startElement(const QXmlName &name) override;
   void endElement() override;

   void attribute(const QXmlName &name, QStringView value) override;

   void comment(const QString &value) override;
   void characters(QStringView value) override;

   void startDocument() override;
   void endDocument() override;

   void processingInstruction(const QXmlName &name, const QString &value) override;

   void namespaceBinding(const QXmlName &nb) override;

   void item(const Item &item) override;

   ProjectedExpression::Vector m_paths;
   const int m_pathCount;

   ProjectedExpression::Action m_action;
   int m_nodesInProcess;

   QAbstractXmlReceiver *const m_receiver;
};

}   // end namespace

#endif
