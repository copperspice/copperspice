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

#include "qscriptenginedebugger.h"
#include "qscriptdebugger_p.h"
#include "qscriptenginedebuggerfrontend_p.h"
#include "qscriptdebuggerstandardwidgetfactory_p.h"

#include <QtCore/qsettings.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdockwidget.h>
#include <QtGui/qmainwindow.h>
#include <QtGui/qmenubar.h>
#include <QtGui/qboxlayout.h>

// this has to be outside the namespace
static void initScriptEngineDebuggerResources()
{
   Q_INIT_RESOURCE(scripttools_debugging);
}

QT_BEGIN_NAMESPACE

class QtScriptDebuggerResourceInitializer
{
 public:
   QtScriptDebuggerResourceInitializer() {
      // call outside-the-namespace function
      initScriptEngineDebuggerResources();
   }
};

/*!
  \since 4.5
  \class QScriptEngineDebugger

  \brief The QScriptEngineDebugger class provides a QScriptEngine debugger.

  \ingroup script


  The QScriptEngineDebugger class provides a debugger that can be
  embedded into Qt applications that use Qt Script. The debugger
  enables the application user to inspect the state of the script
  environment and control script execution.

  To attach the debugger to a script engine, call the attachTo()
  function.

  \snippet doc/src/snippets/code/src.scripttools.qscriptenginedebugger.cpp 0

  Once the debugger has been attached to a script engine, you can
  proceed to evaluate scripts as usual, e.g. by calling
  QScriptEngine::evaluate(). The debugger will be triggered when an
  uncaught exception occurs, or when a \c{debugger} statement is
  encountered in a script. It is also possible to interrupt script
  evaluation at an arbitrary time by triggering the InterruptAction.
  For instance, to start the debugger when script evaluation starts,
  you trigger the action before you begin to \l{QScriptEngine::}{evaluate()}
  the script.

  \snippet doc/src/snippets/scriptdebugger.cpp 2

  By default, the \l{standardWindow()}{standard debugger window} is shown when
  evaluation is suspended.  This can be changed by calling the
  setAutoShowStandardWindow() function.

  The debugger defines a set of \l{DebuggerAction}{actions} that are
  available, such as stopping execution or printing the contents of a
  variable. It also provides a set of widgets (components) that
  display the information available from the debugger and that trigger
  the actions on request. The actions available are identified by the
  DebuggerAction enum, and the widgets are identified by the
  DebuggerWidget enum.

  Access to the individual debugger widgets is provided by the
  widget() function. This makes it possible to arrange the widgets in
  a custom manner. Similarly, the action() function provides access
  to the various debugger actions.

  The createStandardToolBar() function creates a standard toolbar, and the
  createStandardMenu() function creates a standard menu; these functions can
  be useful if you are creating a custom debugger configuration.

  The evaluationSuspended() signal is emitted when the debugger has
  suspended script evaluation and entered interactive mode, i.e., the
  mode in which it accepts input from the user. The
  evaluationResumed() signal is emitted when script evaluation is
  resumed, i.e, when execution control is given back to the script
  engine. The state() function returns the debugger's current state.

  When calling QScriptEngine::evaluate() it is useful to pass a
  descriptive script name (file name) as second argument, as this is
  the name that will be displayed by the debugger in the
  ScriptsWidget; if a name is not passed, the script will be labelled
  "anonymous".

  When evaluation is suspended, the debugger will also suspend the
  event loop of the script. In the following snippet, the call to
  QScriptEngine::evaluate() causes the debugger to be triggered, and
  the function call does not return until the user has finished
  interacting with the debugger.

  \snippet doc/src/snippets/code/src.scripttools.qscriptenginedebugger.cpp 1

  When the Qt Script debugger is running, the C++ application itself
  is not "frozen". This means that it is possible that more scripts
  are evaluated, even though the debugger has suspended evaluation of
  the \bold{current} script evaluation. For example, a C++ timer might
  trigger that causes a script function to be called, or the user
  might click on a button in the main application user interface whose
  clicked() signal is connected to a script function. This kind of
  nested evaluation is permitted. The debugger will enter interactive
  mode for the new script if an exception is thrown or a breakpoint is
  reached. Note that it will not stop when encountering \c{debugger}
  statements. \omit The effects are similar to those achieved by
  typing a program into the debugger's console and evaluating
  it. \endomit

  Nested evaluation requires some thought when deciding
  how the debugger is presented to the user; for example, whether a
  modal dialog is suitable, or whether some parts of the main
  application user interface should be disabled while the debugger is
  running. \omit Seems unfinished somehow \endomit

  Debugging inside of a \l{QWidget::paintEvent()}{paintEvent}() is
  currently not supported. If you need to debug painting-related
  script code, that code should be evaluated outside of the C++
  paintEvent(), e.g. by rendering to an image, like the Context2D and
  Tetrix QtScript examples do. This will make the code safe for
  debugging.

  The debugger adds some special properties to the script engine:
  \c{__FILE__} holds the name of the script in which the current
  evaluation occurs, and \c{__LINE__} holds the current line
  number. These are useful when doing print()-style debugging (the
  messages appear in the debugger's debug output widget).

  The \l{Qt Script Debugger Manual} describes how to use the debugger.
  The \l{Context2D Example}{Context2D example} shows how to integrate
  the debugger in applications.

  \sa QScriptEngine, {Context2D Example}
*/

