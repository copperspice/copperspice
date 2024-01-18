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

#ifndef QCallSite_P_H
#define QCallSite_P_H

#include <qunlimitedcontainer_p.h>
#include <qcalltargetdescription_p.h>
#include <qxmlname.h>

namespace QPatternist {

class CallSite : public UnlimitedContainer
{
 public:
   QXmlName name() const;
   bool isRecursive() const;
   void setIsRecursive(const bool value);

   /**
    * Called in the earliest stages of the compilation process. @p sign can
    * be any function signature for a user declared function. If @p sign
    * matches this UserFunctionCallsite, it means the UserFunction represented
    * by @p sign is recursive and that this UserFunctionCallsite should take
    * appropriate measures.
    *
    * @returns @c true if is recursive, otherwise @c false
    */
   virtual bool configureRecursion(const CallTargetDescription::Ptr &sign) = 0;

   /**
    * @short Returns the body of the function/template/component that is
    * being called.
    */
   virtual Expression::Ptr body() const = 0;

   virtual CallTargetDescription::Ptr callTargetDescription() const = 0;

 protected:
   CallSite(const QXmlName &name = QXmlName());

 private:
   CallSite(const CallSite &) = delete;
   CallSite &operator=(const CallSite &) = delete;

   bool            m_isRecursive;
   const QXmlName  m_name;
};

}

#endif
