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

#include "qvalidationerror_p.h"

using namespace QPatternist;

ValidationError::ValidationError(const QString &msg,
                                 const ReportContext::ErrorCode code) : m_message(msg),
   m_code(code)
{
}

AtomicValue::Ptr ValidationError::createError(const QString &description,
      const ReportContext::ErrorCode code)
{
   return ValidationError::Ptr(new ValidationError(description, code));
}

bool ValidationError::hasError() const
{
   return true;
}

QString ValidationError::stringValue() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return QString();
}

QString ValidationError::message() const
{
   return m_message;
}

ItemType::Ptr ValidationError::type() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return ItemType::Ptr();
}

ReportContext::ErrorCode ValidationError::errorCode() const
{
   return m_code;
}
