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

#ifndef QTimezoneFNs_P_H
#define QTimezoneFNs_P_H

#include <qatomiccomparator_p.h>
#include <qfunctioncall_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class AdjustTimezone : public FunctionCall
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

 protected:
   virtual Item createValue(const QDateTime &dt) const = 0;
};

class AdjustDateTimeToTimezoneFN : public AdjustTimezone
{
 protected:
   Item createValue(const QDateTime &dt) const override;
};

class AdjustDateToTimezoneFN : public AdjustTimezone
{
 protected:
   Item createValue(const QDateTime &dt) const override;
};

class AdjustTimeToTimezoneFN : public AdjustTimezone
{
 protected:
   Item createValue(const QDateTime &dt) const override;
};
}

QT_END_NAMESPACE

#endif
