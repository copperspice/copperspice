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

#ifndef QSCRIPTDEBUGGERCONSOLEWIDGET_P_H
#define QSCRIPTDEBUGGERCONSOLEWIDGET_P_H

#include <qscriptdebuggerconsolewidgetinterface_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerConsoleWidgetPrivate;

class QScriptDebuggerConsoleWidget : public QScriptDebuggerConsoleWidgetInterface
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerConsoleWidget)

 public:
   QScriptDebuggerConsoleWidget(QWidget *parent = nullptr);
   ~QScriptDebuggerConsoleWidget();

   void message(QtMsgType type, const QString &text,
                const QString &fileName = QString(),
                int lineNumber = -1, int columnNumber = -1,
                const QVariant &data = QVariant());

   void setLineContinuationMode(bool enabled);

   void clear();

 protected:
   void keyPressEvent(QKeyEvent *event);
   bool focusNextPrevChild(bool);

   QScriptDebuggerConsoleWidget(
      QScriptDebuggerConsoleWidgetPrivate &dd,
      QWidget *parent);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerConsoleWidget)
   Q_DISABLE_COPY(QScriptDebuggerConsoleWidget)

   CS_SLOT_1(Private, void _q_onLineEntered(const QString &un_named_arg1))
   CS_SLOT_2(_q_onLineEntered)

   CS_SLOT_1(Private, void _q_onLineEdited(const QString &un_named_arg1))
   CS_SLOT_2(_q_onLineEdited)
 
   CS_SLOT_1(Private, void _q_onCompletionTaskFinished())
   CS_SLOT_2(_q_onCompletionTaskFinished)
  
};

QT_END_NAMESPACE

#endif
