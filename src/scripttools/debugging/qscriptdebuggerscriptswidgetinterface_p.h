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

#ifndef QSCRIPTDEBUGGERSCRIPTSWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERSCRIPTSWIDGETINTERFACE_P_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerScriptsModel;
class QScriptDebuggerScriptsWidgetInterfacePrivate;

class QScriptDebuggerScriptsWidgetInterface : public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerScriptsWidgetInterface)

 public:
   ~QScriptDebuggerScriptsWidgetInterface();

   virtual QScriptDebuggerScriptsModel *scriptsModel() const = 0;
   virtual void setScriptsModel(QScriptDebuggerScriptsModel *model) = 0;

   virtual qint64 currentScriptId() const = 0;
   virtual void setCurrentScript(qint64 id) = 0;

   CS_SIGNAL_1(Public, void currentScriptChanged(qint64 scriptId))
   CS_SIGNAL_2(currentScriptChanged, scriptId)
   CS_SIGNAL_1(Public, void scriptLocationSelected(int lineNumber))
   CS_SIGNAL_2(scriptLocationSelected, lineNumber)

 protected:
   QScriptDebuggerScriptsWidgetInterface(
      QScriptDebuggerScriptsWidgetInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerScriptsWidgetInterface)
   Q_DISABLE_COPY(QScriptDebuggerScriptsWidgetInterface)
};

QT_END_NAMESPACE

#endif
