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

#ifndef QSCRIPTDEBUGGER_P_H
#define QSCRIPTDEBUGGER_P_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerFrontend;
class QScriptDebuggerConsoleWidgetInterface;
class QScriptDebuggerScriptsWidgetInterface;
class QScriptDebuggerCodeWidgetInterface;
class QScriptDebuggerCodeFinderWidgetInterface;
class QScriptBreakpointsWidgetInterface;
class QScriptDebuggerStackWidgetInterface;
class QScriptDebuggerLocalsWidgetInterface;
class QScriptDebugOutputWidgetInterface;
class QScriptErrorLogWidgetInterface;
class QScriptDebuggerWidgetFactoryInterface;
class QAction;
class QEvent;
class QMenu;

#ifndef QT_NO_TOOLBAR
class QToolBar;
#endif

class QScriptDebuggerPrivate;

class QScriptDebugger : public QObject
{
   SCRIPT_T_CS_OBJECT(QScriptDebugger)

 public:
   // mirrors QScriptEngineDebugger::DebuggerWidget
   enum DebuggerWidget {
      ConsoleWidget,
      StackWidget,
      ScriptsWidget,
      LocalsWidget,
      CodeWidget,
      CodeFinderWidget,
      BreakpointsWidget,
      DebugOutputWidget,
      ErrorLogWidget
   };
   // mirrors QScriptEngineDebugger::DebuggerAction
   enum DebuggerAction {
      InterruptAction,
      ContinueAction,
      StepIntoAction,
      StepOverAction,
      StepOutAction,
      RunToCursorAction,
      RunToNewScriptAction,
      ToggleBreakpointAction,
      ClearDebugOutputAction,
      ClearErrorLogAction,
      ClearConsoleAction,
      FindInScriptAction,
      FindNextInScriptAction,
      FindPreviousInScriptAction,
      GoToLineAction
   };

   QScriptDebugger(QObject *parent = nullptr);
   ~QScriptDebugger();

   QScriptDebuggerFrontend *frontend() const;
   void setFrontend(QScriptDebuggerFrontend *frontend);

   QWidget *widget(DebuggerWidget widget);
   QAction *action(DebuggerAction action, QObject *parent);

   QScriptDebuggerConsoleWidgetInterface *consoleWidget() const;
   void setConsoleWidget(QScriptDebuggerConsoleWidgetInterface *consoleWidget);

   QScriptDebuggerScriptsWidgetInterface *scriptsWidget() const;
   void setScriptsWidget(QScriptDebuggerScriptsWidgetInterface *scriptsWidget);

   QScriptDebuggerCodeWidgetInterface *codeWidget() const;
   void setCodeWidget(QScriptDebuggerCodeWidgetInterface *codeWidget);

   QScriptDebuggerCodeFinderWidgetInterface *codeFinderWidget() const;
   void setCodeFinderWidget(QScriptDebuggerCodeFinderWidgetInterface *codeFinderWidget);

   QScriptDebuggerStackWidgetInterface *stackWidget() const;
   void setStackWidget(QScriptDebuggerStackWidgetInterface *stackWidget);

   QScriptDebuggerLocalsWidgetInterface *localsWidget() const;
   void setLocalsWidget(QScriptDebuggerLocalsWidgetInterface *localsWidget);

   QScriptBreakpointsWidgetInterface *breakpointsWidget() const;
   void setBreakpointsWidget(QScriptBreakpointsWidgetInterface *breakpointsWidget);

   QScriptDebugOutputWidgetInterface *debugOutputWidget() const;
   void setDebugOutputWidget(QScriptDebugOutputWidgetInterface *debugOutputWidget);

   QScriptErrorLogWidgetInterface *errorLogWidget() const;
   void setErrorLogWidget(QScriptErrorLogWidgetInterface *errorLogWidget);

   QScriptDebuggerWidgetFactoryInterface *widgetFactory() const;
   void setWidgetFactory(QScriptDebuggerWidgetFactoryInterface *factory);

