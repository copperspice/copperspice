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

#ifndef QValidate_P_H
#define QValidate_P_H

#include <qexpression_p.h>

namespace QPatternist {

class Validate
{
 public:

   /**
    * Represents the validation mode.
    */
   enum Mode {
      Lax = 1,
      Strict
   };

   /**
    * Creates the necessary Expression instances
    * that validates the operand node @p operandNode in mode @p validationMode,
    * and returns it.
    */
   static Expression::Ptr create(const Expression::Ptr &operandNode,
                                 const Mode validationMode,
                                 const StaticContext::Ptr &context);
 private:
   Validate();

   Validate(const Validate &) = delete;
   Validate &operator=(const Validate &) = delete;
};

}

#endif
