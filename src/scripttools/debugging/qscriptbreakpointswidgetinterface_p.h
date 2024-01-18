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

#ifndef QSCRIPTBREAKPOINTSWIDGETINTERFACE_P_H
#define QSCRIPTBREAKPOINTSWIDGETINTERFACE_P_H

#include <qwidget.h>

QT_BEGIN_NAMESPACE

class QScriptBreakpointsModel;
class QScriptDebuggerScriptsModel;
class QScriptBreakpointsWidgetInterfacePrivate;

class QScriptBreakpointsWidgetInterface : public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptBreakpointsWidgetInterface)

 public:
   ~QScriptBreakpointsWidgetInterface();

   virtual QScriptBreakpointsModel *breakpointsModel() const = 0;
   virtual void setBreakpointsModel(QScriptBreakpointsModel *model) = 0;

   virtual QScriptDebuggerScriptsModel *scriptsModel() const = 0;
   virtual void setScriptsModel(QScriptDebuggerScriptsModel *model) = 0;

 public:
   CS_SIGNAL_1(Public, void currentScriptChanged(qint64 scriptId))
   CS_SIGNAL_2(currentScriptChanged, scriptId)

 protected:
   QScriptBreakpointsWidgetInterface(
      QScriptBreakpointsWidgetInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptBreakpointsWidgetInterface)
   Q_DISABLE_COPY(QScriptBreakpointsWidgetInterface)
};

QT_END_NAMESPACE

#endif
