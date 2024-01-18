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

#ifndef QSYSTEMERROR_P_H
#define QSYSTEMERROR_P_H

#include <qstring8.h>

class Q_CORE_EXPORT QSystemError
{
 public:
   enum ErrorScope {
      NoError,
      StandardLibraryError,
      NativeError
   };

   inline QSystemError(int error, ErrorScope scope);
   inline QSystemError();

   QString toString();
   inline ErrorScope scope();
   inline int error();

   // data members
   int errorCode;
   ErrorScope errorScope;
};

QSystemError::QSystemError(int error, QSystemError::ErrorScope scope)
   : errorCode(error), errorScope(scope)
{
}

QSystemError::QSystemError()
   : errorCode(0), errorScope(NoError)
{
}

QSystemError::ErrorScope QSystemError::scope()
{
   return errorScope;
}

int QSystemError::error()
{
   return errorCode;
}

#endif