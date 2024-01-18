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

#ifndef QContextNodeChecker_P_H
#define QContextNodeChecker_P_H

#include <qfunctioncall_p.h>

namespace QPatternist {

class ContextNodeChecker : public FunctionCall
{
 protected:
   /**
    * @short Checks that the root node of @p node is a document node, and
    * otherwise issues an error.
    */
   void checkTargetNode(const QXmlNodeModelIndex &node,
                        const DynamicContext::Ptr &context,
                        const ReportContext::ErrorCode) const;
};

}

#endif
