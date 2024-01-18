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

#ifndef QSCRIPTDEBUGGERLOCALSWIDGET_P_H
#define QSCRIPTDEBUGGERLOCALSWIDGET_P_H

#include <qscriptdebuggerlocalswidgetinterface_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerLocalsWidgetPrivate;

class QScriptDebuggerLocalsWidget : public QScriptDebuggerLocalsWidgetInterface
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerLocalsWidget)

 public:
   QScriptDebuggerLocalsWidget(QWidget *parent = nullptr);
   ~QScriptDebuggerLocalsWidget();

   QScriptDebuggerLocalsModel *localsModel() const;
   void setLocalsModel(QScriptDebuggerLocalsModel *model);

   void expand(const QModelIndex &index);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerLocalsWidget)
   Q_DISABLE_COPY(QScriptDebuggerLocalsWidget)
 
   CS_SLOT_1(Private, void _q_onCompletionTaskFinished())
   CS_SLOT_2(_q_onCompletionTaskFinished)
  
   CS_SLOT_1(Private, void _q_insertCompletion(const QString &un_named_arg1))
   CS_SLOT_2(_q_insertCompletion)

   CS_SLOT_1(Private, void _q_expandIndex(const QModelIndex &un_named_arg1))
   CS_SLOT_2(_q_expandIndex)

};

QT_END_NAMESPACE

#endif