   QAction *interruptAction(QObject *parent) const;
   QAction *continueAction(QObject *parent) const;
   QAction *stepIntoAction(QObject *parent) const;
   QAction *stepOverAction(QObject *parent) const;
   QAction *stepOutAction(QObject *parent) const;
   QAction *runToCursorAction(QObject *parent) const;
   QAction *runToNewScriptAction(QObject *parent) const;

   QAction *toggleBreakpointAction(QObject *parent) const;

   QAction *findInScriptAction(QObject *parent) const;
   QAction *findNextInScriptAction(QObject *parent) const;
   QAction *findPreviousInScriptAction(QObject *parent) const;
   QAction *goToLineAction(QObject *parent) const;

   QAction *clearDebugOutputAction(QObject *parent) const;
   QAction *clearConsoleAction(QObject *parent) const;
   QAction *clearErrorLogAction(QObject *parent) const;

   QMenu *createStandardMenu(QWidget *widgetParent, QObject *actionParent);

#ifndef QT_NO_TOOLBAR
   QToolBar *createStandardToolBar(QWidget *widgetParent, QObject *actionParent);
#endif

   bool eventFilter(QObject *, QEvent *e);
   bool isInteractive() const;

   CS_SIGNAL_1(Public, void stopped())
   CS_SIGNAL_2(stopped)
   CS_SIGNAL_1(Public, void started())
   CS_SIGNAL_2(started)

 protected:
   void timerEvent(QTimerEvent *e);
 
   QScriptDebugger(QScriptDebuggerPrivate &dd, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QScriptDebugger)
   Q_DISABLE_COPY(QScriptDebugger)


   CS_SLOT_1(Private, void _q_onLineEntered(const QString &un_named_arg1))
   CS_SLOT_2(_q_onLineEntered)

   CS_SLOT_1(Private, void _q_onCurrentFrameChanged(int un_named_arg1))
   CS_SLOT_2(_q_onCurrentFrameChanged)

   CS_SLOT_1(Private, void _q_onCurrentScriptChanged(qint64 un_named_arg1))
   CS_SLOT_2(_q_onCurrentScriptChanged)
 
   CS_SLOT_1(Private, void _q_onScriptLocationSelected(int un_named_arg1))
   CS_SLOT_2(_q_onScriptLocationSelected)
  
   CS_SLOT_1(Private, void _q_interrupt())
   CS_SLOT_2(_q_interrupt)

   CS_SLOT_1(Private, void _q_continue())
   CS_SLOT_2(_q_continue)
  
   CS_SLOT_1(Private, void _q_stepInto())
   CS_SLOT_2(_q_stepInto)
   
   CS_SLOT_1(Private, void _q_stepOver())
   CS_SLOT_2(_q_stepOver)
  
   CS_SLOT_1(Private, void _q_stepOut())
   CS_SLOT_2(_q_stepOut)
   
   CS_SLOT_1(Private, void _q_runToCursor())
   CS_SLOT_2(_q_runToCursor)
 
   CS_SLOT_1(Private, void _q_runToNewScript())
   CS_SLOT_2(_q_runToNewScript)
  
   CS_SLOT_1(Private, void _q_toggleBreakpoint())
   CS_SLOT_2(_q_toggleBreakpoint)    

   CS_SLOT_1(Private, void _q_clearDebugOutput())
   CS_SLOT_2(_q_clearDebugOutput)

   CS_SLOT_1(Private, void _q_clearErrorLog())
   CS_SLOT_2(_q_clearErrorLog)

   CS_SLOT_1(Private, void _q_clearConsole())
   CS_SLOT_2(_q_clearConsole)

   CS_SLOT_1(Private, void _q_findInScript())
   CS_SLOT_2(_q_findInScript)

   CS_SLOT_1(Private, void _q_findNextInScript())
   CS_SLOT_2(_q_findNextInScript)

   CS_SLOT_1(Private, void _q_findPreviousInScript())
   CS_SLOT_2(_q_findPreviousInScript)

   CS_SLOT_1(Private, void _q_onFindCodeRequest(const QString &un_named_arg1, int un_named_arg2))
   CS_SLOT_2(_q_onFindCodeRequest)

   CS_SLOT_1(Private, void _q_goToLine())
   CS_SLOT_2(_q_goToLine)

   
};

QT_END_NAMESPACE

#endif
