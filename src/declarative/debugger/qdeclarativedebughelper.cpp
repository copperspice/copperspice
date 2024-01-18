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

#include <QtScript/QScriptEngine>
#include <qdeclarativedebughelper_p.h>
#include <QtCore/QAbstractAnimation>
#include <QtScript/QScriptEngine>
#include <qdeclarativeengine_p.h>
#include <qabstractanimation_p.h>
#include <qdeclarativeengine_p.h>

QT_BEGIN_NAMESPACE

QScriptEngine *QDeclarativeDebugHelper::getScriptEngine(QDeclarativeEngine *engine)
{
   return QDeclarativeEnginePrivate::getScriptEngine(engine);
}

void QDeclarativeDebugHelper::setAnimationSlowDownFactor(qreal factor)
{
   QUnifiedTimer *timer = QUnifiedTimer::instance();
   timer->setSlowModeEnabled(factor != 1.0);
   timer->setSlowdownFactor(factor);
}

void QDeclarativeDebugHelper::enableDebugging()
{
#ifndef QDECLARATIVE_NO_DEBUG_PROTOCOL
   if (!QDeclarativeEnginePrivate::qml_debugging_enabled) {
      qWarning("Qml debugging is enabled. Only use this in a safe environment!");
   }
   QDeclarativeEnginePrivate::qml_debugging_enabled = true;
#endif
}

QT_END_NAMESPACE
