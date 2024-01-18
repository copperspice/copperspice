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

#include "qcontextnodechecker_p.h"

using namespace QPatternist;

void ContextNodeChecker::checkTargetNode(const QXmlNodeModelIndex &node, const DynamicContext::Ptr &context,
                  const ReportContext::ErrorCode code) const
{
   if (node.root().kind() != QXmlNodeModelIndex::Document) {
      context->error(QtXmlPatterns::tr("The root node of the second argument to function %1 must be a document node. %2 is not a document node.")
                  .formatArgs(formatFunction(context->namePool(), signature()), formatData(node)), code, this);
   }
}
