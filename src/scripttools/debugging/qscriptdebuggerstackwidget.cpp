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

#include "qscriptdebuggerstackwidget_p.h"
#include "qscriptdebuggerstackwidgetinterface_p_p.h"

#include <QtCore/qdebug.h>
#include <QtGui/qheaderview.h>
#include <QtGui/qtreeview.h>
#include <QtGui/qboxlayout.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerStackWidgetPrivate
   : public QScriptDebuggerStackWidgetInterfacePrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerStackWidget)
 public:
   QScriptDebuggerStackWidgetPrivate();
   ~QScriptDebuggerStackWidgetPrivate();

   // private slots
   void _q_onCurrentChanged(const QModelIndex &index);

   QTreeView *view;
};

QScriptDebuggerStackWidgetPrivate::QScriptDebuggerStackWidgetPrivate()
{
}

QScriptDebuggerStackWidgetPrivate::~QScriptDebuggerStackWidgetPrivate()
{
}

void QScriptDebuggerStackWidgetPrivate::_q_onCurrentChanged(const QModelIndex &index)
{
   Q_Q(QScriptDebuggerStackWidget);
   emit q->currentFrameChanged(index.row());
}

QScriptDebuggerStackWidget::QScriptDebuggerStackWidget(QWidget *parent)
   : QScriptDebuggerStackWidgetInterface(*new QScriptDebuggerStackWidgetPrivate, parent, 0)
{
   Q_D(QScriptDebuggerStackWidget);
   d->view = new QTreeView();
   d->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
   d->view->setAlternatingRowColors(true);
   d->view->setRootIsDecorated(false);
   d->view->setSelectionBehavior(QAbstractItemView::SelectRows);
   d->view->header()->setDefaultAlignment(Qt::AlignLeft);
   //  d->view->header()->setResizeMode(QHeaderView::ResizeToContents);

   QVBoxLayout *vbox = new QVBoxLayout(this);
   vbox->setMargin(0);
   vbox->addWidget(d->view);
}

QScriptDebuggerStackWidget::~QScriptDebuggerStackWidget()
{
}

/*!
  \reimp
*/
QAbstractItemModel *QScriptDebuggerStackWidget::stackModel() const
{
   Q_D(const QScriptDebuggerStackWidget);
   return d->view->model();
}

/*!
  \reimp
*/
void QScriptDebuggerStackWidget::setStackModel(QAbstractItemModel *model)
{
   Q_D(QScriptDebuggerStackWidget);
   d->view->setModel(model);

   QObject::connect(d->view->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(_q_onCurrentChanged(const QModelIndex &)));

   d->view->header()->resizeSection(0, 50);
}

/*!
  \reimp
*/
int QScriptDebuggerStackWidget::currentFrameIndex() const
{
   Q_D(const QScriptDebuggerStackWidget);
   return d->view->currentIndex().row();
}

/*!
  \reimp
*/
void QScriptDebuggerStackWidget::setCurrentFrameIndex(int frameIndex)
{
   Q_D(QScriptDebuggerStackWidget);
   d->view->setCurrentIndex(d->view->model()->index(frameIndex, 0));
}

void QScriptDebuggerStackWidget::_q_onCurrentChanged(const QModelIndex & un_named_arg1)
{
  	Q_D(QScriptDebuggerStackWidget);
  	d->_q_onCurrentChanged();
}

QT_END_NAMESPACE
