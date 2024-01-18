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

#include "qscriptdebuggercodewidget_p.h"
#include "qscriptdebuggercodewidgetinterface_p_p.h"
#include "qscriptdebuggercodeview_p.h"
#include "qscriptdebuggerscriptsmodel_p.h"
#include "qscriptbreakpointsmodel_p.h"
#include "qscripttooltipproviderinterface_p.h"

#include <QtCore/qdebug.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qstackedwidget.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCodeWidgetPrivate
   : public QScriptDebuggerCodeWidgetInterfacePrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerCodeWidget)
 public:
   QScriptDebuggerCodeWidgetPrivate();
   ~QScriptDebuggerCodeWidgetPrivate();

   qint64 scriptId(QScriptDebuggerCodeViewInterface *view) const;

   // private slots
   void _q_onBreakpointToggleRequest(int lineNumber, bool on);
   void _q_onBreakpointEnableRequest(int lineNumber, bool enable);
   void _q_onBreakpointsAboutToBeRemoved(const QModelIndex &, int, int);
   void _q_onBreakpointsInserted(const QModelIndex &, int, int);
   void _q_onBreakpointsDataChanged(const QModelIndex &, const QModelIndex &);
   void _q_onScriptsChanged();
   void _q_onToolTipRequest(const QPoint &pos, int lineNumber, const QStringList &path);

   QScriptDebuggerScriptsModel *scriptsModel;
   QStackedWidget *viewStack;
   QHash<qint64, QScriptDebuggerCodeViewInterface *> viewHash;
   QScriptBreakpointsModel *breakpointsModel;
   QScriptToolTipProviderInterface *toolTipProvider;
};

QScriptDebuggerCodeWidgetPrivate::QScriptDebuggerCodeWidgetPrivate()
{
   scriptsModel = 0;
   breakpointsModel = 0;
   toolTipProvider = 0;
}

QScriptDebuggerCodeWidgetPrivate::~QScriptDebuggerCodeWidgetPrivate()
{
}

qint64 QScriptDebuggerCodeWidgetPrivate::scriptId(QScriptDebuggerCodeViewInterface *view) const
{
   if (!view) {
      return -1;
   }
   return viewHash.key(view);
}

void QScriptDebuggerCodeWidgetPrivate::_q_onBreakpointToggleRequest(int lineNumber, bool on)
{
   QScriptDebuggerCodeViewInterface *view = qobject_cast<QScriptDebuggerCodeViewInterface *>(q_func()->sender());
   qint64 sid = scriptId(view);
   Q_ASSERT(sid != -1);
   if (on) {
      QScriptBreakpointData data(sid, lineNumber);
      data.setFileName(scriptsModel->scriptData(sid).fileName());
      breakpointsModel->setBreakpoint(data);
   } else {
      int bpid = breakpointsModel->resolveBreakpoint(sid, lineNumber);
      if (bpid == -1) {
         bpid = breakpointsModel->resolveBreakpoint(scriptsModel->scriptData(sid).fileName(), lineNumber);
      }
      Q_ASSERT(bpid != -1);
      breakpointsModel->deleteBreakpoint(bpid);
   }
}

void QScriptDebuggerCodeWidgetPrivate::_q_onBreakpointEnableRequest(int lineNumber, bool enable)
{
   QScriptDebuggerCodeViewInterface *view = qobject_cast<QScriptDebuggerCodeViewInterface *>(q_func()->sender());
   qint64 sid = scriptId(view);
   int bpid = breakpointsModel->resolveBreakpoint(sid, lineNumber);
   if (bpid == -1) {
      bpid = breakpointsModel->resolveBreakpoint(scriptsModel->scriptData(sid).fileName(), lineNumber);
   }
   Q_ASSERT(bpid != -1);
   QScriptBreakpointData data = breakpointsModel->breakpointData(bpid);
   data.setEnabled(enable);
   breakpointsModel->setBreakpointData(bpid, data);
}

