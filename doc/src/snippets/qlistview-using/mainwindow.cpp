/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "mainwindow.h"
#include "model.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    QMenu *itemsMenu = new QMenu(tr("&Items"));

    insertAction = itemsMenu->addAction(tr("&Insert Item"));
    removeAction = itemsMenu->addAction(tr("&Remove Item"));
    QAction *ascendingAction = itemsMenu->addAction(tr("Sort in &Ascending Order"));
    QAction *descendingAction = itemsMenu->addAction(tr("Sort in &Descending Order"));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(itemsMenu);

    QStringList strings;
    strings << tr("Oak") << tr("Fir") << tr("Pine") << tr("Birch")
            << tr("Hazel") << tr("Redwood") << tr("Sycamore") << tr("Chestnut");
    model = new StringListModel(strings, this);
/*  For convenient quoting:
    QListView *listView = new QListView(this);
*/
    listView = new QListView(this);
    listView->setModel(model);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ascendingAction, SIGNAL(triggered()), this, SLOT(sortAscending()));
    connect(descendingAction, SIGNAL(triggered()), this, SLOT(sortDescending()));
    connect(insertAction, SIGNAL(triggered()), this, SLOT(insertItem()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeItem()));
    connect(listView->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(updateMenus(const QModelIndex &)));

    updateMenus(listView->selectionModel()->currentIndex());

    setCentralWidget(listView);
    setWindowTitle(tr("List View"));
}

void MainWindow::sortAscending()
{
    model->sort(0, Qt::AscendingOrder);
}

void MainWindow::sortDescending()
{
    model->sort(0, Qt::DescendingOrder);
}

void MainWindow::insertItem()
{
    QModelIndex currentIndex = listView->currentIndex();
    if (!currentIndex.isValid())
        return;

    QString itemText = QInputDialog::getText(this, tr("Insert Item"),
        tr("Input text for the new item:"));

    if (itemText.isNull())
        return;

    if (model->insertRow(currentIndex.row(), QModelIndex())) {
        QModelIndex newIndex = model->index(currentIndex.row(), 0, QModelIndex());
        model->setData(newIndex, itemText, Qt::EditRole);

        QString toolTipText = tr("Tooltip:") + itemText;
        QString statusTipText = tr("Status tip:") + itemText;
        QString whatsThisText = tr("What's This?:") + itemText;
        model->setData(newIndex, toolTipText, Qt::ToolTipRole);
        model->setData(newIndex, toolTipText, Qt::StatusTipRole);
        model->setData(newIndex, whatsThisText, Qt::WhatsThisRole);
    }
}

void MainWindow::removeItem()
{
    QModelIndex currentIndex = listView->currentIndex();
    if (!currentIndex.isValid())
        return;

    model->removeRow(currentIndex.row(), QModelIndex());
}

void MainWindow::updateMenus(const QModelIndex &current)
{
    insertAction->setEnabled(current.isValid());
    removeAction->setEnabled(current.isValid());
}
