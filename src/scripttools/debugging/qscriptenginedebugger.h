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

#ifndef QSCRIPTENGINEDEBUGGER_H
#define QSCRIPTENGINEDEBUGGER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QAction;
class QScriptEngine;
class QWidget;

#ifndef QT_NO_MAINWINDOW
class QMainWindow;
#endif

class QMenu;
class QToolBar;

class QScriptEngineDebuggerPrivate;

class Q_SCRIPTTOOLS_EXPORT QScriptEngineDebugger : public QObject
{
   SCRIPT_T_CS_OBJECT(QScriptEngineDebugger)

 public:
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

   enum DebuggerState {
      RunningState,
      SuspendedState
   };

   QScriptEngineDebugger(QObject *parent = nullptr);
   ~QScriptEngineDebugger();

   void attachTo(QScriptEngine *engine);
   void detach();

   bool autoShowStandardWindow() const;
   void setAutoShowStandardWindow(bool autoShow);

#ifndef QT_NO_MAINWINDOW
   QMainWindow *standardWindow() const;
#endif
   QToolBar *createStandardToolBar(QWidget *parent = nullptr);
   QMenu *createStandardMenu(QWidget *parent = nullptr);

   QWidget *widget(DebuggerWidget widget) const;
   QAction *action(DebuggerAction action) const;

   DebuggerState state() const;

   CS_SIGNAL_1(Public, void evaluationSuspended())
   CS_SIGNAL_2(evaluationSuspended)

   CS_SIGNAL_1(Public, void evaluationResumed())
   CS_SIGNAL_2(evaluationResumed)

 private:
   Q_DECLARE_PRIVATE(QScriptEngineDebugger)
   Q_DISABLE_COPY(QScriptEngineDebugger)

   CS_SLOT_1(Private, void _q_showStandardWindow())
   CS_SLOT_2(_q_showStandardWindow)     
};

QT_END_NAMESPACE

#endif
