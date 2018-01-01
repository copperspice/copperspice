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

#ifndef OPENPAGESWIDGET_H
#define OPENPAGESWIDGET_H

#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTreeView>

QT_BEGIN_NAMESPACE

class OpenPagesModel;

class OpenPagesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit OpenPagesDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;

    mutable QModelIndex pressedIndex;
};

class OpenPagesWidget : public QTreeView
{
    Q_OBJECT
public:
    OpenPagesWidget(OpenPagesModel *model);
    ~OpenPagesWidget();

    void selectCurrentPage();
    void allowContextMenu(bool ok);

signals:
    void setCurrentPage(const QModelIndex &index);
    void closePage(const QModelIndex &index);
    void closePagesExcept(const QModelIndex &index);

private slots:
    void contextMenuRequested(QPoint pos);
    void handlePressed(const QModelIndex &index);
    void handleClicked(const QModelIndex &index);

private:
    bool eventFilter(QObject *obj, QEvent *event);

    bool m_allowContextMenu;
    OpenPagesDelegate *m_delegate;
};

QT_END_NAMESPACE

#endif // OPENPAGESWIDGET_H
