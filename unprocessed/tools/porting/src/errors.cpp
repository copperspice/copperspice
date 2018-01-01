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

#include "errors.h"

QT_BEGIN_NAMESPACE

QT_STATIC_CONST_IMPL Error Errors::InternalError = Error( 1, -1, QLatin1String("Internal Error") );
QT_STATIC_CONST_IMPL Error Errors::SyntaxError = Error( 2, -1, QLatin1String("Syntax Error before '%1'") );
QT_STATIC_CONST_IMPL Error Errors::ParseError = Error( 3, -1, QLatin1String("Parse Error before '%1'") );

QT_END_NAMESPACE
