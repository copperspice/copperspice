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

#ifndef QSCRIPTDEBUGGERCODEFINDERWIDGETINTERFACE_P_H
#define QSCRIPTDEBUGGERCODEFINDERWIDGETINTERFACE_P_H

#include <QtGui/qwidget.h>
#include <QtGui/qtextdocument.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCodeFinderWidgetInterfacePrivate;

class QScriptDebuggerCodeFinderWidgetInterface : public QWidget
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerCodeFinderWidgetInterface)

 public:
   ~QScriptDebuggerCodeFinderWidgetInterface();

   virtual int findOptions() const = 0;

   virtual QString text() const = 0;
   virtual void setText(const QString &text) = 0;

   virtual void setOK(bool ok) = 0;
   virtual void setWrapped(bool wrapped) = 0;
 
   CS_SIGNAL_1(Public, void findRequest(const QString &exp, int options))
   CS_SIGNAL_2(findRequest, exp, options)

 protected:
   QScriptDebuggerCodeFinderWidgetInterface(
      QScriptDebuggerCodeFinderWidgetInterfacePrivate &dd,
      QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerCodeFinderWidgetInterface)
   Q_DISABLE_COPY(QScriptDebuggerCodeFinderWidgetInterface)
};

QT_END_NAMESPACE

#endif
