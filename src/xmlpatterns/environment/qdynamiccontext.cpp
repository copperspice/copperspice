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

#include "qfocus_p.h"
#include "qreceiverdynamiccontext_p.h"
#include "qstackcontextbase_p.h"

#include "qdynamiccontext_p.h"

using namespace QPatternist;

DynamicContext::Ptr DynamicContext::createFocus()
{
   return Ptr(new Focus(Ptr(this)));
}

DynamicContext::Ptr DynamicContext::createStack()
{
   return Ptr(new StackContext(Ptr(this)));
}

DynamicContext::Ptr DynamicContext::createReceiverContext(QAbstractXmlReceiver *const receiver)
{
   Q_ASSERT(receiver);
   return Ptr(new ReceiverDynamicContext(Ptr(this), receiver));
}