void QScriptDebuggerCodeWidgetPrivate::_q_onBreakpointsAboutToBeRemoved(
   const QModelIndex &, int first, int last)
{
   for (int i = first; i <= last; ++i) {
      QScriptBreakpointData data = breakpointsModel->breakpointDataAt(i);
      qint64 scriptId = data.scriptId();
      if (scriptId == -1) {
         scriptId = scriptsModel->resolveScript(data.fileName());
         if (scriptId == -1) {
            continue;
         }
      }
      QScriptDebuggerCodeViewInterface *view = viewHash.value(scriptId);
      if (!view) {
         continue;
      }
      view->deleteBreakpoint(data.lineNumber());
   }
}

void QScriptDebuggerCodeWidgetPrivate::_q_onBreakpointsInserted(
   const QModelIndex &, int first, int last)
{
   for (int i = first; i <= last; ++i) {
      QScriptBreakpointData data = breakpointsModel->breakpointDataAt(i);
      qint64 scriptId = data.scriptId();
      if (scriptId == -1) {
         scriptId = scriptsModel->resolveScript(data.fileName());
         if (scriptId == -1) {
            continue;
         }
      }
      QScriptDebuggerCodeViewInterface *view = viewHash.value(scriptId);
      if (!view) {
         continue;
      }
      view->setBreakpoint(data.lineNumber());
   }
}

void QScriptDebuggerCodeWidgetPrivate::_q_onBreakpointsDataChanged(
   const QModelIndex &tl, const QModelIndex &br)
{
   for (int i = tl.row(); i <= br.row(); ++i) {
      QScriptBreakpointData data = breakpointsModel->breakpointDataAt(i);
      qint64 scriptId = data.scriptId();
      if (scriptId == -1) {
         scriptId = scriptsModel->resolveScript(data.fileName());
         if (scriptId == -1) {
            continue;
         }
      }
      QScriptDebuggerCodeViewInterface *view = viewHash.value(scriptId);
      if (!view) {
         continue;
      }
      view->setBreakpointEnabled(data.lineNumber(), data.isEnabled());
   }
}

void QScriptDebuggerCodeWidgetPrivate::_q_onScriptsChanged()
{
   // kill editors for scripts that have been removed
   QHash<qint64, QScriptDebuggerCodeViewInterface *>::iterator it;
   for (it = viewHash.begin(); it != viewHash.end(); ) {
      if (!scriptsModel->scriptData(it.key()).isValid()) {
         it = viewHash.erase(it);
      } else {
         ++it;
      }
   }
}

void QScriptDebuggerCodeWidgetPrivate::_q_onToolTipRequest(
   const QPoint &pos, int lineNumber, const QStringList &path)
{
   toolTipProvider->showToolTip(pos, /*frameIndex=*/ -1, lineNumber, path);
}

QScriptDebuggerCodeWidget::QScriptDebuggerCodeWidget(QWidget *parent)
   : QScriptDebuggerCodeWidgetInterface(*new QScriptDebuggerCodeWidgetPrivate, parent, 0)
{
   Q_D(QScriptDebuggerCodeWidget);
   QVBoxLayout *vbox = new QVBoxLayout(this);
   vbox->setMargin(0);
   d->viewStack = new QStackedWidget();
   vbox->addWidget(d->viewStack);
}

QScriptDebuggerCodeWidget::~QScriptDebuggerCodeWidget()
{
}

QScriptDebuggerScriptsModel *QScriptDebuggerCodeWidget::scriptsModel() const
{
   Q_D(const QScriptDebuggerCodeWidget);
   return d->scriptsModel;
}

void QScriptDebuggerCodeWidget::setScriptsModel(QScriptDebuggerScriptsModel *model)
{
   Q_D(QScriptDebuggerCodeWidget);
   d->scriptsModel = model;
   QObject::connect(model, SIGNAL(layoutChanged()), this, SLOT(_q_onScriptsChanged()));
}

qint64 QScriptDebuggerCodeWidget::currentScriptId() const
{
   Q_D(const QScriptDebuggerCodeWidget);
   return d->scriptId(currentView());
}

