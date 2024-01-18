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

#include "qscriptdebuggerstandardwidgetfactory_p.h"
#include "qscriptdebuggerconsolewidget_p.h"
#include "qscriptdebuggerstackwidget_p.h"
#include "qscriptdebuggerscriptswidget_p.h"
#include "qscriptdebuggerlocalswidget_p.h"
#include "qscriptdebuggercodewidget_p.h"
#include "qscriptdebuggercodefinderwidget_p.h"
#include "qscriptbreakpointswidget_p.h"
#include "qscriptdebugoutputwidget_p.h"
#include "qscripterrorlogwidget_p.h"

QT_BEGIN_NAMESPACE

QScriptDebuggerStandardWidgetFactory::QScriptDebuggerStandardWidgetFactory(QObject *parent)
   : QObject(parent)
{
}

QScriptDebuggerStandardWidgetFactory::~QScriptDebuggerStandardWidgetFactory()
{
}

QScriptDebugOutputWidgetInterface *QScriptDebuggerStandardWidgetFactory::createDebugOutputWidget()
{
   return new QScriptDebugOutputWidget();
}

QScriptDebuggerConsoleWidgetInterface *QScriptDebuggerStandardWidgetFactory::createConsoleWidget()
{
   return new QScriptDebuggerConsoleWidget();
}

QScriptErrorLogWidgetInterface *QScriptDebuggerStandardWidgetFactory::createErrorLogWidget()
{
   return new QScriptErrorLogWidget();
}

QScriptDebuggerCodeFinderWidgetInterface *QScriptDebuggerStandardWidgetFactory::createCodeFinderWidget()
{
   return new QScriptDebuggerCodeFinderWidget();
}

QScriptDebuggerStackWidgetInterface *QScriptDebuggerStandardWidgetFactory::createStackWidget()
{
   return new QScriptDebuggerStackWidget();
}

QScriptDebuggerScriptsWidgetInterface *QScriptDebuggerStandardWidgetFactory::createScriptsWidget()
{
   return new QScriptDebuggerScriptsWidget();
}

QScriptDebuggerLocalsWidgetInterface *QScriptDebuggerStandardWidgetFactory::createLocalsWidget()
{
   return new QScriptDebuggerLocalsWidget();
}

QScriptDebuggerCodeWidgetInterface *QScriptDebuggerStandardWidgetFactory::createCodeWidget()
{
   return new QScriptDebuggerCodeWidget();
}

QScriptBreakpointsWidgetInterface *QScriptDebuggerStandardWidgetFactory::createBreakpointsWidget()
{
   return new QScriptBreakpointsWidget();
}

QT_END_NAMESPACE
