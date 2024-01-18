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

#ifndef QSCRIPTDEBUGGERCODEVIEWINTERFACE_P_H
#define QSCRIPTDEBUGGERCODEVIEWINTERFACE_P_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QPoint;
class QStringList;

class QScriptDebuggerCodeViewInterfacePrivate;
class QScriptDebuggerCodeViewInterface:
   public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerCodeViewInterface)
 public:
   ~QScriptDebuggerCodeViewInterface();

   virtual QString text() const = 0;
   virtual void setText(const QString &text) = 0;

   virtual int cursorLineNumber() const = 0;
   virtual void gotoLine(int lineNumber) = 0;

   virtual int find(const QString &exp, int options = 0) = 0;

   virtual void setExecutionLineNumber(int lineNumber, bool error) = 0;
   virtual void setExecutableLineNumbers(const QSet<int> &lineNumbers) = 0;

   virtual int baseLineNumber() const = 0;
   virtual void setBaseLineNumber(int lineNumber) = 0;

   virtual void setBreakpoint(int lineNumber) = 0;
   virtual void deleteBreakpoint(int lineNumber) = 0;
   virtual void setBreakpointEnabled(int lineNumber, bool enable) = 0;

 public:
   CS_SIGNAL_1(Public, void breakpointToggleRequest(int lineNumber, bool on))
   CS_SIGNAL_2(breakpointToggleRequest, lineNumber, on)
   CS_SIGNAL_1(Public, void breakpointEnableRequest(int lineNumber, bool enable))
   CS_SIGNAL_2(breakpointEnableRequest, lineNumber, enable)
   CS_SIGNAL_1(Public, void toolTipRequest(const QPoint &pos, int lineNumber, const QStringList &path))
   CS_SIGNAL_2(toolTipRequest, pos, lineNumber, path)

 protected:
   QScriptDebuggerCodeViewInterface(
      QScriptDebuggerCodeViewInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerCodeViewInterface)
   Q_DISABLE_COPY(QScriptDebuggerCodeViewInterface)
};

QT_END_NAMESPACE

#endif
