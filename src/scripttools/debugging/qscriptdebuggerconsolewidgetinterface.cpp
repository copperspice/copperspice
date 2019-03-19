/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qscriptdebuggerconsolewidgetinterface_p.h"
#include "qscriptdebuggerconsolewidgetinterface_p_p.h"

QT_BEGIN_NAMESPACE

QScriptDebuggerConsoleWidgetInterfacePrivate::QScriptDebuggerConsoleWidgetInterfacePrivate()
{
   historian = 0;
   completionProvider = 0;
}

QScriptDebuggerConsoleWidgetInterfacePrivate::~QScriptDebuggerConsoleWidgetInterfacePrivate()
{
}

QScriptDebuggerConsoleWidgetInterface::~QScriptDebuggerConsoleWidgetInterface()
{
}

QScriptDebuggerConsoleWidgetInterface::QScriptDebuggerConsoleWidgetInterface(
   QScriptDebuggerConsoleWidgetInterfacePrivate &dd,
   QWidget *parent, Qt::WindowFlags flags)
   : QWidget(dd, parent, flags)
{
}

QScriptDebuggerConsoleHistorianInterface *QScriptDebuggerConsoleWidgetInterface::commandHistorian() const
{
   Q_D(const QScriptDebuggerConsoleWidgetInterface);
   return d->historian;
}

void QScriptDebuggerConsoleWidgetInterface::setCommandHistorian(
   QScriptDebuggerConsoleHistorianInterface *historian)
{
   Q_D(QScriptDebuggerConsoleWidgetInterface);
   d->historian = historian;
}

QScriptCompletionProviderInterface *QScriptDebuggerConsoleWidgetInterface::completionProvider() const
{
   Q_D(const QScriptDebuggerConsoleWidgetInterface);
   return d->completionProvider;
}

void QScriptDebuggerConsoleWidgetInterface::setCompletionProvider(
   QScriptCompletionProviderInterface *completionProvider)
{
   Q_D(QScriptDebuggerConsoleWidgetInterface);
   d->completionProvider = completionProvider;
}

QT_END_NAMESPACE
