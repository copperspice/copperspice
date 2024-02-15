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

#ifndef QCRASHHANDLER_P_H
#define QCRASHHANDLER_P_H

#include <qglobal.h>

#ifndef QT_NO_CRASHHANDLER

using FP_Void = void (*)();

class Q_CORE_EXPORT QSegfaultHandler
{
   friend void qt_signal_handler(int);
   static FP_Void callback;

 public:
   static void initialize(char **, int);

   static void installCrashHandler(FP_Void h) {
      callback = h;
   }

   static FP_Void crashHandler() {
      return callback;
   }
};

#endif
#endif // QCRASHHANDLER_P_H
