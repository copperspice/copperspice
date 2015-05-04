/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QCallTargetDescription_P_H
#define QCallTargetDescription_P_H

template<typename Key, typename Value> class QHash;
template<typename T> class QList;

#include <QSharedData>
#include <qexpression_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class CallSite;


class CallTargetDescription : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<CallTargetDescription> Ptr;
   typedef QList<Ptr> List;

   CallTargetDescription(const QXmlName &name);

   /**
    * The function's name. For example, the name of the signature
    * <tt>fn:string() as xs:string</tt> is <tt>fn:string</tt>.
    */
   QXmlName name() const;

   /**
    * Flags callsites to be aware of their recursion by calling
    * UserFunctionCallsite::configureRecursion(), if that is the case.
    *
    * @note We pass @p expr by value here intentionally.
    */
   static void checkCallsiteCircularity(CallTargetDescription::List &signList,
                                        const Expression::Ptr expr);
 private:
   /**
    * Helper function for checkCallsiteCircularity(). If C++ allowed it,
    * it would have been local to it.
    */
   static void checkArgumentsCircularity(CallTargetDescription::List &signList,
                                         const Expression::Ptr callsite);

   Q_DISABLE_COPY(CallTargetDescription)
   const QXmlName m_name;
};
}

QT_END_NAMESPACE

#endif

