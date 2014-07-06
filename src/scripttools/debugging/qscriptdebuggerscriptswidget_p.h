/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSCRIPTDEBUGGERSCRIPTSWIDGET_P_H
#define QSCRIPTDEBUGGERSCRIPTSWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qscriptdebuggerscriptswidgetinterface_p.h"

QT_BEGIN_NAMESPACE

class QScriptDebuggerScriptsWidgetPrivate;
class QScriptDebuggerScriptsWidget:
   public QScriptDebuggerScriptsWidgetInterface
{
   CS_OBJECT(QScriptDebuggerScriptsWidget)
 public:
   QScriptDebuggerScriptsWidget(QWidget *parent = 0);
   ~QScriptDebuggerScriptsWidget();

   QScriptDebuggerScriptsModel *scriptsModel() const;
   void setScriptsModel(QScriptDebuggerScriptsModel *model);

   qint64 currentScriptId() const;
   void setCurrentScript(qint64 id);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerScriptsWidget)
   Q_DISABLE_COPY(QScriptDebuggerScriptsWidget)

   CS_SLOT_1(Private, void _q_onCurrentChanged(const QModelIndex &un_named_arg1))
   CS_SLOT_2(_q_onCurrentChanged)

   /*  PRIVATE_SLOT
   void _q_onCurrentChanged(const QModelIndex & un_named_arg1)
   {
   	Q_D(QScriptDebuggerScriptsWidget);
   	d->_q_onCurrentChanged();
   }
   */
};

QT_END_NAMESPACE

#endif
