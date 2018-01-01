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

#ifndef QTemplateInvoker_P_H
#define QTemplateInvoker_P_H

#include <qcallsite_p.h>
#include <qwithparam_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class TemplateInvoker : public CallSite
{
 public:
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   inline const WithParam::Hash &withParams() const;
   WithParam::Hash m_withParams;

   SequenceType::List expectedOperandTypes() const override;

 protected:
   TemplateInvoker(const WithParam::Hash &withParams, const QXmlName &name = QXmlName());

 private:
   Q_DISABLE_COPY(TemplateInvoker)
};

const WithParam::Hash &TemplateInvoker::withParams() const
{
   return m_withParams;
}

}

QT_END_NAMESPACE

#endif