/*!
    \enum QScriptEngineDebugger::DebuggerWidget

    This enum decides the widget that the widget() function should
    retrieve. We treat these widgets in more detail in the \l{Qt
    Script Debugger Manual}.

    \value ConsoleWidget Provides a command-line interface to the debugger.
    \value StackWidget Shows a backtrace of the script's execution state.
    \value ScriptsWidget Displays a list of currently loaded scripts.
    \value LocalsWidget Shows the local variables of the current stack frame.
    \value CodeWidget Displays the code of the current script.
    \value CodeFinderWidget Provides a widget that can search for text in the script shown in the
           CodeWidget.
    \value BreakpointsWidget Shows breakpoints that have been set.
    \value DebugOutputWidget Contains output from the \c print() script function.
    \value ErrorLogWidget Shows error messages that have been generated.
*/

/*!
    \enum QScriptEngineDebugger::DebuggerAction

    This enum specifies the action that the action() function should
    retrieve.  The actions retrieved can be connected to any slot and
    connected to any widget. Please see the \l{Qt Script Debugger Manual}'s
    \l{Console Command Reference} for a detailed description of these actions.

    \value InterruptAction Suspends script execution as soon as the next script statement is reached.
    \value ContinueAction Gives the execution control back to the script engine.
    \value StepIntoAction Performs a step action.
    \value StepOverAction Performs a next action.
    \value StepOutAction Executes the script until the current function returns.
    \value RunToCursorAction Continues execution to the selected line (which contains the cursor) in the CodeWidget.
    \value RunToNewScriptAction Returns control to the script engine until a new script is executed.
    \value ToggleBreakpointAction Toggles a breakpoint at the selected line in the CodeWidget.
    \value ClearDebugOutputAction Clears the contents of the DebugOutputWidget.
    \value ClearErrorLogAction Clears the contents of the ErrorLogWidget.
    \value ClearConsoleAction Clears the contents of the ConsoleWidget.
    \value FindInScriptAction Displays the CodeFinderWidget.
    \value FindNextInScriptAction Finds next occurrence in the CodeWidget.
    \value FindPreviousInScriptAction Finds previous occurrence in the CodeWidget.
    \value GoToLineAction Shows the "Go to Line" dialog.
*/

/*!
    \enum QScriptEngineDebugger::DebuggerState
    \since 4.6

    This enum specifies the current state of the debugger.

    \value RunningState   The debugger is running.  (Script evaluation is allowed.)
    \value SuspendedState The debugger has suspended script evaluation.
*/

class QScriptEngineDebuggerPrivate
{
   Q_DECLARE_PUBLIC(QScriptEngineDebugger)

 public:
   QScriptEngineDebuggerPrivate();
   ~QScriptEngineDebuggerPrivate();

   // private slots
   void _q_showStandardWindow();

   void createDebugger();

   QScriptDebugger *debugger;
   QScriptEngineDebuggerFrontend *frontend;
#ifndef QT_NO_MAINWINDOW
   QMainWindow *standardWindow;
#endif
   bool autoShow;

   static QtScriptDebuggerResourceInitializer resourceInitializer;
};

namespace {

class WidgetClosedNotifier : public QObject
{
   SCRIPT_T_CS_OBJECT(WidgetClosedNotifier)

 public:
   WidgetClosedNotifier(QWidget *w, QObject *parent = nullptr)
      : QObject(parent), widget(w) {
      w->installEventFilter(this);
   }

   bool eventFilter(QObject *watched, QEvent *e) {
      if (watched != widget) {
         return false;
      }
      if (e->type() != QEvent::Close) {
         return false;
      }
      emit widgetClosed();
      return true;
   }

