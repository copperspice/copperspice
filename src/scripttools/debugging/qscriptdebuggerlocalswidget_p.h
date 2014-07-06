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

#ifndef QSCRIPTDEBUGGERLOCALSWIDGET_P_H
#define QSCRIPTDEBUGGERLOCALSWIDGET_P_H

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

#include "qscriptdebuggerlocalswidgetinterface_p.h"

QT_BEGIN_NAMESPACE

class QScriptDebuggerLocalsWidgetPrivate;
class QScriptDebuggerLocalsWidget:
   public QScriptDebuggerLocalsWidgetInterface
{
   CS_OBJECT(QScriptDebuggerLocalsWidget)
 public:
   QScriptDebuggerLocalsWidget(QWidget *parent = 0);
   ~QScriptDebuggerLocalsWidget();

   QScriptDebuggerLocalsModel *localsModel() const;
   void setLocalsModel(QScriptDebuggerLocalsModel *model);

   void expand(const QModelIndex &index);

 private:
   Q_DECLARE_PRIVATE(QScriptDebuggerLocalsWidget)
   Q_DISABLE_COPY(QScriptDebuggerLocalsWidget)

   CS_SLOT_1(Private, void _q_onCompletionTaskFinished())
   CS_SLOT_2(_q_onCompletionTaskFinished)

   /*  PRIVATE_SLOT
   void _q_onCompletionTaskFinished()
   {
   	Q_D(QScriptDebuggerLocalsWidget);
   	d->_q_onCompletionTaskFinished();
   }
   */
   CS_SLOT_1(Private, void _q_insertCompletion(const QString &un_named_arg1))
   CS_SLOT_2(_q_insertCompletion)

   /*  PRIVATE_SLOT
   void _q_insertCompletion(const QString & un_named_arg1)
   {
   	Q_D(QScriptDebuggerLocalsWidget);
   	d->_q_insertCompletion();
   }
   */
   CS_SLOT_1(Private, void _q_expandIndex(const QModelIndex &un_named_arg1))
   CS_SLOT_2(_q_expandIndex)

   /*  PRIVATE_SLOT
   void _q_expandIndex(const QModelIndex & un_named_arg1)
   {
   	Q_D(QScriptDebuggerLocalsWidget);
   	d->_q_expandIndex();
   }
   */
};

QT_END_NAMESPACE

#endif
