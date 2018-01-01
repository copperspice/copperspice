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

#include "indexwindow.h"

#include "centralwidget.h"
#include "helpenginewrapper.h"
#include "helpviewer.h"
#include "openpagesmanager.h"
#include "topicchooser.h"
#include "tracer.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QListWidgetItem>

#include <QtHelp/QHelpIndexWidget>

QT_BEGIN_NAMESPACE

IndexWindow::IndexWindow(QWidget *parent)
    : QWidget(parent)
    , m_searchLineEdit(new QLineEdit)
    , m_indexWidget(HelpEngineWrapper::instance().indexWidget())
{
    TRACE_OBJ
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *l = new QLabel(tr("&Look for:"));
    layout->addWidget(l);

    l->setBuddy(m_searchLineEdit);
    connect(m_searchLineEdit, SIGNAL(textChanged(QString)), this,
        SLOT(filterIndices(QString)));
    m_searchLineEdit->installEventFilter(this);
    layout->setMargin(4);
    layout->addWidget(m_searchLineEdit);

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    m_indexWidget->installEventFilter(this);
    connect(helpEngine.indexModel(), SIGNAL(indexCreationStarted()), this,
        SLOT(disableSearchLineEdit()));
    connect(helpEngine.indexModel(), SIGNAL(indexCreated()), this,
        SLOT(enableSearchLineEdit()));
    connect(m_indexWidget, SIGNAL(linkActivated(QUrl,QString)), this,
        SIGNAL(linkActivated(QUrl)));
    connect(m_indexWidget, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)),
        this, SIGNAL(linksActivated(QMap<QString,QUrl>,QString)));
    connect(m_searchLineEdit, SIGNAL(returnPressed()), m_indexWidget,
        SLOT(activateCurrentItem()));
    layout->addWidget(m_indexWidget);

    m_indexWidget->viewport()->installEventFilter(this);
}

IndexWindow::~IndexWindow()
{
    TRACE_OBJ
}

void IndexWindow::filterIndices(const QString &filter)
{
    TRACE_OBJ
    if (filter.contains(QLatin1Char('*')))
        m_indexWidget->filterIndices(filter, filter);
    else
        m_indexWidget->filterIndices(filter, QString());
}

bool IndexWindow::eventFilter(QObject *obj, QEvent *e)
{
    TRACE_OBJ
    if (obj == m_searchLineEdit && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        QModelIndex idx = m_indexWidget->currentIndex();
        switch (ke->key()) {
        case Qt::Key_Up:
            idx = m_indexWidget->model()->index(idx.row()-1,
                idx.column(), idx.parent());
            if (idx.isValid()) {
                m_indexWidget->setCurrentIndex(idx);
                return true;
            }
            break;
        case Qt::Key_Down:
            idx = m_indexWidget->model()->index(idx.row()+1,
                idx.column(), idx.parent());
            if (idx.isValid()) {
                m_indexWidget->setCurrentIndex(idx);
                return true;
            }
            break;
        case Qt::Key_Escape:
            emit escapePressed();
            return true;
        default: ; // stop complaining
        }
    } else if (obj == m_indexWidget && e->type() == QEvent::ContextMenu) {
        QContextMenuEvent *ctxtEvent = static_cast<QContextMenuEvent*>(e);
        QModelIndex idx = m_indexWidget->indexAt(ctxtEvent->pos());
        if (idx.isValid()) {
            QMenu menu;
            QAction *curTab = menu.addAction(tr("Open Link"));
            QAction *newTab = menu.addAction(tr("Open Link in New Tab"));
            menu.move(m_indexWidget->mapToGlobal(ctxtEvent->pos()));

            QAction *action = menu.exec();
            if (curTab == action)
                m_indexWidget->activateCurrentItem();
            else if (newTab == action) {
                open(m_indexWidget, idx);
            }
        }
    } else if (m_indexWidget && obj == m_indexWidget->viewport()
        && e->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        QModelIndex idx = m_indexWidget->indexAt(mouseEvent->pos());
        if (idx.isValid()) {
            Qt::MouseButtons button = mouseEvent->button();
            if (((button == Qt::LeftButton) && (mouseEvent->modifiers() & Qt::ControlModifier))
                || (button == Qt::MidButton)) {
                open(m_indexWidget, idx);
            }
        }
    }
#ifdef Q_OS_MAC
    else if (obj == m_indexWidget && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
           m_indexWidget->activateCurrentItem();
    }
#endif
    return QWidget::eventFilter(obj, e);
}

void IndexWindow::enableSearchLineEdit()
{
    TRACE_OBJ
    m_searchLineEdit->setDisabled(false);
    filterIndices(m_searchLineEdit->text());
}

void IndexWindow::disableSearchLineEdit()
{
    TRACE_OBJ
    m_searchLineEdit->setDisabled(true);
}

void IndexWindow::setSearchLineEditText(const QString &text)
{
    TRACE_OBJ
    m_searchLineEdit->setText(text);
}

void IndexWindow::focusInEvent(QFocusEvent *e)
{
    TRACE_OBJ
    if (e->reason() != Qt::MouseFocusReason) {
        m_searchLineEdit->selectAll();
        m_searchLineEdit->setFocus();
    }
}

void IndexWindow::open(QHelpIndexWidget* indexWidget, const QModelIndex &index)
{
    TRACE_OBJ
    QHelpIndexModel *model = qobject_cast<QHelpIndexModel*>(indexWidget->model());
    if (model) {
        QString keyword = model->data(index, Qt::DisplayRole).toString();
        QMap<QString, QUrl> links = model->linksForKeyword(keyword);

        QUrl url;
        if (links.count() > 1) {
            TopicChooser tc(this, keyword, links);
            if (tc.exec() == QDialog::Accepted) 
                url = tc.link();
        } else if (links.count() == 1) {
            url = links.constBegin().value();
        } else {
            return;
        }

        if (!HelpViewer::canOpenPage(url.path()))
            CentralWidget::instance()->setSource(url);
        else
            OpenPagesManager::instance()->createPage(url);
    }
}

QT_END_NAMESPACE
