/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "contentwindow.h"

#include "centralwidget.h"
#include "helpenginewrapper.h"
#include "helpviewer.h"
#include "openpagesmanager.h"
#include "tracer.h"

#include <QtGui/QLayout>
#include <QtGui/QFocusEvent>
#include <QtGui/QMenu>

#include <QtHelp/QHelpContentWidget>

QT_BEGIN_NAMESPACE

ContentWindow::ContentWindow()
    : m_contentWidget(HelpEngineWrapper::instance().contentWidget())
    , m_expandDepth(-2)
{
    TRACE_OBJ
    m_contentWidget->viewport()->installEventFilter(this);
    m_contentWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(4);
    layout->addWidget(m_contentWidget);

    connect(m_contentWidget, SIGNAL(customContextMenuRequested(QPoint)), this,
        SLOT(showContextMenu(QPoint)));
    connect(m_contentWidget, SIGNAL(linkActivated(QUrl)), this,
        SIGNAL(linkActivated(QUrl)));

    QHelpContentModel *contentModel =
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    connect(contentModel, SIGNAL(contentsCreated()), this, SLOT(expandTOC()));
}

ContentWindow::~ContentWindow()
{
    TRACE_OBJ
}

bool ContentWindow::syncToContent(const QUrl& url)
{
    TRACE_OBJ
    QModelIndex idx = m_contentWidget->indexOf(url);
    if (!idx.isValid())
        return false;
    m_contentWidget->setCurrentIndex(idx);
    return true;
}

void ContentWindow::expandTOC()
{
    TRACE_OBJ
    Q_ASSERT(m_expandDepth >= -2);
    if (m_expandDepth > -2) {
        expandToDepth(m_expandDepth);
        m_expandDepth = -2;
    }
}

void ContentWindow::expandToDepth(int depth)
{
    TRACE_OBJ
    Q_ASSERT(depth >= -2);
    m_expandDepth = depth;
    if (depth == -1)
        m_contentWidget->expandAll();
    else if (depth == 0)
        m_contentWidget->collapseAll();
    else
        m_contentWidget->expandToDepth(depth - 1);
}

void ContentWindow::focusInEvent(QFocusEvent *e)
{
    TRACE_OBJ
    if (e->reason() != Qt::MouseFocusReason)
        m_contentWidget->setFocus();
}

void ContentWindow::keyPressEvent(QKeyEvent *e)
{
    TRACE_OBJ
    if (e->key() == Qt::Key_Escape)
        emit escapePressed();
}

bool ContentWindow::eventFilter(QObject *o, QEvent *e)
{
    TRACE_OBJ
    if (m_contentWidget && o == m_contentWidget->viewport()
        && e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        const QModelIndex &index = m_contentWidget->indexAt(me->pos());
        if (!index.isValid())
            return QWidget::eventFilter(o, e);

        const Qt::MouseButtons button = me->button();
        QItemSelectionModel *sm = m_contentWidget->selectionModel();
        if (sm->isSelected(index)) {
            if ((button == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier))
                || (button == Qt::MidButton)) {
                QHelpContentModel *contentModel =
                    qobject_cast<QHelpContentModel*>(m_contentWidget->model());
                if (contentModel) {
                    QHelpContentItem *itm = contentModel->contentItemAt(index);
                    if (itm && HelpViewer::canOpenPage(itm->url().path()))
                        OpenPagesManager::instance()->createPage(itm->url());
                }
            } else if (button == Qt::LeftButton) {
                itemClicked(index);
            }
        }
    }
    return QWidget::eventFilter(o, e);
}


void ContentWindow::showContextMenu(const QPoint &pos)
{
    TRACE_OBJ
    if (!m_contentWidget->indexAt(pos).isValid())
        return;

    QHelpContentModel *contentModel =
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());
    QHelpContentItem *itm =
        contentModel->contentItemAt(m_contentWidget->currentIndex());

    QMenu menu;
    QAction *curTab = menu.addAction(tr("Open Link"));
    QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
    if (!HelpViewer::canOpenPage(itm->url().path()))
        newTab->setEnabled(false);
    
    menu.move(m_contentWidget->mapToGlobal(pos));

    QAction *action = menu.exec();
    if (curTab == action)
        emit linkActivated(itm->url());
    else if (newTab == action)
        OpenPagesManager::instance()->createPage(itm->url());
}

void ContentWindow::itemClicked(const QModelIndex &index)
{
    TRACE_OBJ
    QHelpContentModel *contentModel =
        qobject_cast<QHelpContentModel*>(m_contentWidget->model());

    if (contentModel) {
        if (QHelpContentItem *itm = contentModel->contentItemAt(index)) {
            const QUrl &url = itm->url();
            if (url != CentralWidget::instance()->currentSource())
                emit linkActivated(url);
        }
    }
}

QT_END_NAMESPACE