 Q_SIGNALS:
   void widgetClosed();

 private:
   QWidget *widget;
};

} // namespace

QtScriptDebuggerResourceInitializer QScriptEngineDebuggerPrivate::resourceInitializer;

QScriptEngineDebuggerPrivate::QScriptEngineDebuggerPrivate()
{
   debugger = 0;
   frontend = 0;
#ifndef QT_NO_MAINWINDOW
   standardWindow = 0;
#endif
   autoShow = true;
}

QScriptEngineDebuggerPrivate::~QScriptEngineDebuggerPrivate()
{
   delete debugger;
   delete frontend;
#ifndef QT_NO_MAINWINDOW
   if (standardWindow) {
      QSettings settings(QSettings::UserScope, QLatin1String("CopperSpice"));
      QByteArray geometry = standardWindow->saveGeometry();
      settings.setValue(QLatin1String("CS/scripttools/debugging/mainWindowGeometry"), geometry);
      QByteArray state = standardWindow->saveState();
      settings.setValue(QLatin1String("CS/scripttools/debugging/mainWindowState"), state);
      if (standardWindow->parent() == 0) {
         delete standardWindow;
      }
   }
#endif
}

#ifndef QT_NO_MAINWINDOW
void QScriptEngineDebuggerPrivate::_q_showStandardWindow()
{
   Q_Q(QScriptEngineDebugger);
   (void)q->standardWindow(); // ensure it's created
   standardWindow->show();
}
#endif

void QScriptEngineDebuggerPrivate::createDebugger()
{
   Q_Q(QScriptEngineDebugger);
   if (!debugger) {
      debugger = new QScriptDebugger();
      debugger->setWidgetFactory(new QScriptDebuggerStandardWidgetFactory(q));

      QObject::connect(debugger, SIGNAL(started()), q, SIGNAL(evaluationResumed()));
      QObject::connect(debugger, SIGNAL(stopped()), q, SIGNAL(evaluationSuspended()));

      if (autoShow) {
         QObject::connect(q, SIGNAL(evaluationSuspended()), q, SLOT(_q_showStandardWindow()));
      }
   }
}

/*!
  Constructs a new QScriptEngineDebugger object with the given \a
  parent.

  To attach a QScriptEngine to the debugger, use attachTo()
  function.

*/
QScriptEngineDebugger::QScriptEngineDebugger(QObject *parent)
   : QObject(*new QScriptEngineDebuggerPrivate, parent)
{
}

/*!
  Destroys this QScriptEngineDebugger.
*/
QScriptEngineDebugger::~QScriptEngineDebugger()
{
}

/*!
  Attaches to the given \a engine.

  The debugger will install a custom agent (using
  QScriptEngine::setAgent()) to monitor the engine. While the debugger
  is attached, you should not change the agent; however, if you do
  have to perform additional monitoring, you must set a proxy agent
  that forwards all events to the debugger's agent.

  \sa detach()
*/
void QScriptEngineDebugger::attachTo(QScriptEngine *engine)
{
   Q_D(QScriptEngineDebugger);
   if (!engine) {
      detach();
      return;
   }
   d->createDebugger();
   if (!d->frontend) {
      d->frontend = new QScriptEngineDebuggerFrontend();
   }
   d->frontend->attachTo(engine);
   d->debugger->setFrontend(d->frontend);
}

/*!
  Detaches from the current script engine, if any.

  \sa attachTo()
*/
void QScriptEngineDebugger::detach()
{
   Q_D(QScriptEngineDebugger);
   if (d->frontend) {
      d->frontend->detach();
   }
   if (d->debugger) {
      d->debugger->setFrontend(0);
   }
}

/*!
  \since 4.6

  Returns the current state of the debugger.

  \sa evaluationResumed()
  \sa evaluationSuspended()
*/
QScriptEngineDebugger::DebuggerState QScriptEngineDebugger::state() const
{
   Q_D(const QScriptEngineDebugger);
   return !d->debugger || !d->debugger->isInteractive() ? SuspendedState : RunningState;
}

