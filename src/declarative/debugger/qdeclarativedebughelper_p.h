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

#ifndef QDECLARATIVEDEBUGHELPER_P_H
#define QDECLARATIVEDEBUGHELPER_P_H

#include <QtCore/qglobal.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QScriptEngine;
class QDeclarativeEngine;

// Helper methods to access private API through a stable interface
// This is used in the qmljsdebugger library of QtCreator.
class Q_DECLARATIVE_EXPORT QDeclarativeDebugHelper
{
 public:
   static QScriptEngine *getScriptEngine(QDeclarativeEngine *engine);
   static void setAnimationSlowDownFactor(qreal factor);

   // Enables remote debugging functionality
   // Only use this for debugging in a safe environment!
   static void enableDebugging();
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEBUGHELPER_P_H
