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

#include "qcallsite_p.h"

#include "qcalltargetdescription_p.h"

using namespace QPatternist;

CallTargetDescription::CallTargetDescription(const QXmlName &name) : m_name(name)
{
   Q_ASSERT(!m_name.isNull());
}

QXmlName CallTargetDescription::name() const
{
   return m_name;
}

void CallTargetDescription::checkArgumentsCircularity(CallTargetDescription::List &signList,
      const Expression::Ptr callsite)
{
   /* Check the arguments. */
   const Expression::List ops(callsite->operands());

   const Expression::List::const_iterator end(ops.constEnd());
   Expression::List::const_iterator it(ops.constBegin());

   for (; it != end; ++it) {
      checkCallsiteCircularity(signList, *it);
   }
}

void CallTargetDescription::checkCallsiteCircularity(CallTargetDescription::List &signList,
      const Expression::Ptr expr)
{
   Q_ASSERT(expr);

   if (expr->is(Expression::IDUserFunctionCallsite)) {
      CallTargetDescription::List::const_iterator it(signList.constBegin());
      const CallTargetDescription::List::const_iterator end(signList.constEnd());
      CallSite *const callsite = static_cast<CallSite *>(expr.data());

      for (; it != end; ++it) {
         if (callsite->configureRecursion(*it)) {
            /* A callsite inside the function body to the function. This user function
             * is recursive if it's to the same function, in other words. Which it was
             * if configureRecursion() returned true. */

            /* Now we continue and check the arguments of the callsite. That is, the arguments.
             * This catches for instance local:foo(local:foo(3)). */
            checkArgumentsCircularity(signList, expr);
            return;
         }
      }
      /* Check the body of the function so this callsite isn't "indirectly" a
       * recursive call to the function we're checking. XQTS test case
       * default_namespace-011 is an example of this. */
      signList.append(callsite->callTargetDescription());
      checkCallsiteCircularity(signList, callsite->body());
   }

   checkArgumentsCircularity(signList, expr); /* We're done in this case. */
}
