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

#ifndef QSCRIPTDEBUGGERCODEVIEW_P_H
#define QSCRIPTDEBUGGERCODEVIEW_P_H

#include <qscriptdebuggercodeviewinterface_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCodeViewPrivate;

class QScriptDebuggerCodeView : public QScriptDebuggerCodeViewInterface
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerCodeView)

 public:
   QScriptDebuggerCodeView(QWidget *parent = nullptr);
   ~QScriptDebuggerCodeView();

   QString text() const;
   void setText(const QString &text);

   int cursorLineNumber() const;
   void gotoLine(int lineNumber);

   int find(const QString &exp, int options = 0);

   void setExecutionLineNumber(int lineNumber, bool error);
   void setExecutableLineNumbers(const QSet<int> &lineNumbers);

   int baseLineNumber() const;
   void setBaseLineNumber(int lineNumber);

   void setBreakpoint(int lineNumber);
   void deleteBreakpoint(int lineNumber);
   void setBreakpointEnabled(int lineNumber, bool enable);

   bool event(QEvent *e);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerCodeView)
   Q_DISABLE_COPY(QScriptDebuggerCodeView)
};

QT_END_NAMESPACE

#endif
