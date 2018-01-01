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

#ifndef CONTENTWINDOW_H
#define CONTENTWINDOW_H

#include <QtCore/QUrl>
#include <QtCore/QModelIndex>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QHelpEngine;
class QHelpContentItem;
class QHelpContentWidget;

class ContentWindow : public QWidget
{
    Q_OBJECT

public:
    ContentWindow();
    ~ContentWindow();

    bool syncToContent(const QUrl &url);
    void expandToDepth(int depth);

signals:
    void linkActivated(const QUrl &link);
    void escapePressed();

private slots:
    void showContextMenu(const QPoint &pos);
    void expandTOC();
    void itemClicked(const QModelIndex &index);

private:
    void focusInEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

    QHelpContentWidget * const m_contentWidget;
    int m_expandDepth;
};

QT_END_NAMESPACE

#endif // CONTENTWINDOW_H