/*!

    Returns a pointer to the instance of the specified standard \a
    widget. The widgets available are defined by the DebuggerWidget
    enum.

    A main window containing all widgets is returned by
    standardWindow(). If you do not want to use this window, you can
    fetch the individual widgets with this function. For instance, the
    code example below shows how to set up a layout containing a
    \l{QScriptEngineDebugger::CodeWidget}{code window} and a
    \l{QScriptEngineDebugger::StackWidget}{stack widget}.

    \snippet doc/src/snippets/scriptdebugger.cpp 0

    Note that you need to set setAutoShowStandardWindow() to false; if
    not, the standard window will be shown regardless.

    \sa action(), standardWindow(), setAutoShowStandardWindow()
*/
QWidget *QScriptEngineDebugger::widget(DebuggerWidget widget) const
{
   Q_D(const QScriptEngineDebugger);
   const_cast<QScriptEngineDebuggerPrivate *>(d)->createDebugger();
   return d->debugger->widget(static_cast<QScriptDebugger::DebuggerWidget>(static_cast<int>(widget)));
}

/*!
    Returns a pointer to the specified \a action. The actions available
    are given by the DebuggerAction enum.

    With this function, you can add the actions to your own widgets,
    toolbars, and menus. It is also convenient if you, for example,
    wish to spice things up with your own groovy icons. The code
    example below shows how to add actions to a QToolBar.

    \snippet doc/src/snippets/scriptdebugger.cpp 1

    Note that QScriptEngineDebugger has already added the actions to
    its \l{DebuggerWidget}{standard widgets} and \l{standardWindow()}{standard window}.

    \sa widget(), createStandardMenu(), createStandardToolBar(), standardWindow()
*/
QAction *QScriptEngineDebugger::action(DebuggerAction action) const
{
   Q_D(const QScriptEngineDebugger);
   QScriptEngineDebugger *that = const_cast<QScriptEngineDebugger *>(this);
   that->d_func()->createDebugger();
   return d->debugger->action(static_cast<QScriptDebugger::DebuggerAction>(static_cast<int>(action)), that);
}

/*!
  Returns whether the standard debugger window is automatically shown when
  evaluation is suspended.

  The default is true.
*/
bool QScriptEngineDebugger::autoShowStandardWindow() const
{
   Q_D(const QScriptEngineDebugger);
   return d->autoShow;
}

/*!
  Sets whether the standard debugger window is automatically shown when
  evaluation is suspended. If \a autoShow is true, the window will be
  automatically shown, otherwise it will not.
*/
void QScriptEngineDebugger::setAutoShowStandardWindow(bool autoShow)
{
   Q_D(QScriptEngineDebugger);
   if (autoShow == d->autoShow) {
      return;
   }

   if (autoShow) {
      QObject::connect(this, SIGNAL(evaluationSuspended()), this, SLOT(_q_showStandardWindow()));

   } else {
      QObject::disconnect(this, SIGNAL(evaluationSuspended()), this, SLOT(_q_showStandardWindow()));
   }

   d->autoShow = autoShow;
}

