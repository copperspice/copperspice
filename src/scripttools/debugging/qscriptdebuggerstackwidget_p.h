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

#ifndef QSCRIPTDEBUGGERSTACKWIDGET_P_H
#define QSCRIPTDEBUGGERSTACKWIDGET_P_H

#include <qscriptdebuggerstackwidgetinterface_p.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerStackWidgetPrivate;

class QScriptDebuggerStackWidget: public QScriptDebuggerStackWidgetInterface
{
   SCRIPT_T_CS_OBJECT(QScriptDebuggerStackWidget)

 public:
   QScriptDebuggerStackWidget(QWidget *parent = nullptr);
   ~QScriptDebuggerStackWidget();

   QAbstractItemModel *stackModel() const;
   void setStackModel(QAbstractItemModel *model);

   int currentFrameIndex() const;
   void setCurrentFrameIndex(int frameIndex);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerStackWidget)
   Q_DISABLE_COPY(QScriptDebuggerStackWidget)

   CS_SLOT_1(Private, void _q_onCurrentChanged(const QModelIndex &un_named_arg1))
   CS_SLOT_2(_q_onCurrentChanged)    
};

QT_END_NAMESPACE

#endif