void QScriptDebuggerCodeWidget::setCurrentScript(qint64 scriptId)
{
   Q_D(QScriptDebuggerCodeWidget);
   if (scriptId == -1) {
      // ### show "native script"
      return;
   }
   QScriptDebuggerCodeViewInterface *view = d->viewHash.value(scriptId);
   if (!view) {
      Q_ASSERT(d->scriptsModel != 0);
      QScriptScriptData data = d->scriptsModel->scriptData(scriptId);
      if (!data.isValid()) {
         return;
      }
      view = new QScriptDebuggerCodeView(); // ### use factory, so user can provide his own view
      view->setBaseLineNumber(data.baseLineNumber());
      view->setText(data.contents());
      view->setExecutableLineNumbers(d->scriptsModel->executableLineNumbers(scriptId));

      QObject::connect(view, SIGNAL(breakpointToggleRequest(int, bool)),
                       this, SLOT(_q_onBreakpointToggleRequest(int, bool)));

      QObject::connect(view, SIGNAL(breakpointEnableRequest(int, bool)),
                       this, SLOT(_q_onBreakpointEnableRequest(int, bool)));

      QObject::connect(view, SIGNAL(toolTipRequest(const QPoint &, int, const QStringList &)),
                       this, SLOT(_q_onToolTipRequest(const QPoint &, int, const QStringList &)));

      d->viewStack->addWidget(view);
      d->viewHash.insert(scriptId, view);
   }

   d->viewStack->setCurrentWidget(view);
}

void QScriptDebuggerCodeWidget::invalidateExecutionLineNumbers()
{
   Q_D(QScriptDebuggerCodeWidget);
   QHash<qint64, QScriptDebuggerCodeViewInterface *>::const_iterator it;
   for (it = d->viewHash.constBegin(); it != d->viewHash.constEnd(); ++it) {
      it.value()->setExecutionLineNumber(-1, /*error=*/false);
   }
}

QScriptBreakpointsModel *QScriptDebuggerCodeWidget::breakpointsModel() const
{
   Q_D(const QScriptDebuggerCodeWidget);
   return d->breakpointsModel;
}

void QScriptDebuggerCodeWidget::setBreakpointsModel(QScriptBreakpointsModel *model)
{
   Q_D(QScriptDebuggerCodeWidget);
   d->breakpointsModel = model;

   QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                    this, SLOT(_q_onBreakpointsAboutToBeRemoved(const QModelIndex &, int, int)));

   QObject::connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                    this, SLOT(_q_onBreakpointsInserted(const QModelIndex &, int, int)));

   QObject::connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                    this, SLOT(_q_onBreakpointsDataChanged(const QModelIndex &, const QModelIndex &)));
}

void QScriptDebuggerCodeWidget::setToolTipProvider(QScriptToolTipProviderInterface *toolTipProvider)
{
   Q_D(QScriptDebuggerCodeWidget);
   d->toolTipProvider = toolTipProvider;
}

QScriptDebuggerCodeViewInterface *QScriptDebuggerCodeWidget::currentView() const
{
   Q_D(const QScriptDebuggerCodeWidget);
   return qobject_cast<QScriptDebuggerCodeViewInterface *>(d->viewStack->currentWidget());
}

void QScriptDebuggerCodeWidget::_q_onBreakpointToggleRequest(int un_named_arg1,bool un_named_arg2)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onBreakpointToggleRequest();
}

void QScriptDebuggerCodeWidget:_q_onBreakpointEnableRequest(int un_named_arg1,bool un_named_arg2)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onBreakpointEnableRequest();
}

void QScriptDebuggerCodeWidget:_q_onBreakpointsAboutToBeRemoved(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onBreakpointsAboutToBeRemoved();
}

void QScriptDebuggerCodeWidget:_q_onBreakpointsInserted(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onBreakpointsInserted();
}

void QScriptDebuggerCodeWidget:_q_onBreakpointsDataChanged(const QModelIndex & un_named_arg1,const QModelIndex & un_named_arg2)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onBreakpointsDataChanged();
}

void QScriptDebuggerCodeWidget: _q_onScriptsChanged()
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onScriptsChanged();
}

void QScriptDebuggerCodeWidget:_q_onToolTipRequest(const QPoint & un_named_arg1,int un_named_arg2,const QStringList & un_named_arg3)
{
	Q_D(QScriptDebuggerCodeWidget);
	d->_q_onToolTipRequest();
}

QT_END_NAMESPACE