/*!
  Returns a main window with a standard configuration of the debugger's
  components.

  \sa createStandardMenu(), createStandardToolBar()
*/
#ifndef QT_NO_MAINWINDOW
QMainWindow *QScriptEngineDebugger::standardWindow() const
{
   Q_D(const QScriptEngineDebugger);
   if (d->standardWindow) {
      return d->standardWindow;
   }
   if (!QApplication::instance()) {
      return 0;
   }
   QScriptEngineDebugger *that = const_cast<QScriptEngineDebugger *>(this);

   QMainWindow *win = new QMainWindow();
   QDockWidget *scriptsDock = new QDockWidget(win);
   scriptsDock->setObjectName(QLatin1String("qtscriptdebugger_scriptsDockWidget"));
   scriptsDock->setWindowTitle(tr("Loaded Scripts"));
   scriptsDock->setWidget(widget(ScriptsWidget));
   win->addDockWidget(Qt::LeftDockWidgetArea, scriptsDock);

   QDockWidget *breakpointsDock = new QDockWidget(win);
   breakpointsDock->setObjectName(QLatin1String("qtscriptdebugger_breakpointsDockWidget"));
   breakpointsDock->setWindowTitle(tr("Breakpoints"));
   breakpointsDock->setWidget(widget(BreakpointsWidget));
   win->addDockWidget(Qt::LeftDockWidgetArea, breakpointsDock);

   QDockWidget *stackDock = new QDockWidget(win);
   stackDock->setObjectName(QLatin1String("qtscriptdebugger_stackDockWidget"));
   stackDock->setWindowTitle(tr("Stack"));
   stackDock->setWidget(widget(StackWidget));
   win->addDockWidget(Qt::RightDockWidgetArea, stackDock);

   QDockWidget *localsDock = new QDockWidget(win);
   localsDock->setObjectName(QLatin1String("qtscriptdebugger_localsDockWidget"));
   localsDock->setWindowTitle(tr("Locals"));
   localsDock->setWidget(widget(LocalsWidget));
   win->addDockWidget(Qt::RightDockWidgetArea, localsDock);

   QDockWidget *consoleDock = new QDockWidget(win);
   consoleDock->setObjectName(QLatin1String("qtscriptdebugger_consoleDockWidget"));
   consoleDock->setWindowTitle(tr("Console"));
   consoleDock->setWidget(widget(ConsoleWidget));
   win->addDockWidget(Qt::BottomDockWidgetArea, consoleDock);

   QDockWidget *debugOutputDock = new QDockWidget(win);
   debugOutputDock->setObjectName(QLatin1String("qtscriptdebugger_debugOutputDockWidget"));
   debugOutputDock->setWindowTitle(tr("Debug Output"));
   debugOutputDock->setWidget(widget(DebugOutputWidget));
   win->addDockWidget(Qt::BottomDockWidgetArea, debugOutputDock);

   QDockWidget *errorLogDock = new QDockWidget(win);
   errorLogDock->setObjectName(QLatin1String("qtscriptdebugger_errorLogDockWidget"));
   errorLogDock->setWindowTitle(tr("Error Log"));
   errorLogDock->setWidget(widget(ErrorLogWidget));
   win->addDockWidget(Qt::BottomDockWidgetArea, errorLogDock);

   win->tabifyDockWidget(errorLogDock, debugOutputDock);
   win->tabifyDockWidget(debugOutputDock, consoleDock);

   win->addToolBar(Qt::TopToolBarArea, that->createStandardToolBar());

#ifndef QT_NO_MENUBAR
   win->menuBar()->addMenu(that->createStandardMenu(win));

   QMenu *editMenu = win->menuBar()->addMenu(tr("Search"));
   editMenu->addAction(action(FindInScriptAction));
   editMenu->addAction(action(FindNextInScriptAction));
   editMenu->addAction(action(FindPreviousInScriptAction));
   editMenu->addSeparator();
   editMenu->addAction(action(GoToLineAction));

   QMenu *viewMenu = win->menuBar()->addMenu(tr("View"));
   viewMenu->addAction(scriptsDock->toggleViewAction());
   viewMenu->addAction(breakpointsDock->toggleViewAction());
   viewMenu->addAction(stackDock->toggleViewAction());
   viewMenu->addAction(localsDock->toggleViewAction());
   viewMenu->addAction(consoleDock->toggleViewAction());
   viewMenu->addAction(debugOutputDock->toggleViewAction());
   viewMenu->addAction(errorLogDock->toggleViewAction());
#endif

   QWidget *central = new QWidget();
   QVBoxLayout *vbox = new QVBoxLayout(central);
   vbox->setMargin(0);
   vbox->addWidget(widget(CodeWidget));
   vbox->addWidget(widget(CodeFinderWidget));
   widget(CodeFinderWidget)->hide();
   win->setCentralWidget(central);

   win->setWindowTitle(tr("Qt Script Debugger"));
   win->setUnifiedTitleAndToolBarOnMac(true);

   QSettings settings(QSettings::UserScope, QLatin1String("CopperSpice"));
   QVariant geometry = settings.value(QLatin1String("CS/scripttools/debugging/mainWindowGeometry"));
   if (geometry.isValid()) {
      win->restoreGeometry(geometry.toByteArray());
   }
   QVariant state = settings.value(QLatin1String("CS/scripttools/debugging/mainWindowState"));
   if (state.isValid()) {
      win->restoreState(state.toByteArray());
   }

   WidgetClosedNotifier *closedNotifier = new WidgetClosedNotifier(win, that);
   QObject::connect(closedNotifier, SIGNAL(widgetClosed()), action(ContinueAction), SLOT(trigger()));

   const_cast<QScriptEngineDebuggerPrivate *>(d)->standardWindow = win;
   return win;
}
#endif // QT_NO_MAINWINDOW

QMenu *QScriptEngineDebugger::createStandardMenu(QWidget *parent)
{
   Q_D(QScriptEngineDebugger);
   d->createDebugger();
   return d->debugger->createStandardMenu(parent, this);
}

#ifndef QT_NO_TOOLBAR
QToolBar *QScriptEngineDebugger::createStandardToolBar(QWidget *parent)
{
   Q_D(QScriptEngineDebugger);
   d->createDebugger();
   return d->debugger->createStandardToolBar(parent, this);
}
#endif

void QScriptEngineDebugger::_q_showStandardWindow()
{
  	Q_D(QScriptEngineDebugger);
	d->_q_showStandardWindow();
}

QT_END_NAMESPACE