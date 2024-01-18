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

#ifndef QSCRIPTDEBUGGERCODEWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERCODEWIDGETINTERFACE_P_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerScriptsModel;
class QScriptBreakpointsModel;
class QScriptToolTipProviderInterface;
class QScriptDebuggerCodeViewInterface;
class QScriptDebuggerCodeWidgetInterfacePrivate;

class QScriptDebuggerCodeWidgetInterface : public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerCodeWidgetInterface)

 public:
   ~QScriptDebuggerCodeWidgetInterface();

   virtual QScriptDebuggerScriptsModel *scriptsModel() const = 0;
   virtual void setScriptsModel(QScriptDebuggerScriptsModel *model) = 0;

   virtual QScriptBreakpointsModel *breakpointsModel() const = 0;
   virtual void setBreakpointsModel(QScriptBreakpointsModel *model) = 0;

   virtual void setToolTipProvider(QScriptToolTipProviderInterface *toolTipProvider) = 0;

   virtual qint64 currentScriptId() const = 0;
   virtual void setCurrentScript(qint64 scriptId) = 0;

   virtual void invalidateExecutionLineNumbers() = 0;

   virtual QScriptDebuggerCodeViewInterface *currentView() const = 0;

 protected:
   QScriptDebuggerCodeWidgetInterface(
      QScriptDebuggerCodeWidgetInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerCodeWidgetInterface)
   Q_DISABLE_COPY(QScriptDebuggerCodeWidgetInterface)
};

QT_END_NAMESPACE

#endif
