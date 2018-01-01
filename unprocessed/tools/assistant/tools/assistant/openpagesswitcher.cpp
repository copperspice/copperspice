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

#include "openpagesswitcher.h"

#include "centralwidget.h"
#include "openpagesmodel.h"
#include "openpageswidget.h"
#include "tracer.h"

#include <QtCore/QEvent>

#include <QtGui/QKeyEvent>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

const int gWidth = 300;
const int gHeight = 200;

OpenPagesSwitcher::OpenPagesSwitcher(OpenPagesModel *model)
    : QFrame(0, Qt::Popup)
    , m_openPagesModel(model)
{
    TRACE_OBJ
    resize(gWidth, gHeight);

    m_openPagesWidget = new OpenPagesWidget(m_openPagesModel);

    // We disable the frame on this list view and use a QFrame around it instead.
    // This improves the look with QGTKStyle.
#ifndef Q_WS_MAC
    setFrameStyle(m_openPagesWidget->frameStyle());
#endif
    m_openPagesWidget->setFrameStyle(QFrame::NoFrame);

    m_openPagesWidget->allowContextMenu(false);
    m_openPagesWidget->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_openPagesWidget);

    connect(m_openPagesWidget, SIGNAL(closePage(QModelIndex)), this,
        SIGNAL(closePage(QModelIndex)));
    connect(m_openPagesWidget, SIGNAL(setCurrentPage(QModelIndex)), this,
        SIGNAL(setCurrentPage(QModelIndex)));
}

OpenPagesSwitcher::~OpenPagesSwitcher()
{
    TRACE_OBJ
}

void OpenPagesSwitcher::gotoNextPage()
{
    TRACE_OBJ
    selectPageUpDown(1);
}

void OpenPagesSwitcher::gotoPreviousPage()
{
    TRACE_OBJ
    selectPageUpDown(-1);
}

void OpenPagesSwitcher::selectAndHide()
{
    TRACE_OBJ
    setVisible(false);
    emit setCurrentPage(m_openPagesWidget->currentIndex());
}

void OpenPagesSwitcher::selectCurrentPage()
{
    TRACE_OBJ
    m_openPagesWidget->selectCurrentPage();
}

void OpenPagesSwitcher::setVisible(bool visible)
{
    TRACE_OBJ
    QWidget::setVisible(visible);
    if (visible)
        setFocus();
}

void OpenPagesSwitcher::focusInEvent(QFocusEvent *event)
{
    TRACE_OBJ
    Q_UNUSED(event)
    m_openPagesWidget->setFocus();
}

bool OpenPagesSwitcher::eventFilter(QObject *object, QEvent *event)
{
    TRACE_OBJ
    if (object == m_openPagesWidget) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Escape) {
                setVisible(false);
                return true;
            }

            const int key = ke->key();
            if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
                emit setCurrentPage(m_openPagesWidget->currentIndex());
                return true;
            }

            Qt::KeyboardModifier modifier = Qt::ControlModifier;
#ifdef Q_WS_MAC
            modifier = Qt::AltModifier;
#endif
            if (key == Qt::Key_Backtab
                && (ke->modifiers() == (modifier | Qt::ShiftModifier)))
                gotoPreviousPage();
            else if (key == Qt::Key_Tab && (ke->modifiers() == modifier))
                gotoNextPage();
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (ke->modifiers() == 0
               /*HACK this is to overcome some event inconsistencies between platforms*/
               || (ke->modifiers() == Qt::AltModifier
               && (ke->key() == Qt::Key_Alt || ke->key() == -1))) {
                selectAndHide();
            }
        }
    }
    return QWidget::eventFilter(object, event);
}

void OpenPagesSwitcher::selectPageUpDown(int summand)
{
    TRACE_OBJ
    const int pageCount = m_openPagesModel->rowCount();
    if (pageCount < 2)
        return;

    const QModelIndexList &list = m_openPagesWidget->selectionModel()->selectedIndexes();
    if (list.isEmpty())
        return;

    QModelIndex index = list.first();
    if (!index.isValid())
        return;

    index = m_openPagesModel->index((index.row() + summand + pageCount) % pageCount, 0);
    if (index.isValid()) {
        m_openPagesWidget->setCurrentIndex(index);
        m_openPagesWidget->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

QT_END_NAMESPACE
