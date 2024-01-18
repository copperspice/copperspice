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

#include "qscriptdebuggerscriptswidget_p.h"
#include "qscriptdebuggerscriptswidgetinterface_p_p.h"
#include "qscriptdebuggerscriptsmodel_p.h"

#include <QtCore/qdebug.h>
#include <QtGui/qheaderview.h>
#include <QtGui/qtreeview.h>
#include <QtGui/qboxlayout.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerScriptsWidgetPrivate
   : public QScriptDebuggerScriptsWidgetInterfacePrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerScriptsWidget)
 public:
   QScriptDebuggerScriptsWidgetPrivate();
   ~QScriptDebuggerScriptsWidgetPrivate();

   // private slots
   void _q_onCurrentChanged(const QModelIndex &index);

   QTreeView *view;
   qint64 currentScriptId;
};

QScriptDebuggerScriptsWidgetPrivate::QScriptDebuggerScriptsWidgetPrivate()
{
   currentScriptId = -1;
}

QScriptDebuggerScriptsWidgetPrivate::~QScriptDebuggerScriptsWidgetPrivate()
{
}

void QScriptDebuggerScriptsWidgetPrivate::_q_onCurrentChanged(const QModelIndex &index)
{
   Q_Q(QScriptDebuggerScriptsWidget);
   if (!index.isValid()) {
      return;
   }
   qint64 sid = q->scriptsModel()->scriptIdFromIndex(index);
   if (sid != -1) {
      if (currentScriptId != sid) {
         currentScriptId = sid;
         emit q->currentScriptChanged(sid);
      }
   } else {
      qint64 sid = q->scriptsModel()->scriptIdFromIndex(index.parent());
      Q_ASSERT(sid != -1);
      currentScriptId = sid;
      emit q->currentScriptChanged(sid);
      QPair<QString, int> info = q->scriptsModel()->scriptFunctionInfoFromIndex(index);
      emit q->scriptLocationSelected(info.second);
   }
}

QScriptDebuggerScriptsWidget::QScriptDebuggerScriptsWidget(QWidget *parent)
   : QScriptDebuggerScriptsWidgetInterface(*new QScriptDebuggerScriptsWidgetPrivate, parent, 0)
{
   Q_D(QScriptDebuggerScriptsWidget);
   d->view = new QTreeView();
   d->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
   //    d->view->setAlternatingRowColors(true);
   //    d->view->setRootIsDecorated(false);
   d->view->setSelectionBehavior(QAbstractItemView::SelectRows);
   d->view->header()->hide();
   //    d->view->header()->setDefaultAlignment(Qt::AlignLeft);
   //    d->view->header()->setResizeMode(QHeaderView::ResizeToContents);

   QVBoxLayout *vbox = new QVBoxLayout(this);
   vbox->setMargin(0);
   vbox->addWidget(d->view);
}

QScriptDebuggerScriptsWidget::~QScriptDebuggerScriptsWidget()
{
}

/*!
  \reimp
*/
QScriptDebuggerScriptsModel *QScriptDebuggerScriptsWidget::scriptsModel() const
{
   Q_D(const QScriptDebuggerScriptsWidget);
   return qobject_cast<QScriptDebuggerScriptsModel *>(d->view->model());
}

/*!
  \reimp
*/
void QScriptDebuggerScriptsWidget::setScriptsModel(QScriptDebuggerScriptsModel *model)
{
   Q_D(QScriptDebuggerScriptsWidget);
   d->view->setModel(model);
   QObject::connect(d->view->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(_q_onCurrentChanged(const QModelIndex &)));
}

qint64 QScriptDebuggerScriptsWidget::currentScriptId() const
{
   Q_D(const QScriptDebuggerScriptsWidget);
   return scriptsModel()->scriptIdFromIndex(d->view->currentIndex());
}

void QScriptDebuggerScriptsWidget::setCurrentScript(qint64 id)
{
   Q_D(QScriptDebuggerScriptsWidget);
   d->view->setCurrentIndex(scriptsModel()->indexFromScriptId(id));
}

void QScriptDebuggerScriptsWidget::_q_onCurrentChanged(const QModelIndex & un_named_arg1)
{
  	Q_D(QScriptDebuggerScriptsWidget);
  	d->_q_onCurrentChanged();
}

QT_END_NAMESPACE
